#include "LessonFetchService.hpp"

#include "esp_http_client.h"
#include "sdkconfig.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
    static void setError(char *errorOut, size_t errorOutSize, const char *msg)
    {
        if (errorOut && errorOutSize > 0)
        {
            snprintf(errorOut, errorOutSize, "%s", msg ? msg : "unknown error");
        }
    }

    static bool hasJsonExt(const char *filename)
    {
        if (!filename)
        {
            return false;
        }

        const size_t len = strlen(filename);
        if (len < 5)
        {
            return false;
        }

        const char *ext = filename + (len - 5);
        return (tolower((unsigned char)ext[0]) == '.' &&
                tolower((unsigned char)ext[1]) == 'j' &&
                tolower((unsigned char)ext[2]) == 's' &&
                tolower((unsigned char)ext[3]) == 'o' &&
                tolower((unsigned char)ext[4]) == 'n');
    }
}

namespace LessonFetch
{
    bool isSupportedUrl(const char *url)
    {
        if (!url || !url[0])
        {
            return false;
        }
        return (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0);
    }

    bool isSafeFilename(const char *filename)
    {
        if (!filename || !filename[0])
        {
            return false;
        }

        if (strstr(filename, "..") != nullptr)
        {
            return false;
        }

        for (const char *p = filename; *p; ++p)
        {
            const char c = *p;
            if (c == '/' || c == '\\')
            {
                return false;
            }
        }

        return true;
    }

    bool isValidAuthInput(const char *username, const char *password)
    {
        const bool hasUser = (username && username[0]);
        const bool hasPass = (password && password[0]);

        // Allow either no credentials, or both username/password provided.
        return (hasUser == hasPass);
    }

    bool sanitizeDirName(const char *dir, char *out, size_t outSize)
    {
        if (!out || outSize == 0)
        {
            return false;
        }

        const char *src = (dir && dir[0]) ? dir : "lessons";
        for (size_t i = 0; src[i]; ++i)
        {
            const char c = src[i];
            if (!isalnum((unsigned char)c) && c != '_' && c != '-')
            {
                out[0] = '\0';
                return false;
            }
        }

        if (snprintf(out, outSize, "%s", src) >= (int)outSize)
        {
            out[0] = '\0';
            return false;
        }

        return true;
    }

    bool deriveFilenameFromUrl(const char *url, char *out, size_t outSize)
    {
        if (!url || !out || outSize == 0)
        {
            return false;
        }

        const char *start = strrchr(url, '/');
        start = start ? (start + 1) : url;

        const char *end = start;
        while (*end && *end != '?' && *end != '#')
        {
            ++end;
        }

        const size_t rawLen = (size_t)(end - start);
        if (rawLen == 0)
        {
            return snprintf(out, outSize, "LESSON.JSON") < (int)outSize;
        }

        if (rawLen + 1 > outSize)
        {
            return false;
        }

        memcpy(out, start, rawLen);
        out[rawLen] = '\0';

        return true;
    }

    bool ensureJsonExtension(char *filename, size_t filenameSize)
    {
        if (!filename || filenameSize == 0 || !filename[0])
        {
            return false;
        }

        if (hasJsonExt(filename))
        {
            return true;
        }

        const size_t len = strlen(filename);
        if (len + 5 >= filenameSize)
        {
            return false;
        }

        strcat(filename, ".JSON");
        return true;
    }

    bool buildSdFilePath(const char *dir, const char *filename, char *out, size_t outSize)
    {
        if (!dir || !filename || !out || outSize == 0)
        {
            return false;
        }

        if (snprintf(out, outSize, "/sdcard/%s/%s", dir, filename) >= (int)outSize)
        {
            return false;
        }

        return true;
    }

    esp_err_t fetchJsonToSd(const char *url,
                           const char *dir,
                           const char *preferredFilename,
                           const char *username,
                           const char *password,
                           char *savedPath,
                           size_t savedPathSize,
                           char *errorOut,
                           size_t errorOutSize)
    {
        if (!isSupportedUrl(url))
        {
            setError(errorOut, errorOutSize, "Invalid URL (http/https only)");
            return ESP_ERR_INVALID_ARG;
        }

        if (!isValidAuthInput(username, password))
        {
            setError(errorOut, errorOutSize, "Invalid auth input (username/password must both be set)");
            return ESP_ERR_INVALID_ARG;
        }

        char safeDir[64] = {0};
        if (!sanitizeDirName(dir, safeDir, sizeof(safeDir)))
        {
            setError(errorOut, errorOutSize, "Invalid target directory");
            return ESP_ERR_INVALID_ARG;
        }

        char filename[128] = {0};
        if (preferredFilename && preferredFilename[0])
        {
            if (!isSafeFilename(preferredFilename) || snprintf(filename, sizeof(filename), "%s", preferredFilename) >= (int)sizeof(filename))
            {
                setError(errorOut, errorOutSize, "Invalid filename");
                return ESP_ERR_INVALID_ARG;
            }
        }
        else if (!deriveFilenameFromUrl(url, filename, sizeof(filename)))
        {
            setError(errorOut, errorOutSize, "Failed to derive filename from URL");
            return ESP_ERR_INVALID_ARG;
        }

        if (!ensureJsonExtension(filename, sizeof(filename)))
        {
            setError(errorOut, errorOutSize, "Filename too long");
            return ESP_ERR_INVALID_ARG;
        }

        char folderPath[96] = {0};
        if (snprintf(folderPath, sizeof(folderPath), "/sdcard/%s", safeDir) >= (int)sizeof(folderPath))
        {
            setError(errorOut, errorOutSize, "Target directory path too long");
            return ESP_ERR_INVALID_ARG;
        }

        struct stat st;
        if (stat(folderPath, &st) != 0)
        {
            if (mkdir(folderPath, 0755) != 0)
            {
                setError(errorOut, errorOutSize, "Failed to create target directory");
                return ESP_FAIL;
            }
        }

        char finalPath[256] = {0};
        if (!buildSdFilePath(safeDir, filename, finalPath, sizeof(finalPath)))
        {
            setError(errorOut, errorOutSize, "Final path too long");
            return ESP_ERR_INVALID_SIZE;
        }

        char tempPath[272] = {0};
        if (snprintf(tempPath, sizeof(tempPath), "%s.part", finalPath) >= (int)sizeof(tempPath))
        {
            setError(errorOut, errorOutSize, "Temp path too long");
            return ESP_ERR_INVALID_SIZE;
        }

        FILE *fp = fopen(tempPath, "wb");
        if (!fp)
        {
            setError(errorOut, errorOutSize, "Failed to create temporary file");
            return ESP_FAIL;
        }

        esp_http_client_config_t cfg = {};
        cfg.url = url;
        cfg.timeout_ms = 15000;
    #if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
        cfg.crt_bundle_attach = esp_crt_bundle_attach;
    #endif
        cfg.keep_alive_enable = false;

        if (username && username[0] && password && password[0])
        {
            cfg.username = username;
            cfg.password = password;
            cfg.auth_type = HTTP_AUTH_TYPE_BASIC;
        }

        esp_http_client_handle_t client = esp_http_client_init(&cfg);
        if (!client)
        {
            fclose(fp);
            unlink(tempPath);
            setError(errorOut, errorOutSize, "Failed to init HTTP client");
            return ESP_FAIL;
        }

        esp_err_t err = esp_http_client_open(client, 0);
        if (err != ESP_OK)
        {
            esp_http_client_cleanup(client);
            fclose(fp);
            unlink(tempPath);
            setError(errorOut, errorOutSize, "Failed to open URL");
            return err;
        }

        (void)esp_http_client_fetch_headers(client);
        const int status = esp_http_client_get_status_code(client);
        if (status != 200)
        {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            fclose(fp);
            unlink(tempPath);
            setError(errorOut, errorOutSize, "Remote server returned non-200 status");
            return ESP_FAIL;
        }

        char buf[1024];
        int total = 0;
        while (true)
        {
            const int r = esp_http_client_read(client, buf, sizeof(buf));
            if (r < 0)
            {
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                fclose(fp);
                unlink(tempPath);
                setError(errorOut, errorOutSize, "HTTP read failed");
                return ESP_FAIL;
            }

            if (r == 0)
            {
                break;
            }

            const size_t w = fwrite(buf, 1, (size_t)r, fp);
            if (w != (size_t)r)
            {
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                fclose(fp);
                unlink(tempPath);
                setError(errorOut, errorOutSize, "File write failed");
                return ESP_FAIL;
            }

            total += r;
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        fclose(fp);

        if (total <= 0)
        {
            unlink(tempPath);
            setError(errorOut, errorOutSize, "Downloaded file is empty");
            return ESP_FAIL;
        }

        unlink(finalPath);
        if (rename(tempPath, finalPath) != 0)
        {
            unlink(tempPath);
            setError(errorOut, errorOutSize, "Failed to finalize file");
            return ESP_FAIL;
        }

        if (savedPath && savedPathSize > 0)
        {
            snprintf(savedPath, savedPathSize, "%s", finalPath);
        }

        if (errorOut && errorOutSize > 0)
        {
            errorOut[0] = '\0';
        }

        return ESP_OK;
    }
}
