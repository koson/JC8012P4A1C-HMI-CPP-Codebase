#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief UART bridge between ESP32-P4 and STM32H7 measurement module.
 *
 * Protocol: SCPI text, 115200 8N1, line-terminated with '\n'.
 *   ESP32 sends: "CMD params\n"
 *   H7   replies: "OK\r\n" | "0\r\n" | "1\r\n" | "ERROR:...\r\n"
 *
 * SCPI commands used:
 *   *IDN?                      → "LabBuddy,STM32H723_Worker,SN001,v1.0.0\r\n"
 *   *RST                       → "OK\r\n"
 *   CONF:PIN:MODE <pin>,OUTPUT → "OK\r\n"  (H7 drives DIP pin → IC input)
 *   CONF:PIN:MODE <pin>,INPUT  → "OK\r\n"  (H7 reads DIP pin  → IC output)
 *   DIG:OUT <pin>,<0|1>        → "OK\r\n"
 *   DIG:IN? <pin>              → "0\r\n" or "1\r\n"
 *
 * DIP-14 pin numbering (1–14):
 *   Pin 7  = GND  → reserved (H7 returns ERROR:Reserved)
 *   Pin 14 = VCC  → reserved (H7 returns ERROR:Reserved)
 *   Pins 1–6, 8–13 are testable lines → H7 PB0–PB11
 *
 * Physical wiring (ESP32-P4 side):
 *   UART_BRIDGE_TX_PIN  →  H7 PA10 (USART1_RX)
 *   UART_BRIDGE_RX_PIN  ←  H7 PA9  (USART1_TX)
 *   GND shared
 *
 * Usage:
 * @code
 *   uart_bridge_init();
 *   uart_bridge_reset();                        // *RST — all pins to input
 *   uart_bridge_conf_output(1);                 // CONF:PIN:MODE 1,OUTPUT
 *   uart_bridge_conf_input(8);                  // CONF:PIN:MODE 8,INPUT
 *   uart_bridge_set_pin(1, 1);                  // DIG:OUT 1,1
 *   uint8_t out; uart_bridge_read_pin(8, &out); // DIG:IN? 8
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

// ── Hardware config ───────────────────────────────────────────────────────────
#define UART_BRIDGE_PORT        UART_NUM_1
#define UART_BRIDGE_BAUD        115200
#define UART_BRIDGE_TX_PIN      4       ///< ESP32-P4 TX → H7 PA10 (USART1_RX)
#define UART_BRIDGE_RX_PIN      5       ///< ESP32-P4 RX ← H7 PA9  (USART1_TX)
#define UART_BRIDGE_TIMEOUT_MS  2000    ///< 2-second response timeout

// ── Error codes ───────────────────────────────────────────────────────────────
typedef enum {
    UB_OK            = 0,
    UB_ERR_TIMEOUT   = 1,  ///< No response within UART_BRIDGE_TIMEOUT_MS
    UB_ERR_H7        = 2,  ///< H7 returned "ERROR:..."
    UB_ERR_NOT_INIT  = 3,  ///< uart_bridge_init() not called
} uart_bridge_err_t;

// ── Lifecycle ─────────────────────────────────────────────────────────────────

/** Initialise UART peripheral. Safe to call multiple times (no-op on re-call). */
void uart_bridge_init(void);

/** Deinitialise UART driver. Call before sleep or reconfiguration. */
void uart_bridge_deinit(void);

// ── SCPI commands ─────────────────────────────────────────────────────────────

/**
 * @brief *IDN? — query H7 identity string.
 * @param[out] buf  Buffer for response (NUL-terminated, strips \\r\\n).
 * @param       len  Buffer size.
 */
uart_bridge_err_t uart_bridge_idn(char *buf, size_t len);

/**
 * @brief *RST — reset all DIP pins to floating input state.
 */
uart_bridge_err_t uart_bridge_reset(void);

/**
 * @brief CONF:PIN:MODE <pin>,OUTPUT — configure DIP pin as output (H7 drives it).
 * @param dip_pin  DIP-14 pin number (1–13, not 7 or 14).
 */
uart_bridge_err_t uart_bridge_conf_output(uint8_t dip_pin);

/**
 * @brief CONF:PIN:MODE <pin>,INPUT — configure DIP pin as input (H7 reads it).
 * @param dip_pin  DIP-14 pin number (1–13, not 7 or 14).
 */
uart_bridge_err_t uart_bridge_conf_input(uint8_t dip_pin);

/**
 * @brief DIG:OUT <pin>,<val> — drive DIP output pin.
 * @param dip_pin  DIP-14 pin number configured as OUTPUT.
 * @param val      0 or 1.
 */
uart_bridge_err_t uart_bridge_set_pin(uint8_t dip_pin, uint8_t val);

/**
 * @brief DIG:IN? <pin> — read DIP input pin level.
 * @param dip_pin  DIP-14 pin number configured as INPUT.
 * @param[out] val  0 or 1.
 */
uart_bridge_err_t uart_bridge_read_pin(uint8_t dip_pin, uint8_t *val);

#ifdef __cplusplus
}
#endif
