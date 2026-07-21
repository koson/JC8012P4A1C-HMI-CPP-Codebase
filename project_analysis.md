# รายงานการวิเคราะห์โปรเจค LabBuddy-JC8012P4-HMI

โปรเจค **LabBuddy-JC8012P4-HMI** เป็นซอฟต์แวร์ระบบแสดงผลและสั่งการ (Human-Machine Interface - HMI) ที่พัฒนาขึ้นสำหรับอุปกรณ์ **LabBuddy** (เครื่องมือสนับสนุนการเรียนรู้การทดสอบวงจรและไอซีในห้องแล็บ) โดยพัฒนาบนบอร์ดไมโครคอนโทรลเลอร์ประสิทธิภาพสูง **ESP32-P4** (Dual-core RISC-V, 360 MHz) ร่วมกับหน้าจอสัมผัสขนาด 7 นิ้วผ่านอินเทอร์เฟซ MIPI-DSI

---

## 🏗️ สถาปัตยกรรมระบบโดยรวม (System Architecture)

ระบบประกอบด้วยโครงสร้างการทำงานหลัก 3 ระดับ:

```mermaid
graph TD
    subgraph Hardware Layer
        P4[ESP32-P4 Main CPU]
        C6[ESP32-C6 WiFi Coprocessor]
        STM[STM32H7 Measurement Unit]
        SD[SD Card Reader]
        LCD[MIPI-DSI 7" LCD & Touch]
    end

    subgraph Core Abstraction Layer (C++ OOP Wrapper)
        LVW[LVWidget Base Class]
        LVS[Screen / Style / Event / Timer Managers]
        JSR[JsonRenderer & SvgRenderer]
    end

    subgraph Application Layer
        NAV[HMINavigator Screen Manager]
        FM[FileManagerApplication Web UI]
        LP[LessonPlayer Course Engine]
        UB[UartBridge / VerificationEngine]
    end

    P4 --> LVW
    LCD --> LVS
    C6 <-->|SDIO / ESP-Hosted| FM
    STM <-->|UART / USB CDC| UB
    SD <-->|FATFS| FM
    SD <-->|FATFS| LP
    LVW --> NAV
    NAV --> LP
    NAV --> FM
    UB --> LP
```

---

## 📂 ส่วนประกอบสำคัญและรายละเอียดไฟล์ (Core Components)

### 1. C++ OOP Wrapper สำหรับ LVGL v9 (`components/lvgl_cpp_v9`)
โปรเจคนี้มีการหุ้ม (Wrap) C API ดั้งเดิมของ LVGL v9 ให้กลายเป็น Class ในภาษา C++ ในรูปแบบเชิงวัตถุ (Object-Oriented) คล้ายกับ Qt Framework หรือ MFC เพื่อความปลอดภัยในการจัดการประเภทข้อมูล (Type Safety), การจัดการหน่วยความจำแบบ RAII และช่วยเพิ่มความสะดวกในเขียนโค้ดด้วย Method Chaining (Fluent API)
*   **Base Class**: [LVWidget.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVWidget.hpp) ทำหน้าที่จัดการ `lv_obj_t*` ของ LVGL, สถานะแสดงผล, ตลอดจนการผูกเหตุการณ์ (Event Handling)
*   **Derived Widgets**: คลุม widget ของ LVGL ถึง 24 ชนิด เช่น [LVButton.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVButton.hpp), [LVLabel.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVLabel.hpp), [LVSlider.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVSlider.hpp) และ [LVCanvas.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVCanvas.hpp)
*   **System Managers**:
    *   [LVDisplay.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVDisplay.hpp) & [ScreenManager.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/ScreenManager.hpp) จัดการการสลับหน้าจอ (Transition) และการหมุนจอ (Rotation)
    *   [LVEvent.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVEvent.hpp) แปลงเหตุการณ์ของ LVGL ให้สามารถใช้ร่วมกับ `std::function` และ Lambda Expressions ของ C++ ได้อย่างสมบูรณ์
    *   [LVTimer.hpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/LVTimer.hpp) จัดการการทำงานแบบขนานแบบ Periodic

### 2. ระบบจัดการไฟล์และ WiFi (`FileManagerApplication`)
*   **WiFi (ESP-Hosted)**: ควบคุมโมดูลร่วม ESP32-C6 ผ่านบัส SDIO เพื่อรับคำสั่ง WiFi ทำให้ ESP32-P4 สามารถใช้งานการเชื่อมต่อเครือข่ายได้โดยไม่ต้องแบกรับภาระโปรโตคอลสแต็ค
*   **HTTP File Server**: สร้าง HTTP Server ภายในขึ้นมาใน [FileManagerApplication.cpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/cpp_version/FileManagerApplication.cpp) เพื่อให้บริการ Web UI สำหรับอัปโหลด ดาวน์โหลด หรือลบไฟล์แบบไร้สายไปยัง SD Card (`/sdcard/lessons` หรือ `/sdcard/WORKSHOP`)
*   **FileViewerUI**: หน้าจอสำหรับเบราส์รายการไฟล์และควบคุมสั่งเรนเดอร์เนื้อหาจากไฟล์ JSON/SVG บนหน้าจอ HMI โดยตรง

### 3. ระบบแสดงเนื้อหาและการทำแล็บ (`LessonPlayer` & `Renderer`)
*   **LessonPlayer**: คลาส [LessonPlayer.cpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/cpp_version/LessonPlayer.cpp) ทำหน้าที่เป็น Course Engine อ่านไฟล์บทเรียนที่เป็นโครงสร้าง JSON ดึงคำสั่งมาเล่นตามขั้นตอน (Multi-page interactive lesson player)
*   **Renderers**:
    *   `svg_renderer`: เรนเดอร์รูปกราฟิกเวกเตอร์จากไฟล์ SVG (มักใช้แสดงแผนภาพวงจร/ขั้วเชื่อมต่อ)
    *   `json_renderer`: เรนเดอร์ข้อมูลวงจรที่มีการระบุพินและการจำลองสถานะ

### 4. ตัวกลางสื่อสารและประมวลผลการทดสอบ (`UartBridge` & `VerificationEngine`)
*   **UartBridge / USB ACM Host**: ทำหน้าที่รับ-ส่งคำสั่งผ่านซีเรียลระหว่าง ESP32-P4 และชิปวัดผลร่วมภายนอก (STM32H7) โดยส่งพารามิเตอร์จำพวกแรงดัน, กระแสไฟฟ้า หรือเวกเตอร์ลอจิก และรับผลการทดสอบที่ตรวจวัดได้กลับมา
*   **VerificationEngine**: จัดการกระบวนการทดสอบไอซีเปรียบเทียบกับตารางความจริง (Truth-Table-based IC Verification) ร่วมกับข้อมูลที่ได้มาจากการติดต่อผ่าน UartBridge

---

## 🔄 ขั้นตอนการทำงานหลัก (App Entry & Workflow)

เมื่อระบบบูตผ่าน [main.cpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/main.cpp):
1.  **System Initialization**: เรียกใช้ [SystemManager](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/cpp_version/SystemManager.cpp) ในการเริ่มทำงานของหน้าจอแสดงผล (`initDisplay()`), ปรับทิศทางการแสดงผล (หมุนจอ 270 องศาเป็นแนวนอน) และเตรียมพอร์ตสัมผัส (Touchscreen)
2.  **App Startup**: เริ่มการเชื่อมต่อ WiFi ผ่านบอร์ดเสริม ESP32-C6 และบูต Web File Manager ขึ้นมาในพื้นหลัง (Background Task)
3.  **SD Card Mount**: ทำการเมานต์การ์ดความจำเพื่อใช้เก็บไฟล์ล็อก (`/sdcard/logs`), บทเรียน (`/sdcard/lessons`) และพื้นที่ทำงาน (`/sdcard/WORKSHOP`)
4.  **UI Navigation (ปกติ)**: เปิดหน้าจอเริ่มต้นผ่าน [HMINavigator](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/cpp_version/HMINavigator.cpp) โดยเริ่มจาก **Splash Screen** -> หน่วงเวลาสลับไป **Home Screen** -> เข้าสู่ **Library Screen** เพื่อดึงรายการบทเรียนและไฟล์ SVG/JSON มาแสดงผล

---

## ⚡ จุดเด่นการวิจัยและพัฒนาของโปรเจคนี้ (Strengths & Workflows)

*   **Rapid Development Cycle (Option 1.5)**: 
    กระบวนการทำงานเพื่ออัปเดต UI/กราฟิกถูกออกแบบให้อัปเดตได้ภายใน 30 วินาทีโดยไม่ต้องแฟลชเฟิร์มแวร์ใหม่ทุกครั้ง:
    `วาด SVG ใน Inkscape` ➔ `แปลงเป็น JSON ด้วย Python Script` ➔ `อัปโหลดผ่านเว็บเบราว์เซอร์ไร้สาย` ➔ `กด Refresh & Render ที่หน้าจอ HMI` เพื่อตรวจดูความถูกต้องของภาพแบบ Real-time
*   **Clean Separation of Concerns**:
    มีการแยกเฟรมเวิร์กส่วนติดต่อระบบภายนอก (`SystemManager`), คอมโพเนนต์ตัววาดและการติดต่อไอซีทดสอบ (`UartBridge`, `VerificationEngine`) และส่วนควบคุมหน้าจอระบบสัมผัส (`HMINavigator`, `ScreenManager`) ออกจากกันอย่างเป็นระเบียบตามหลัก OOP
*   **Thai Character Rendering**:
    มีการรองรับการแสดงผลฟอนต์ภาษาไทยผ่านโมดูล `font_thai` (เช่น TH Niramit) ร่วมกับการเรนเดอร์ฉลากข้อความใน [ThaiLabel.h](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/components/lvgl_cpp_v9/include/ThaiLabel.h)

---

## 🧪 การทดสอบระบบ (Verification and Testing)

โปรเจคนี้ใช้เฟรมเวิร์ก **Unity** สำหรับรัน Unit Test บนตัวอุปกรณ์เป้าหมาย (On-target Testing) ซึ่งมีการเขียนโค้ดทดสอบไว้ เช่น:
*   `tests/test_json_basic.cpp` - ตรวจสอบความถูกต้องของการแปลงพาสซีฟโครงสร้าง JSON
*   `tests/test_svg_basic.cpp` - ตรวจสอบการพาร์ส SVG Path และการแปลงพิกัดจุดเวกเตอร์
*   `tests/test_lesson_fetch_service.cpp` - ตรวจสอบ HTTP Client ในการดึงบทเรียนใหม่ๆ จากเครือข่าย

นอกจากนี้ยังมีตัวเลือกในการรันชุดทดสอบด้วยการตั้งค่า `#define RUN_TESTS 1` ภายใน [main.cpp](file:///d:/GitHubRepos/LabBuddy/Firmware/LabBuddy-JC8012P4-HMI/main/main.cpp) ซึ่งจะสลับระบบไปรันฟังก์ชัน `run_all_tests()` ทันทีเมื่อบูตเสร็จ
