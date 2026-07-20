#pragma once

#include "esp_err.h"
#include <stddef.h>

namespace LessonFetch
{
    bool isSupportedUrl(const char *url);
    bool isSafeFilename(const char *filename);
    bool isValidAuthInput(const char *username, const char *password);
    bool sanitizeDirName(const char *dir, char *out, size_t outSize);
    bool deriveFilenameFromUrl(const char *url, char *out, size_t outSize);
    bool ensureJsonExtension(char *filename, size_t filenameSize);
    bool buildSdFilePath(const char *dir, const char *filename, char *out, size_t outSize);

    esp_err_t fetchJsonToSd(const char *url,
                           const char *dir,
                           const char *preferredFilename,
                           const char *username,
                           const char *password,
                           char *savedPath,
                           size_t savedPathSize,
                           char *errorOut,
                           size_t errorOutSize);
}
