#include "cJSON.h"
#include "UartBridge.h"
#include "VerificationEngine.h"
#include <stdio.h>
#include <stdbool.h>

extern "C" {
void uart_bridge_init(void) {}
void uart_bridge_deinit(void) {}
uart_bridge_err_t uart_bridge_idn(char *buf, size_t len) {
    if (buf && len > 0) snprintf(buf, len, "LabBuddy,PC_Simulator_Mock,SN001,v1.0.0");
    return UB_OK;
}
uart_bridge_err_t uart_bridge_reset(void) { return UB_OK; }
uart_bridge_err_t uart_bridge_conf_output(uint8_t dip_pin) { (void)dip_pin; return UB_OK; }
uart_bridge_err_t uart_bridge_conf_input(uint8_t dip_pin) { (void)dip_pin; return UB_OK; }
uart_bridge_err_t uart_bridge_set_pin(uint8_t dip_pin, uint8_t val) { (void)dip_pin; (void)val; return UB_OK; }
uart_bridge_err_t uart_bridge_read_pin(uint8_t ch, uint8_t *val) { (void)ch; if (val) *val = 0; return UB_OK; }
uart_bridge_err_t uart_bridge_pwr(uint8_t on) { (void)on; return UB_OK; }
uart_bridge_err_t uart_bridge_port_out(uint8_t mask) { (void)mask; return UB_OK; }
uart_bridge_err_t uart_bridge_port_in(uint8_t *mask) { if (mask) *mask = 0; return UB_OK; }
}

VerifResult VerificationEngine::run(cJSON *verif_obj) {
    (void)verif_obj;
    VerifResult res;
    res.pass = true;
    res.total = 2;
    res.passed = 2;
    snprintf(res.msg, sizeof(res.msg), "PC Simulator: Hardware test mocked (PASS)");
    return res;
}
