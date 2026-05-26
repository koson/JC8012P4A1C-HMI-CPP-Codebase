#include "VerificationEngine.h"
#include "UartBridge.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "VerifEngine";

static int jint(cJSON *obj, const char *key, int def)
{
    cJSON *v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsNumber(v)) ? v->valueint : def;
}

VerifResult VerificationEngine::run(cJSON *verif_obj)
{
    VerifResult res{};
    res.pass = false;

    if (!verif_obj)
    {
        snprintf(res.msg, sizeof(res.msg), "verification JSON missing");
        return res;
    }

    cJSON *rows_arr = cJSON_GetObjectItem(verif_obj, "rows");
    if (!rows_arr || !cJSON_IsArray(rows_arr))
    {
        snprintf(res.msg, sizeof(res.msg), "ไม่พบ 'rows' ใน verification JSON");
        return res;
    }

    const int pwr_settle_ms = jint(verif_obj, "pwr_settle_ms", 150);
    const int settle_ms = jint(verif_obj, "settle_ms", 20);
    const uint8_t read_mask = (uint8_t)jint(verif_obj, "read_mask", 0xFF);

    /* ---- power cycle IC ---- */
    uart_bridge_init();
    uart_bridge_pwr(0);
    vTaskDelay(pdMS_TO_TICKS(50));

    uart_bridge_err_t err = uart_bridge_pwr(1);
    if (err != UB_OK)
    {
        snprintf(res.msg, sizeof(res.msg), "เปิด VCC ไม่ได้ (timeout)");
        return res;
    }
    vTaskDelay(pdMS_TO_TICKS(pwr_settle_ms));

    /* ---- run truth table rows ---- */
    int n = cJSON_GetArraySize(rows_arr);
    res.total = n;
    res.passed = 0;
    res.pass = true;

    for (int i = 0; i < n; i++)
    {
        cJSON *row = cJSON_GetArrayItem(rows_arr, i);
        if (!row)
            continue;

        uint8_t drive = (uint8_t)jint(row, "drive", 0);
        uint8_t expected = (uint8_t)jint(row, "expected", 0);

        err = uart_bridge_port_out(drive);
        if (err != UB_OK)
        {
            snprintf(res.msg, sizeof(res.msg), "PORT:OUT ล้มเหลว row %d (timeout)", i);
            res.pass = false;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(settle_ms));

        uint8_t actual = 0;
        err = uart_bridge_port_in(&actual);
        if (err != UB_OK)
        {
            snprintf(res.msg, sizeof(res.msg), "PORT:IN? ล้มเหลว row %d (timeout)", i);
            res.pass = false;
            break;
        }

        uint8_t actual_m = actual & read_mask;
        uint8_t expected_m = expected & read_mask;

        ESP_LOGI(TAG, "row %d  drive=0x%02X  actual=0x%02X  expected=0x%02X  %s",
                 i, drive, actual_m, expected_m,
                 (actual_m == expected_m) ? "PASS" : "FAIL");

        if (actual_m == expected_m)
        {
            res.passed++;
        }
        else
        {
            if (res.pass) // record first failure only
                snprintf(res.msg, sizeof(res.msg),
                         "FAIL row %d: drive=0x%02X ได้ 0x%02X คาดหวัง 0x%02X",
                         i, drive, actual_m, expected_m);
            res.pass = false;
            // continue — test all remaining rows to get passed count
        }
    }

    uart_bridge_pwr(0);

    if (res.pass)
        snprintf(res.msg, sizeof(res.msg), "PASS %d/%d ผ่านทั้งหมด", res.passed, res.total);

    return res;
}
