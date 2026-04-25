# ESP WiFi Remote - Local Copy

**Version:** 1.5.1 (locked from espressif/esp_wifi_remote)  
**Project:** LabBuddy JC8012P4-HMI

## Why Local Copy?

This component is copied locally instead of using `managed_components/` because:

### Kconfig Bug Fix (CRITICAL)

The original component has a bug in `Kconfig` line 10:

```kconfig
orsource "./Kconfig.idf_v$ESP_IDF_VERSION.in"  # ❌ Bug: Variable doesn't expand
```

**Impact:**
- `$ESP_IDF_VERSION` variable **does not expand** in orsource directive
- `Kconfig.idf_v5.5.1.in` is **never loaded**
- `CONFIG_SLAVE_IDF_TARGET_ESP32C6` option **never appears** in menuconfig
- Without slave target config, `esp_wifi_default_config.h` doesn't load WiFi defaults
- `WIFI_INIT_CONFIG_DEFAULT()` macro fails with "CONFIG_WIFI_RMT_* was not declared"

### Fixed Version

**File:** `components/esp_wifi_remote/Kconfig` line 10

```diff
- orsource "./Kconfig.idf_v$ESP_IDF_VERSION.in"
+ rsource "./Kconfig.idf_v5.5.1.in"
```

Now:
- ✅ `Kconfig.idf_v5.5.1.in` loads correctly
- ✅ `CONFIG_SLAVE_IDF_TARGET_ESP32C6` available in menuconfig
- ✅ `esp_wifi_default_config.h` loads ESP32-C6 WiFi defaults
- ✅ Build succeeds

## Configuration

Required settings in `sdkconfig.defaults`:

```ini
# Enable ESP WiFi Remote with ESP-HOSTED backend
CONFIG_ESP_WIFI_REMOTE_LIBRARY_HOSTED=y
CONFIG_ESP_HOSTED_ENABLED=y

# Slave target: ESP32-C6 co-processor (NOW WORKS!)
CONFIG_SLAVE_IDF_TARGET_ESP32C6=y

# SDIO transport
CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE=y
```

## Hardware

- **Host:** ESP32-P4 (JC8012P4A1C board)
- **Slave:** ESP32-C6 coprocessor
- **Transport:** SDIO (Slot 1, 4-bit, 40MHz)
- **Reset GPIO:** 54

## Updating

If you need to update this component:

1. Download new version: `idf.py update-dependencies`
2. Copy from managed_components: `Copy-Item -Recurse managed_components\espressif__esp_wifi_remote components\esp_wifi_remote -Force`
3. Apply the Kconfig fix (change line 10: `orsource` → `rsource`)
4. Update version number in this README
5. Test build: `idf fullclean; idf build`

## Original Source

https://components.espressif.com/components/espressif/esp_wifi_remote

## Related Issues

- Kconfig `orsource` doesn't expand environment variables in ESP-IDF 5.5.x
- Same workaround used in JC4880P443C project
