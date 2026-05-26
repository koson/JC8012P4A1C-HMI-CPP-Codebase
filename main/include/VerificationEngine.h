#pragma once

#include "cJSON.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Result of a VerificationEngine::run() call.
 */
struct VerifResult
{
    bool pass;
    int total;
    int passed;
    char msg[96]; ///< PASS: summary text; FAIL: first failure description
};

/**
 * @brief Drives PORT:OUT / PORT:IN? SCPI commands over UartBridge to verify
 *        a physical IC against a truth table embedded in the lesson JSON.
 *
 * Expected JSON schema inside a "verify" page:
 * @code
 * "verification": {
 *   "pwr_settle_ms": 150,       // optional — wait after VCC on (default 150)
 *   "settle_ms":     20,        // optional — wait after PORT:OUT (default 20)
 *   "read_mask":     255,       // optional — AND mask for PORT:IN? result (default 0xFF)
 *   "rows": [
 *     { "drive": 0, "expected": 1 },
 *     { "drive": 1, "expected": 0 }
 *   ]
 * }
 * @endcode
 *
 * "drive"    = 8-bit value written to PORT:OUT (GPIOA PA0–PA7)
 * "expected" = expected PORT:IN? value after masking with read_mask (GPIOD PD0–PD7)
 */
class VerificationEngine
{
public:
    /**
     * @brief Run verification.
     * @param verif_obj  cJSON object from lesson JSON "verification" key.
     *                   Must outlive this call (not freed here).
     * @return VerifResult with pass/fail details.
     */
    static VerifResult run(cJSON *verif_obj);
};
