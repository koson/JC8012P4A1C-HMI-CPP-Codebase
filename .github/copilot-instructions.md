# ESP32-P4 Workspace Instructions

## ESP-IDF Command Wrapper

**CRITICAL**: This workspace uses `idf` command, NOT `idf.py` directly.

- `idf` is a wrapper script that handles ESP-IDF environment setup
- It automatically sources the ESP-IDF environment and forwards arguments to `idf.py`
- Always use `idf` instead of `idf.py` for all ESP-IDF commands

### Command Examples
```powershell
# CORRECT - Use idf wrapper
idf build
idf menuconfig
idf flash monitor
idf -p COMxx flash

# WRONG - Don't use idf.py directly
idf.py build           # ❌ Will fail (environment not loaded)
```

## Terminal Command Rules

### **CRITICAL: ALWAYS Use Sync Mode**
**NEVER use `mode: 'async'`** - it will cause VS Code to freeze and hang!

For ALL commands, ALWAYS use these settings:
- `mode: 'sync'` - REQUIRED to prevent freezing
- `timeout: 300000` - 5 minute timeout (adjust as needed)
- All commands must run in sync mode for proper terminal interaction

### Interactive Commands 
Commands requiring user interaction (MUST use sync mode):
- `idf menuconfig` - interactive configuration menu
- `idf monitor` - serial monitor  
- Any command requiring keyboard input

### Examples
```typescript
// CORRECT - ALL commands use mode: 'sync'
run_in_terminal("idf menuconfig", mode: 'sync', timeout: 300000)
run_in_terminal("idf build", mode: 'sync', timeout: 300000)
run_in_terminal("idf flash monitor", mode: 'sync', timeout: 300000)

// WRONG - async mode will FREEZE VS Code ❌
run_in_terminal("idf menuconfig", mode: 'async')  // ❌ WILL HANG!
run_in_terminal("idf build", mode: 'async')       // ❌ WILL HANG!
```

## ESP-IDF Project Configuration
- Use `idf menuconfig` for interactive configuration
- Always run in foreground terminal (isBackground: false)
- Let user interact with the TUI menu directly

## Common Build/Flash Commands
```powershell
# Clean build (recommended when changing dependencies)
Remove-Item -Recurse -Force build, sdkconfig, managed_components, dependencies.lock -ErrorAction SilentlyContinue
idf build

# Flash and monitor
idf -p COMxx flash monitor

# Build only
idf build
```
