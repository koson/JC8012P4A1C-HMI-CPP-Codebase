# LabBuddy File Manager Integration

## Overview

This document explains how to use the **FileManagerApplication** - a web-based file manager that allows you to upload, download, and delete JSON circuit files via WiFi.

## Features

✅ WiFi connectivity via ESP-HOSTED (ESP32-C6 coprocessor)  
✅ Web-based UI accessible from any browser  
✅ File upload with drag & drop support  
✅ File download and delete operations  
✅ Long filename (LFN) support with UTF-8 encoding  
✅ Supports JSON files up to 10MB  

## Hardware Requirements

- **ESP32-P4-Function-EV-Board** (or compatible)
- **ESP32-C6** coprocessor (onboard via ESP-HOSTED)
- **SD Card** (mounted at /sdcard)
- **WiFi network** (default: aesfiber / 29052552)

## Quick Start

### Option 1: Use FileManager as Main Application

1. **Backup current main.cpp:**
   ```bash
   cd main
   mv main.cpp main_menu_demo.cpp
   mv examples/main_file_manager.cpp main.cpp
   ```

2. **Configure WiFi** (optional - defaults to aesfiber):
   Edit `main.cpp` and uncomment/modify:
   ```cpp
   FileManagerApplication::WiFiConfig wifi = {
       .ssid = "YOUR_SSID",
       .password = "YOUR_PASSWORD",
       .max_retry = 5,
       .connect_timeout_ms = 10000
   };
   fileApp.init(sysMgr, &wifi);
   ```

3. **Build and flash:**
   ```bash
   idf build
   idf -p COM36 flash monitor
   ```

4. **Access web UI:**
   - Wait for IP address in serial monitor (e.g., 192.168.1.41)
   - Open browser: `http://192.168.1.41`
   - Upload/download/delete JSON files

### Option 2: Add FileManager to Demo Gallery

1. **Edit DemoManager** to add FileManager demo
2. **Create LVGL UI** that displays IP and file list
3. **Integrate** with MenuDemoApplication

## Configuration

### WiFi Settings

Default WiFi credentials are in `FileManagerApplication.cpp`:
- SSID: `aesfiber`
- Password: `29052552`

Change in `getDefaultWiFiConfig()` or pass custom `WiFiConfig` to `init()`.

### SD Card Structure

Files are stored in `/sdcard/WORKSHOP/`:
- Auto-created on first run
- Supports long filenames (up to 255 chars)
- UTF-8 encoding enabled

### FATFS Configuration

Configured in `sdkconfig.defaults`:
```ini
CONFIG_FATFS_LFN_HEAP=y
CONFIG_FATFS_MAX_LFN=255
CONFIG_FATFS_API_ENCODING_UTF_8=y
```

### ESP-HOSTED Configuration

SDIO transport settings in `sdkconfig.defaults`:
```ini
CONFIG_ESP_HOSTED_SDIO_EN=y
CONFIG_ESP_HOSTED_TRANSPORT=1
CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE=1
CONFIG_ESP_HOSTED_SDIO_4_BIT=y
CONFIG_ESP_HOSTED_SDIO_FREQ=40000000
```

## ESP32-C6 Slave Firmware

The ESP32-C6 firmware is a separate project. It is **not** stored in this repo's `managed_components/` folder.

This project currently resolves `espressif/esp_hosted` from the component registry via `dependencies.lock`. That means the host-side dependency version and the slave firmware version should be kept in sync.

For a repeatable workflow, keep a separate checkout next to this project, for example:

```bash
cd ..
git clone --recurse-submodules https://github.com/espressif/esp-hosted-mcu.git
cd esp-hosted-mcu\slave
idf.py set-target esp32c6
idf.py menuconfig
idf.py -p <ESP_PROG_PORT> flash monitor
```

Recommended reminders for this board:
- Do not rely on `idf.py create-project-from-example "espressif/esp_hosted:slave"` here; that example name is not available for the `esp_hosted` component version currently resolved by this project.
- Match the slave firmware release to the host-side `esp_hosted` release used by this project.
- Older notes mentioning `2.12.6+` refer to the older `esp-hosted-mcu` workflow and may be stale for newer host dependencies.
- Use the same SDIO transport config as the host.
- If you do this often, keep the `slave/` checkout in a fixed sibling folder next to this project so you do not have to search for it again.

If the serial log still prints `ESP-Hosted-MCU Slave FW version :: 2.1.10`, the C6 is still running the old image. Flash the `slave/` project again from `esp-hosted-mcu` and verify the ESP-Prog wiring on `PROG_C6`:
- `ESP_EN` -> `EN`
- `ESP_TXD` -> `TXD`
- `ESP_RXD` -> `RXD`
- `GND` -> `GND`
- `ESP_IO0` -> `IO0`
- Do not connect `VDD`

## API Usage

### Initialize and Start

```cpp
#include "SystemManager.h"
#include "FileManagerApplication.h"

// Initialize system
SystemManager& sysMgr = SystemManager::getInstance();
sysMgr.initDisplay();
sysMgr.mountSDCard();

// Initialize file manager
FileManagerApplication& fileApp = FileManagerApplication::getInstance();

// Option A: Use default WiFi config
fileApp.init(sysMgr);

// Option B: Custom WiFi config
FileManagerApplication::WiFiConfig wifi = {
    .ssid = "MyNetwork",
    .password = "MyPassword",
    .max_retry = 5,
    .connect_timeout_ms = 10000
};
fileApp.init(sysMgr, &wifi);

// Start WiFi and HTTP server
fileApp.start();

// Check connection status
if (fileApp.isWiFiConnected()) {
    ESP_LOGI("app", "IP: %s", fileApp.getIPAddress());
}
```

### Check Status

```cpp
// WiFi connected?
bool connected = fileApp.isWiFiConnected();

// Get IP address
const char* ip = fileApp.getIPAddress(); // e.g., "192.168.1.41"

// Get HTTP server handle
httpd_handle_t server = fileApp.getServerHandle();
```

### Stop FileManager

```cpp
fileApp.stop(); // Stops HTTP server and disconnects WiFi
```

## Web UI Features

### Upload Files
- Drag & drop JSON files onto upload area
- Or click "Upload" button to browse
- Files are converted to uppercase (e.g., circuit.json → CIRCUIT.JSON)
- Progress indication during upload

### File List
- Shows all files in `/sdcard/WORKSHOP/`
- Displays filename and size in KB
- Updates automatically after upload/delete

### Download Files
- Click download button (⬇️) next to file
- Browser downloads file

### Delete Files
- Click delete button (🗑️) next to file
- Confirmation prompt before deletion

## HTTP API

### GET /
Returns HTML web interface

### GET /files
Returns JSON list of files:
```json
{
  "files": [
    {"name": "CIRCUIT1.JSON", "size": 8192},
    {"name": "CIRCUIT2.JSON", "size": 4096}
  ]
}
```

### POST /upload
Upload file via multipart form data.

Request:
```
Content-Type: multipart/form-data
Body: file data
```

Response:
```json
{"success": true, "message": "Uploaded: CIRCUIT.JSON"}
```

### GET /file/{filename}
Download file.

Response: Binary file data

### DELETE /delete/{filename}
Delete file.

Response:
```json
{"success": true, "message": "Deleted: CIRCUIT.JSON"}
```

## Troubleshooting

### WiFi not connecting

1. **Check SSID/password** in code or config
2. **Check ESP32-C6 firmware version** (should be v2.12.x+):
   ```
   I (xxx) esp_hosted: Slave FW version: 2.12.6
   ```
3. **Check ESP-HOSTED config** in sdkconfig
4. **Verify SDIO wiring** (GPIO 39-44)

### SD Card not mounting

1. **Check SD card format** (FAT32)
2. **Check LDO power control** (should auto-initialize)
3. **Check SDMMC Slot** (Slot 0 for SD card, Slot 1 for WiFi)

### File upload fails

1. **Check SD card space** (must have enough free space)
2. **Check WORKSHOP directory** exists (auto-created)
3. **Check FATFS LFN config** (must be enabled)
4. **Check serial monitor** for detailed error logs

### HTTP server not starting

1. **Check WiFi connected** first
2. **Check port 80** not in use
3. **Check memory** (HTTP server needs ~50KB RAM)

## Memory Usage

Approximate memory usage:
- **FileManagerApplication**: ~5KB static
- **HTTP server**: ~50KB heap
- **WiFi stack**: ~100KB heap
- **Upload buffer**: 1KB stack per request

Total: ~155KB heap + 1KB stack

## Performance

- **Upload speed**: ~200KB/s (limited by WiFi)
- **Download speed**: ~500KB/s
- **Max file size**: Limited by SD card space
- **Concurrent connections**: 4 (HTTP server default)

## Security Considerations

⚠️ **WARNING**: This is a development tool with NO authentication!

- No username/password protection
- Anyone on the network can access files
- Suitable for **development only**
- Do NOT use in production without adding authentication

## Future Enhancements

Possible improvements:
- [ ] Add HTTP basic authentication
- [ ] Support for folders/subdirectories
- [ ] File preview in browser
- [ ] Batch upload/download
- [ ] File search and filtering
- [ ] HTTPS support
- [ ] mDNS for easy discovery (http://labbuddy.local)

## Credits

Based on:
- WebFileManager (original C implementation)
- ESP-IDF HTTP server examples
- LVGL for potential UI integration

## License

Same as main LabBuddy project.

---

**Last Updated**: 2026-04-25  
**Author**: GitHub Copilot  
**Version**: 1.0.0
