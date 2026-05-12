#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief UART bridge between ESP32-P4 and STM32H7 measurement module.
 *
 * Protocol (binary, fixed-width):
 *   ESP32 → H7 :  [0xBB][CMD][PARAM_HI][PARAM_LO]   (4 bytes)
 *   H7 → ESP32 :  [0xCC][RESULT]                     (2 bytes)
 *
 * Commands:
 *   CMD_SET_OUTPUT  0x01  — set banana-jack output pins (PARAM_LO = bitmask, bits 0-3)
 *   CMD_READ_INPUT  0x02  — read banana-jack input pins → RESULT bitmask (bits 0-3)
 *
 * Timeout: 2 seconds. On timeout, the call returns UB_ERR_TIMEOUT.
 *
 * Usage:
 * @code
 *   uart_bridge_init();
 *   uint8_t value;
 *   uart_bridge_err_t err = uart_bridge_read_input(&value);
 *   if (err == UB_OK) { ... }
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

// ── Hardware config (adjust to physical wiring) ───────────────────────────────
#define UART_BRIDGE_PORT        UART_NUM_1
#define UART_BRIDGE_BAUD        115200
#define UART_BRIDGE_TX_PIN      4      ///< GPIO pin: ESP32-P4 TX → H7 RX
#define UART_BRIDGE_RX_PIN      5      ///< GPIO pin: ESP32-P4 RX ← H7 TX
#define UART_BRIDGE_TIMEOUT_MS  2000   ///< 2-second response timeout

// ── Protocol constants ────────────────────────────────────────────────────────
#define UB_START_CMD    0xBB
#define UB_START_RSP    0xCC

#define UB_CMD_SET_OUTPUT   0x01  ///< Set output banana-jack pins (bitmask param_lo)
#define UB_CMD_READ_INPUT   0x02  ///< Read input banana-jack pins → result bitmask

// ── Error codes ───────────────────────────────────────────────────────────────
typedef enum {
    UB_OK            = 0,
    UB_ERR_TIMEOUT   = 1,  ///< No response within UART_BRIDGE_TIMEOUT_MS
    UB_ERR_FRAMING   = 2,  ///< Response start byte != 0xCC
    UB_ERR_NOT_INIT  = 3,  ///< uart_bridge_init() not called
} uart_bridge_err_t;

// ── Public API ────────────────────────────────────────────────────────────────

/**
 * @brief Initialise UART peripheral and install driver.
 *        Safe to call multiple times (no-op after first call).
 */
void uart_bridge_init(void);

/**
 * @brief Set banana-jack output pin states.
 * @param pin_mask  Bitmask for pins 0-3 (bit 0 = port A, bit 1 = port B, …)
 * @return UB_OK on success, error code otherwise.
 */
uart_bridge_err_t uart_bridge_set_output(uint8_t pin_mask);

/**
 * @brief Read banana-jack input pin states.
 * @param[out] pin_mask  Bitmask of read pins (bit 0 = port Y0, …)
 * @return UB_OK on success, error code otherwise.
 */
uart_bridge_err_t uart_bridge_read_input(uint8_t *pin_mask);

/**
 * @brief Deinitialise UART driver.  Call before sleep or reconfiguration.
 */
void uart_bridge_deinit(void);

#ifdef __cplusplus
}
#endif
