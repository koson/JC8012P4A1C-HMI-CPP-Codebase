/**
 * @file usb_cdc_host.h
 * @brief Reusable USB CDC-ACM Host component for ESP32-P4
 *
 * Wraps the IDF usb_host_cdc_acm class driver with a simple line-oriented
 * API suitable for SCPI request-response communication.
 *
 * Architecture:
 *   usb_cdc_host (this component)
 *     └── espressif/usb_host_cdc_acm  (managed)
 *           └── IDF usb (USB Host lib, HCD, DWC2 driver)
 *
 * Usage:
 *   usb_cdc_host_install();                        // once at startup
 *   usb_cdc_host_open(VID, PID, iface, timeout);   // block until device found
 *   usb_cdc_host_send("*IDN?");
 *   char resp[128]; usb_cdc_host_recv_line(resp, sizeof(resp), 2000);
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ── H7 USB CDC device identifiers ────────────────────────────────────────────
// STM32H723 running TinyUSB CDC: VID_0483 / PID_5740, interface 0
#define USB_CDC_HOST_H7_VID 0x0483u
#define USB_CDC_HOST_H7_PID 0x5740u
#define USB_CDC_HOST_H7_IFACE 0u

#define USB_CDC_HOST_LINE_MAX 256 // max bytes in one received line

    // ── Lifecycle ─────────────────────────────────────────────────────────────────

    /**
     * @brief Install the USB Host library and CDC-ACM class driver.
     *        Call once at startup, before any usb_cdc_host_open() call.
     * @return ESP_OK on success.
     */
    esp_err_t usb_cdc_host_install(void);

    /**
     * @brief Try to open a CDC-ACM device by VID/PID.
     *        Blocks until the device enumerates or timeout expires.
     * @param vid          USB Vendor ID
     * @param pid          USB Product ID
     * @param iface        CDC data interface number (usually 0)
     * @param timeout_ms   Enumeration timeout in milliseconds
     * @return ESP_OK on success, ESP_ERR_TIMEOUT if not found.
     */
    esp_err_t usb_cdc_host_open(uint16_t vid, uint16_t pid, uint8_t iface,
                                uint32_t timeout_ms);

    /** @return true if a device is currently open and connected. */
    bool usb_cdc_host_is_connected(void);

    // ── Data transfer ─────────────────────────────────────────────────────────────

    /**
     * @brief Send a SCPI command string (appends "\r\n" automatically).
     * @param cmd   Null-terminated command string (without trailing newline).
     * @return ESP_OK or error.
     */
    esp_err_t usb_cdc_host_send(const char *cmd);

    /**
     * @brief Wait for one complete response line ("\n"-terminated).
     *        The returned string has CR and LF stripped.
     * @param out         Buffer to write the line into.
     * @param len         Size of the buffer.
     * @param timeout_ms  Max wait time in milliseconds.
     * @return ESP_OK, ESP_ERR_TIMEOUT, or ESP_ERR_INVALID_STATE (disconnected).
     */
    esp_err_t usb_cdc_host_recv_line(char *out, size_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
