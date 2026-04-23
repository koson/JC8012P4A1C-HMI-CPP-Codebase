# LabBuddy JSON Circuit Viewer

Load and render circuit diagrams from JSON files on ESP32-P4.

## 📂 SD Card Structure

```
/sdcard/
  └── worksheets/
      ├── half_adder.json
      ├── SVG1_circuit.json
      ├── shapes1_circuit.json
      └── ... (your circuit files)
```

## 🚀 Usage

### 1. Prepare JSON Files

Export circuit JSON from **LabBuddy Designer** (Windows app):
- Open circuit in Designer
- Click **📥 Import SVG** or create new
- File → Save (automatically exports JSON)

### 2. Copy to SD Card

Copy JSON files to SD card:
```
D:\GitHubRepos\LabBuddy\Docs\*.json
→ Copy to →
/sdcard/worksheets/
```

**Files to copy:**
- `half_adder_circuit.json` - Half adder example
- `SVG1_circuit.json` - Flowchart symbols example  
- `shapes1_circuit.json` - Custom shapes (star, spiral)

### 3. Insert SD Card into ESP32-P4

Insert SD card into ESP32-P4 board.

### 4. Run Demo

1. Power on ESP32-P4
2. Navigate to menu: **"JSON Circuit Viewer"**
3. Circuit will load automatically from `/sdcard/worksheets/half_adder.json`

## 🎯 Supported Features (JSON Schema v1.1)

✅ **Embedded Symbols** - Self-contained JSON (no external library needed)
✅ **SVG Symbols** - Logic gates, flowchart shapes, custom paths
✅ **Widgets** - Position, scale, rotation, colors
✅ **Mixed Paths** - Closed paths (fill) + open paths (stroke only)
⚠️ **Wires** - Path parsing not yet implemented (coming soon)
⚠️ **Ports** - Circle rendering ready, needs testing
⚠️ **Junctions** - Circle rendering ready, needs testing

## 📝 JSON Schema Example

```json
{
  "version": "1.1",
  "title": "Half Adder",
  "width": 1280,
  "height": 800,
  "backgroundColor": "#F5F5F5",
  "embeddedSymbols": {
    "Process": {
      "id": "Process",
      "pathData": "M 15,35 H 135 V 115 H 15 Z",
      "viewBox": { "X": 0, "Y": 0, "Width": 150, "Height": 150 }
    }
  },
  "widgets": [
    {
      "symbolId": "Process",
      "type": "svgSymbol",
      "x": 150,
      "y": 100,
      "scale": 1.0,
      "strokeColor": "#2C3E50",
      "strokeWidth": 2.0
    }
  ]
}
```

## 🔧 Customization

To load a different circuit file, edit `LabBuddyJsonDemo.cpp`:

```cpp
// Change this line:
demo->loadCircuit("/sdcard/worksheets/half_adder.json");

// To your file:
demo->loadCircuit("/sdcard/worksheets/your_circuit.json");
```

## 🐛 Troubleshooting

**"Failed to open file"**
- Check SD card is inserted
- Verify file exists: `/sdcard/worksheets/circuit.json`
- Check SD card mount status in console

**"Symbol not found"**
- JSON must have `embeddedSymbols` section (v1.1)
- Check symbolId in widgets matches embeddedSymbols keys

**"Parse error"**
- Verify JSON is valid (use JSON validator)
- Check for syntax errors in exported file

## 📦 Components

- **json_renderer** - JSON parser and renderer
  - `JsonParser.cpp` - cJSON-based parser
  - `JsonRenderer.cpp` - Render to LVGL canvas
  - `JsonTypes.hpp` - Data structures

- **svg_renderer** - SVG path renderer (dependency)
  - Bezier curve conversion
  - Path command parsing

## 🎓 Next Steps

- [ ] Implement wire path rendering
- [ ] Add file browser UI (select from SD card)
- [ ] Add zoom/pan controls
- [ ] Serial upload support (Phase 2)
- [ ] Real-time interaction with hardware

---

**Created:** 2026-04-24  
**Version:** 1.0  
**Status:** ✅ Ready for testing
