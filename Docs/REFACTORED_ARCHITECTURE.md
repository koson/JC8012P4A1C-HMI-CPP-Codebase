# Refactored Codebase Architecture

## 📋 สรุปการ Refactor

โครงสร้างโค้ดถูก refactor เพื่อแยก **system initialization** ออกจาก **application logic** 
ทำให้สามารถนำ codebase ไปสร้าง project อื่นๆ ได้ง่าย โดยใช้หลักการ **OOP** และ **Clean Architecture**

## 🏗️ โครงสร้างใหม่

### 1. **SystemManager** (ระบบพื้นฐาน - นำกลับไปใช้ได้)
**ไฟล์:** 
- `main/include/SystemManager.h`
- `main/cpp_version/SystemManager.cpp`

**หน้าที่:**
- จัดการ display initialization (LVGL, BSP)
- จัดการ touch input
- จัดการ SD card (mount, test write, list files)
- Provide thread-safe display locking

**Features:**
```cpp
class SystemManager {
    static SystemManager& getInstance();
    
    // Display management
    esp_err_t initDisplay(const DisplayConfig* config = nullptr);
    lv_display_t* getDisplay();
    bool lockDisplay(int timeout_ms = -1);
    void unlockDisplay();
    
    // Touch input
    bool isTouchAvailable();
    lv_indev_t* getTouchInput();
    
    // SD card management
    esp_err_t mountSDCard();
    bool isSDCardMounted();
    bool testSDCardWrite(const char* path);
    void listSDCardContents(const char* path);
};
```

**การใช้งาน:**
```cpp
SystemManager& sysMgr = SystemManager::getInstance();

// Initialize display with default config
sysMgr.initDisplay();

// Or customize display config
SystemManager::DisplayConfig config;
config.buffer_size = BSP_LCD_H_RES * 100;
config.rotation = LV_DISPLAY_ROTATION_270;
config.use_spiram = true;
sysMgr.initDisplay(&config);

// Mount SD card
sysMgr.mountSDCard();
sysMgr.listSDCardContents("/sdcard");
```

---

### 2. **VPIApplication** (Application Logic - เฉพาะสำหรับ VPI HMI)
**ไฟล์:**
- `main/include/VPIApplication.h`
- `main/cpp_version/VPIApplication.cpp`

**หน้าที่:**
- จัดการ VPI HMI UI (VPIHMIUI)
- จัดการ UART receiver และ VPI data manager
- จัดการ CSV data logger
- จัดการ event handlers (LOG button, datetime timer)
- เชื่อมต่อระหว่าง C wrapper กับ C++ application

**Features:**
```cpp
class VPIApplication {
    static VPIApplication& getInstance();
    
    // Lifecycle
    esp_err_t init(SystemManager& sysMgr);
    esp_err_t start();
    void stop();
    
    // Data updates (called from C wrappers)
    void updateElectrical(uint8_t phase, float voltage, ...);
    void updatePressure(float percentage);
    void updateStatus(const char* status);
    
    // Access to components
    VPIHMIUI* getUI();
    VPIDocument* getDocument();
    DataLogger* getDataLogger();
};
```

**การใช้งาน:**
```cpp
VPIApplication& app = VPIApplication::getInstance();

// Initialize with system manager
app.init(sysMgr);

// Start application
app.start();

// Update data
app.updatePressure(75.5);
```

---

### 3. **main.cpp** (Entry Point - สั้นและกระชับ)
**ไฟล์:** `main/main.cpp`

**หน้าที่:**
- Entry point (`app_main()`)
- Orchestrate system และ application initialization
- Provide C wrapper functions สำหรับเรียกจาก C code

**โค้ดหลัก:**
```cpp
extern "C" void app_main(void)
{
    // Step 1: Initialize system
    SystemManager& sysMgr = SystemManager::getInstance();
    sysMgr.initDisplay();
    sysMgr.mountSDCard();
    
    // Step 2: Initialize application
    VPIApplication& app = VPIApplication::getInstance();
    app.init(sysMgr);
    
    // Step 3: Start application
    app.start();
}

// C wrappers for compatibility with existing C code
extern "C" void ui_vpi_hmi_update_pressure(float percentage) {
    VPIApplication::getInstance().updatePressure(percentage);
}
```

---

## 🚀 วิธีสร้าง Project ใหม่

### ขั้นตอนที่ 1: ใช้ SystemManager ตามเดิม
```cpp
// ไม่ต้องแก้ไข - ใช้งานได้เลย
SystemManager& sysMgr = SystemManager::getInstance();
sysMgr.initDisplay();
sysMgr.mountSDCard();
```

### ขั้นตอนที่ 2: สร้าง Application Class ใหม่
สร้าง `MyApplication.h` และ `MyApplication.cpp`:

```cpp
// MyApplication.h
class MyApplication {
public:
    static MyApplication& getInstance();
    esp_err_t init(SystemManager& sysMgr);
    esp_err_t start();
    void stop();
    
private:
    MyApplication();
    SystemManager* m_sysMgr;
    // Add your app-specific components here
};
```

### ขั้นตอนที่ 3: แก้ไข main.cpp
```cpp
extern "C" void app_main(void)
{
    SystemManager& sysMgr = SystemManager::getInstance();
    sysMgr.initDisplay();
    
    MyApplication& app = MyApplication::getInstance();
    app.init(sysMgr);
    app.start();
}
```

---

## 📦 ไฟล์ที่สำคัญ

```
main/
├── main.cpp                          # ✅ Refactored - Entry point (กระชับ)
├── main.cpp.backup_original          # 🔙 Backup ของไฟล์เดิม
├── include/
│   ├── SystemManager.h               # 🆕 System management
│   └── VPIApplication.h              # 🆕 VPI application
└── cpp_version/
    ├── SystemManager.cpp             # 🆕 System implementation
    └── VPIApplication.cpp            # 🆕 VPI application implementation
```

---

## 🎯 ข้อดีของโครงสร้างใหม่

### 1. **Separation of Concerns**
- System initialization แยกออกจาก app logic
- แต่ละ class มีหน้าที่ชัดเจน

### 2. **Reusability**
- `SystemManager` ใช้ได้กับทุก ESP32-P4 HMI project
- แค่สร้าง Application class ใหม่สำหรับแต่ละ project

### 3. **Maintainability**
- main.cpp กระชับ อ่านง่าย
- Code จัดเป็นระเบียบ หาแก้ไขง่าย

### 4. **Testability**
- แต่ละ component แยกทดสอบได้
- Mock SystemManager สำหรับทดสอบ Application

### 5. **Scalability**
- เพิ่ม feature ใหม่ใน Application class
- ไม่กระทบ SystemManager

---

## 🔧 Build & Flash

```powershell
# Build project
idf build

# Flash to device
idf -p COM5 flash

# Monitor output
idf -p COM5 monitor

# Flash and monitor
idf -p COM5 flash monitor
```

---

## 📝 ตัวอย่าง: สร้าง Simple Clock Application

```cpp
// SimpleClock.h
class SimpleClock {
public:
    static SimpleClock& getInstance();
    esp_err_t init(SystemManager& sysMgr);
    esp_err_t start();
    
private:
    SimpleClock();
    void createClockUI();
    static void onTimerTick(lv_timer_t* timer);
    
    SystemManager* m_sysMgr;
    lv_obj_t* m_clock_label;
    lv_timer_t* m_timer;
};

// SimpleClock.cpp
SimpleClock& SimpleClock::getInstance() {
    static SimpleClock instance;
    return instance;
}

esp_err_t SimpleClock::init(SystemManager& sysMgr) {
    m_sysMgr = &sysMgr;
    
    if (m_sysMgr->lockDisplay()) {
        createClockUI();
        m_sysMgr->unlockDisplay();
    }
    
    return ESP_OK;
}

void SimpleClock::createClockUI() {
    m_clock_label = lv_label_create(lv_scr_act());
    lv_obj_set_align(m_clock_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(m_clock_label, &lv_font_montserrat_48, 0);
}

esp_err_t SimpleClock::start() {
    m_timer = lv_timer_create(onTimerTick, 1000, nullptr);
    return ESP_OK;
}

void SimpleClock::onTimerTick(lv_timer_t* timer) {
    SimpleClock& app = getInstance();
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
    
    lv_label_set_text(app.m_clock_label, time_str);
}

// main.cpp
extern "C" void app_main(void) {
    SystemManager& sysMgr = SystemManager::getInstance();
    sysMgr.initDisplay();
    
    SimpleClock& app = SimpleClock::getInstance();
    app.init(sysMgr);
    app.start();
}
```

---

## 🎓 สรุป

โครงสร้างใหม่นี้ทำให้:
- ✅ Code สะอาด อ่านง่าย บำรุงรักษาง่าย
- ✅ แยก concerns ชัดเจน (System vs Application)
- ✅ นำกลับไปใช้ได้ (Reusable SystemManager)
- ✅ ขยายง่าย (Extensible architecture)
- ✅ ทดสอบง่าย (Testable components)

**สามารถใช้เป็น template สำหรับสร้าง ESP32-P4 HMI project ใหม่ๆ ได้ทันที! 🚀**
