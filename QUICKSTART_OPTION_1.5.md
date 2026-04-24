# Option 1.5: FileManager with Viewer UI - Quick Start

## Overview

**Option 1.5** combines the best of both worlds:
- ✅ **Web Upload**: Fast file upload via browser (no rebuild needed)
- ✅ **Screen Render**: View rendered JSON/SVG on HMI display
- ✅ **Single Renderer**: Uses same `json_renderer` component as Demo Gallery

## Architecture

```
┌─────────────────────────────────────────────────────┐
│  Developer Workflow                                 │
├─────────────────────────────────────────────────────┤
│  1. Design SVG in Inkscape                          │
│  2. Convert to JSON (svg_to_json_converter.py)      │
│  3. Open browser → http://192.168.1.41              │
│  4. Upload JSON file                                │
│  5. Look at HMI screen → Press "Refresh"            │
│  6. Select file → Press "Render"                    │
│  7. See result on screen!                           │
│                                                     │
│  If not satisfied:                                  │
│  → Fix SVG/converter                                │
│  → Re-upload (step 4)                               │
│  → Refresh → Render                                 │
│  → Iterate in ~30 seconds!                          │
└─────────────────────────────────────────────────────┘
```

## Components

```
ESP32-P4 Firmware
├── FileManagerApplication (WiFi + HTTP server)
│   ├── WiFi via ESP-HOSTED (ESP32-C6)
│   ├── HTTP server on port 80
│   └── Web UI for upload/download/delete
│
├── FileViewerUI (LVGL screen UI)
│   ├── IP address display
│   ├── File list from SD card
│   ├── Refresh/Render/Clear buttons
│   └── Canvas for displaying JSON
│
└── JsonRenderer (shared component)
    ├── Used by FileViewerUI
    ├── Used by Demo Gallery
    └── Edit once → works everywhere
```

## Setup Instructions

### 1. Activate FileManager Mode

```bash
cd D:\GitHubRepos\LabBuddy\Firmware\LabBuddy-JC8012P4-HMI

# Backup current main
mv main/main.cpp main/main_menu_demo.cpp

# Activate FileManager main
mv main/examples/main_file_manager.cpp main/main.cpp
```

### 2. (Optional) Configure WiFi

Edit `main/main.cpp` to change WiFi credentials:

```cpp
// Around line 60
FileManagerApplication::WiFiConfig wifi = {
    .ssid = "YOUR_SSID",          // Change this
    .password = "YOUR_PASSWORD",  // Change this
    .max_retry = 5,
    .connect_timeout_ms = 10000
};
fileApp.init(sysMgr, &wifi);
```

Or use default: `aesfiber / 29052552`

### 3. Build and Flash

```bash
# Build (first time takes longer - downloads dependencies)
idf build

# Flash to ESP32-P4
idf -p COM36 flash monitor
```

### 4. Wait for IP Address

Watch serial monitor:
```
I (xxx) main: WiFi connected: 192.168.1.41
I (xxx) main: Web UI: http://192.168.1.41
```

### 5. Use the System

**On Browser (PC/phone):**
1. Open `http://192.168.1.41`
2. Upload JSON files (drag & drop or browse)

**On HMI Screen:**
1. Press **🔄 Refresh** button → Scans SD card
2. Tap a file in the list → Selects it
3. Press **▶ Render** button → Displays on canvas
4. Press **🗑 Clear** button → Clears canvas

## Screen UI Layout

```
┌──────────────────────────────────────────┐
│  📁 LabBuddy File Manager                │
│  IP: 192.168.1.41                        │
│  Status: Ready                           │
├──────────────────────────────────────────┤
│  [🔄 Refresh] [▶ Render] [🗑 Clear]      │
├──────────────────────────────────────────┤
│  📂 Files:                               │
│  ☑ HALF_ADDER.JSON       (8.9 KB)       │
│  ☐ AND_GATE.JSON         (4.2 KB)       │
│  ☐ LESSON_01.JSON       (12.5 KB)       │
├──────────────────────────────────────────┤
│                                          │
│           Canvas Area                    │
│       (Rendered JSON here)               │
│                                          │
└──────────────────────────────────────────┘
```

## Development Workflow

### Typical Iteration Cycle

```
Time: ~30 seconds per iteration

1. Edit SVG in Inkscape                     [5s]
2. Run converter: python svg_to_json.py     [1s]
3. Upload via browser                       [2s]
4. Press Refresh on screen                  [1s]
5. Select file                              [1s]
6. Press Render                             [2s]
7. Evaluate result                          [10s]
8. Repeat if needed
```

### Comparison with Other Options

| Method                  | Upload   | Build | Flash | Total Time |
| ----------------------- | -------- | ----- | ----- | ---------- |
| **Option 1.5** (This)   | Web (2s) | -     | -     | **~30s**   |
| Option 2 (Demo Gallery) | -        | 60s   | 30s   | **~120s**  |
| Manual Flash            | -        | 60s   | 30s   | **~120s**  |

**4x faster iteration!** 🚀

## Testing Different Content Types

### 1. Logic Circuit (e.g., Half Adder)
```bash
# Upload half_adder.json
# Render → Should show gates + wires
```

### 2. Educational Lesson
```bash
# Upload lesson_01.json
# Render → Should show text + diagrams
```

### 3. Logic Testing UI
```bash
# Upload test_interface.json
# Render → Should show interactive buttons
```

### 4. Student Progress Summary
```bash
# Upload progress_report.json
# Render → Should show charts + stats
```

## Troubleshooting

### WiFi not connecting
```
Check:
- SSID/password in main.cpp
- ESP32-C6 firmware version (should be 2.12.x)
- Serial monitor for error messages
```

### File not found after upload
```
Fix:
1. Press "Refresh" button on screen
2. Check serial monitor: "Found X files"
3. Verify file is in /sdcard/WORKSHOP/
```

### Render fails
```
Debug:
1. Check Status label on screen (shows error)
2. Check serial monitor for detailed error
3. Common issues:
   - Invalid JSON format
   - Missing SVG symbols
   - File too large (>10MB)
```

### Screen UI not responding
```
Fix:
1. Check touch calibration
2. Restart ESP32 (press reset button)
3. Check serial monitor for LVGL errors
```

## Memory Usage

Approximate heap usage:
```
WiFi Stack:        ~100 KB
HTTP Server:        ~50 KB
LVGL UI:            ~20 KB
FileViewerUI:       ~10 KB
JSON Renderer:      ~50 KB (during render)
Total:             ~230 KB

Available: ~400 KB free (ESP32-P4 has 768KB RAM)
```

## File Size Limits

- **Recommended**: < 100 KB per file
- **Maximum**: < 1 MB per file
- **SD Card**: Any size (tested up to 32GB)

## Renderer Modifications

To modify JSON renderer:

```bash
# Edit renderer source
vim components/json_renderer/src/JsonRenderer.cpp

# Rebuild
idf build

# Flash once
idf -p COM36 flash

# Test immediately:
# - Upload file via web
# - Render on screen
# - See changes!
```

**No need to re-upload files after renderer changes!**

## Switch Back to Demo Gallery

```bash
# Restore original main
mv main/main.cpp main/examples/main_file_manager.cpp
mv main/main_menu_demo.cpp main/main.cpp

# Rebuild
idf build
idf -p COM36 flash
```

## API Reference

### FileViewerUI Class

```cpp
#include "FileViewerUI.h"

FileViewerUI viewer;

// Create UI
viewer.create(lv_screen_active());

// Update IP address
viewer.updateIP("192.168.1.41");

// Refresh file list
viewer.refreshFileList();

// Render selected file
viewer.renderSelected();

// Clear canvas
viewer.clearCanvas();
```

### FileManagerApplication Class

```cpp
#include "FileManagerApplication.h"

FileManagerApplication& app = FileManagerApplication::getInstance();

// Initialize
app.init(sysMgr, &wifi_config);

// Start WiFi + HTTP
app.start();

// Check status
if (app.isWiFiConnected()) {
    const char* ip = app.getIPAddress();
}

// Stop
app.stop();
```

## Best Practices

### ✅ Do:
- Upload files via web (fast)
- Use meaningful filenames
- Keep files under 100KB
- Test incrementally
- Use version control for SVG sources

### ❌ Don't:
- Flash new firmware for each file change
- Upload files via serial (slow)
- Use very large files (>1MB)
- Forget to press Refresh after upload
- Skip error messages in serial monitor

## Support

If you encounter issues:
1. Check serial monitor output
2. Verify WiFi connection
3. Check SD card is mounted
4. Ensure files are in /sdcard/WORKSHOP/
5. Try clearing canvas and re-rendering

For renderer bugs:
- Check `components/json_renderer/src/` for implementation
- Same code used by Demo Gallery
- Fix once → works everywhere!

---

**Happy Iterating! 🚀**

Last Updated: 2026-04-25  
Version: 1.0.0
