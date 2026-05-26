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

/**
 * Build 8-bit masks from a JSON array of 0/1 integers.
 * Array index j maps to bit j (max 8 bits).
 * val_mask  : bit j set if array[j] != 0
 * check_mask: bit j set for every valid index
 */
static void build_mask(cJSON *arr, uint8_t *val_mask, uint8_t *check_mask)
{
    *val_mask = 0;
    *check_mask = 0;
    if (!arr || !cJSON_IsArray(arr))
        return;
    int n = cJSON_GetArraySize(arr);
    if (n > 8)
        n = 8;
    for (int i = 0; i < n; i++)
    {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        *check_mask |= (uint8_t)(1u << i);
        if (item && cJSON_IsNumber(item) && item->valueint)
            *val_mask |= (uint8_t)(1u << i);
    }
}

/*
 * VerificationEngine::run()
 *
 * Supports two JSON schemas inside "verification":
 *
 * Schema A - drive_ports / truth_table (existing lesson JSONs L001..):
 *   {
 *     "drive_ports": ["port_A"],   <- index j -> PA bit j (PORT:OUT)
 *     "read_ports":  ["port_Y"],   <- index k -> PD bit k (PORT:IN?)
 *     "truth_table": [
 *       { "inputs": [0], "expected": [1] },
 *       { "inputs": [1], "expected": [0] }
 *     ],
 *     "timing": { "drive_settle_ms": 5, "read_delay_ms": 2 }
 *   }
 *
 * Schema B - compact rows:
 *   {
 *     "pwr_settle_ms": 150,
 *     "settle_ms": 20,
 *     "read_mask": 255,
 *     "rows": [
 *       { "drive": 0, "expected": 1 },
 *       { "drive": 1, "expected": 0 }
 *     ]
 *   }
 */
VerifResult VerificationEngine::run(cJSON *verif_obj)
{
    VerifResult res{};
    res.pass = false;
    snprintf(res.msg, sizeof(res.msg), "verification JSON missing");

    if (!verif_obj)
        return res;

    cJSON *truth_table = cJSON_GetObjectItem(verif_obj, "truth_table");
    cJSON *rows_arr = cJSON_GetObjectItem(verif_obj, "rows");

    if (!truth_table && !rows_arr)
    {
        snprintf(res.msg, sizeof(res.msg), "no truth_table or rows in verification JSON");
        return res;
    }

    /* common power-up */
    cJSON *timing_obj = cJSON_GetObjectItem(verif_obj, "timing");
    int pwr_settle_ms = 150;
    int drive_settle_ms = timing_obj ? jint(timing_obj, "drive_settle_ms", 20) : 20;
    int read_delay_ms = timing_obj ? jint(timing_obj, "read_delay_ms", drive_settle_ms)
                                   : drive_settle_ms;

    uart_bridge_init();

    /* ---- Quick H7 link check ---- */
    char idn[80] = "";
    if (uart_bridge_idn(idn, sizeof(idn)) != UB_OK)
    {
        snprintf(res.msg, sizeof(res.msg),
                 "H7 ไม่ตอบสนอง — ตรวจ UART (GPIO37->PA10, GPIO38<-PA9) และ power H7");
        return res;
    }
    ESP_LOGI(TAG, "H7 link OK: %s", idn);

    /* ---- Power cycle: VCC off -> settle -> VCC on ---- */
    if (uart_bridge_pwr(0) != UB_OK)
    {
        snprintf(res.msg, sizeof(res.msg), "PWR OFF failed (timeout)");
        return res;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    uart_bridge_err_t err = uart_bridge_pwr(1);
    if (err != UB_OK)
    {
        snprintf(res.msg, sizeof(res.msg), "VCC on failed (timeout)");
        return res;
    }
    vTaskDelay(pdMS_TO_TICKS(pwr_settle_ms));

    res.pass = true;
    res.passed = 0;
    res.total = 0;

    /* ================================================================
     * Schema A: drive_ports / truth_table
     * ================================================================ */
    if (truth_table && cJSON_IsArray(truth_table))
    {
        int n = cJSON_GetArraySize(truth_table);
        res.total = n;

        for (int i = 0; i < n; i++)
        {
            cJSON *row = cJSON_GetArrayItem(truth_table, i);
            cJSON *inputs = row ? cJSON_GetObjectItem(row, "inputs") : nullptr;
            cJSON *expected = row ? cJSON_GetObjectItem(row, "expected") : nullptr;

            uint8_t drive_val, drive_chk;
            uint8_t exp_val, exp_chk;
            build_mask(inputs, &drive_val, &drive_chk);
            build_mask(expected, &exp_val, &exp_chk);

            err = uart_bridge_port_out(drive_val);
            if (err != UB_OK)
            {
                snprintf(res.msg, sizeof(res.msg), "PORT:OUT failed row %d (timeout)", i);
                res.pass = false;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(drive_settle_ms + read_delay_ms));

            uint8_t actual = 0;
            err = uart_bridge_port_in(&actual);
            if (err != UB_OK)
            {
                snprintf(res.msg, sizeof(res.msg), "PORT:IN? failed row %d (timeout)", i);
                res.pass = false;
                break;
            }

            uint8_t actual_m = actual & exp_chk;
            ESP_LOGI(TAG, "row %d  drive=0x%02X  actual=0x%02X(mask=0x%02X)  exp=0x%02X  %s",
                     i, drive_val, actual_m, exp_chk, exp_val,
                     (actual_m == exp_val) ? "PASS" : "FAIL");

            if (actual_m == exp_val)
                res.passed++;
            else
            {
                if (res.pass)
                    snprintf(res.msg, sizeof(res.msg),
                             "FAIL row %d: drive=0x%02X got=0x%02X exp=0x%02X",
                             i, drive_val, actual_m, exp_val);
                res.pass = false;
            }
        }
    }
    /* ================================================================
     * Schema B: simple rows { "drive": N, "expected": M }
     * ================================================================ */
    else if (rows_arr && cJSON_IsArray(rows_arr))
    {
        uint8_t read_mask = (uint8_t)jint(verif_obj, "read_mask", 0xFF);
        int settle_ms = jint(verif_obj, "settle_ms", 20);
        int n = cJSON_GetArraySize(rows_arr);
        res.total = n;

        for (int i = 0; i < n; i++)
        {
            cJSON *row = cJSON_GetArrayItem(rows_arr, i);
            uint8_t drive = row ? (uint8_t)jint(row, "drive", 0) : 0;
            uint8_t expected = row ? (uint8_t)jint(row, "expected", 0) : 0;

            err = uart_bridge_port_out(drive);
            if (err != UB_OK)
            {
                snprintf(res.msg, sizeof(res.msg), "PORT:OUT failed row %d (timeout)", i);
                res.pass = false;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(settle_ms));

            uint8_t actual = 0;
            err = uart_bridge_port_in(&actual);
            if (err != UB_OK)
            {
                snprintf(res.msg, sizeof(res.msg), "PORT:IN? failed row %d (timeout)", i);
                res.pass = false;
                break;
            }

            uint8_t actual_m = actual & read_mask;
            uint8_t expected_m = expected & read_mask;

            ESP_LOGI(TAG, "row %d  drive=0x%02X  actual=0x%02X  expected=0x%02X  %s",
                     i, drive, actual_m, expected_m,
                     (actual_m == expected_m) ? "PASS" : "FAIL");

            if (actual_m == expected_m)
                res.passed++;
            else
            {
                if (res.pass)
                    snprintf(res.msg, sizeof(res.msg),
                             "FAIL row %d: drive=0x%02X got=0x%02X exp=0x%02X",
                             i, drive, actual_m, expected_m);
                res.pass = false;
            }
        }
    }

    uart_bridge_pwr(0);

    if (res.pass)
        snprintf(res.msg, sizeof(res.msg), "PASS %d/%d", res.passed, res.total);

    return res;
}