# Testing Guide for LVGL C++ OOP
## From Zero to Hero - เรียนรู้ Testing แบบค่อยเป็นค่อยไป

---

## ทำไมต้อง Test?

**ปัญหาที่เจอถ้าไม่ Test:**
- 😱 Bug ซ่อนอยู่ จนกว่าจะ deploy จริง
- 🔥 แก้ feature A → ทำให้ feature B พัง
- 💸 ใช้เวลามากในการ debug
- 😰 ไม่มั่นใจว่าโค้ดทำงานถูกต้อง

**ประโยชน์ของการ Test:**
- ✅ มั่นใจว่าโค้ดทำงานถูกต้อง
- ✅ จับ bug เร็ว (ก่อนมันกลายเป็นปัญหาใหญ่)
- ✅ Refactor โค้ดได้ไม่กลัว
- ✅ Documentation ที่ดีที่สุด (test = วิธีใช้จริง)

---

## ระดับของการ Test

### 1. Manual Testing (ง่ายที่สุด - เริ่มที่นี่)
**คือ:** ทดสอบด้วยตาเอง กดๆ ดูว่าทำงานไหม

**ตัวอย่าง:**
```cpp
// สร้าง button แล้วกดดู
LVButton btn(screen);
btn.setText("Click Me");

// Test Case (ทดสอบด้วยมือ):
// 1. กดปุ่ม → ควรเห็นเอฟเฟค pressed
// 2. ปล่อยปุ่ม → ควรกลับสู่สถานะปกติ
// 3. ดู console → ควรเห็น "Button clicked"
```

**ข้อดี:**
- ✅ ง่าย ไม่ต้องเขียนโค้ดเพิ่ม
- ✅ เห็นผลจริงบนหน้าจอ
- ✅ เหมาะกับ UI testing

**ข้อเสีย:**
- ❌ ช้า (ต้องทดสอบทุกครั้งด้วยมือ)
- ❌ น่าเบื่อ (ทำซ้ำๆ)
- ❌ พลาดได้ง่าย (ลืมทดสอบบางอย่าง)

### 2. Automated Testing (ระดับกลาง)
**คือ:** เขียนโค้ดเพื่อทดสอบโค้ด

**ตัวอย่าง:**
```cpp
void test_button_creation() {
    LVButton btn(lv_screen_active());
    
    // Assert: ตรวจสอบว่าสร้างสำเร็จ
    assert(btn.getObject() != nullptr);
    printf("✅ Test passed: Button created\n");
}
```

### 3. Unit Testing (ระดับสูง - ทดสอบทีละชิ้นส่วน)
**คือ:** ทดสอบแต่ละ class/function แยกอิสระ

**ตัวอย่าง:**
```cpp
TEST(LVSlider, SetValue) {
    LVSlider slider(screen);
    slider.setValue(75);
    
    EXPECT_EQ(slider.getValue(), 75);
}
```

---

## การ Test ใน LVGL C++ OOP - เริ่มจาก Manual Testing

### Phase 2: Widget Testing Strategy

เราจะใช้วิธี **Systematic Manual Testing** ก่อน เพราะ:
1. ✅ เหมาะกับการทดสอบ UI
2. ✅ เห็นผลชัดเจน
3. ✅ ไม่ซับซ้อน
4. ✅ จับ visual bugs ได้

### Test Case Template

**โครงสร้าง Test Case:**
```
TC-XXX: <ชื่อ Test Case>
---
Objective:     <ทดสอบอะไร>
Precondition:  <เงื่อนไขก่อนทดสอบ>
Steps:         <ขั้นตอนการทดสอบ>
Expected:      <ผลที่คาดหวัง>
Actual:        <ผลที่ได้จริง - เติมตอน test>
Status:        <Pass/Fail>
Notes:         <ข้อสังเกต/bugs>
```

---

## ตัวอย่างจริง: Brightness Control Testing

### Test Plan Overview

**Widget Under Test:** LVSlider, LVPanel, LVLabel

**Test Objectives:**
1. Verify LVSlider value change callback
2. Verify LVPanel color updates
3. Verify LVLabel text updates
4. Check performance (smooth operation)
5. Check memory (no leaks)

### Test Cases

#### TC-001: Slider Minimum Value
```
Objective:     Test slider at minimum value (0)
Precondition:  Brightness Control example running
Steps:
  1. Drag slider to leftmost position
  2. Observe panel color
  3. Read label text
Expected:
  - Slider value = 0
  - Panel color = dark gray (RGB: 48,48,48)
  - Label text = "Brightness: 0%"
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

#### TC-002: Slider Maximum Value
```
Objective:     Test slider at maximum value (100)
Precondition:  Brightness Control example running
Steps:
  1. Drag slider to rightmost position
  2. Observe panel color
  3. Read label text
Expected:
  - Slider value = 100
  - Panel color = light gray (RGB: 240,240,240)
  - Label text = "Brightness: 100%"
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

#### TC-003: Slider Mid Value
```
Objective:     Test slider at middle value (50)
Precondition:  Brightness Control example running
Steps:
  1. Drag slider to middle position
  2. Observe panel color
  3. Read label text
Expected:
  - Slider value = 50
  - Panel color = medium gray (RGB: 144,144,144)
  - Label text = "Brightness: 50%"
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

#### TC-004: Rapid Slider Movement
```
Objective:     Test UI responsiveness with rapid changes
Precondition:  Brightness Control example running
Steps:
  1. Rapidly drag slider back and forth
  2. Observe UI smoothness
  3. Check for lag or freeze
Expected:
  - UI remains smooth
  - No visible lag
  - All updates happen in real-time
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

#### TC-005: Continuous Update
```
Objective:     Test continuous callback execution
Precondition:  Brightness Control example running
Steps:
  1. Touch and hold slider knob
  2. Slowly drag across full range
  3. Observe console output
Expected:
  - Continuous printf output
  - Smooth color transition
  - No callback skips
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

#### TC-006: Memory Leak Check
```
Objective:     Verify no memory leaks during operation
Precondition:  ESP32 running, monitor heap
Steps:
  1. Note initial free heap
  2. Move slider 100 times
  3. Note final free heap
Expected:
  - Free heap remains stable
  - No continuous decrease
  - Difference < 1KB acceptable
Actual:        [Fill during test]
Status:        [ ] Pass  [ ] Fail
Notes:         
```

---

## How to Execute Tests

### Step 1: Build & Flash
```powershell
cd D:\GitHubRepos\__AES\LVGL_V9_OOP\LVGL_CPP_DEV
idf.py build flash monitor
```

### Step 2: Prepare Test Checklist
```
Brightness Control Test Session - [Date]
---
[ ] TC-001: Slider Minimum Value
[ ] TC-002: Slider Maximum Value
[ ] TC-003: Slider Mid Value
[ ] TC-004: Rapid Slider Movement
[ ] TC-005: Continuous Update
[ ] TC-006: Memory Leak Check

Summary:
- Total: 6
- Pass: __
- Fail: __
- Issues Found: ______________________________
```

### Step 3: Execute Each Test Case

**Example: Running TC-001**
```
1. Device ready? ✅
2. Brightness Control running? ✅
3. Drag slider to left → DONE
4. Panel dark gray? ✅
5. Label shows "Brightness: 0%"? ✅
6. Console shows correct RGB? ✅

Result: PASS ✅
```

### Step 4: Document Results

**Test Report Template:**
```markdown
# Test Report: Brightness Control
**Date:** 2026-01-13
**Tester:** Your Name
**Build:** v0.1.0

## Summary
- Total Test Cases: 6
- Passed: 5
- Failed: 1
- Pass Rate: 83%

## Failed Tests
TC-004: Rapid Slider Movement
- Issue: UI lags when moving very fast
- Severity: Medium
- Workaround: Use animation to smooth transitions

## Notes
- Overall widget behavior is good
- Need to optimize callback performance
- Consider debouncing rapid changes
```

---

## Advanced: Programmatic Testing

### Automated Test Functions

**Example: Automated Property Tests**
```cpp
void test_slider_properties() {
    printf("\n=== Testing LVSlider Properties ===\n");
    
    LVSlider slider(lv_screen_active());
    
    // Test 1: Set range
    slider.setRange(0, 100);
    printf("Test 1: Set range 0-100... ");
    // No way to verify directly in LVGL, but check it doesn't crash
    printf("✅ PASS\n");
    
    // Test 2: Set value
    slider.setValue(75, LV_ANIM_OFF);
    int32_t value = slider.getValue();
    printf("Test 2: Set value to 75... ");
    if (value == 75) {
        printf("✅ PASS (value=%ld)\n", value);
    } else {
        printf("❌ FAIL (expected=75, actual=%ld)\n", value);
    }
    
    // Test 3: Boundary test - minimum
    slider.setValue(0, LV_ANIM_OFF);
    value = slider.getValue();
    printf("Test 3: Minimum value (0)... ");
    if (value == 0) {
        printf("✅ PASS\n");
    } else {
        printf("❌ FAIL (actual=%ld)\n", value);
    }
    
    // Test 4: Boundary test - maximum
    slider.setValue(100, LV_ANIM_OFF);
    value = slider.getValue();
    printf("Test 4: Maximum value (100)... ");
    if (value == 100) {
        printf("✅ PASS\n");
    } else {
        printf("❌ FAIL (actual=%ld)\n", value);
    }
    
    // Test 5: Out of range (should clamp)
    slider.setValue(150, LV_ANIM_OFF);
    value = slider.getValue();
    printf("Test 5: Out of range (150)... ");
    if (value == 100) {
        printf("✅ PASS (clamped to 100)\n");
    } else {
        printf("❌ FAIL (expected=100, actual=%ld)\n", value);
    }
    
    printf("\n=== Test Complete ===\n\n");
}
```

### Memory Leak Detection

**Using ESP32 Heap Monitoring:**
```cpp
void test_memory_leak() {
    printf("\n=== Memory Leak Test ===\n");
    
    // Get initial heap
    size_t heap_before = esp_get_free_heap_size();
    printf("Initial heap: %u bytes\n", heap_before);
    
    // Create and destroy widgets 100 times
    for (int i = 0; i < 100; i++) {
        LVSlider* slider = new LVSlider(lv_screen_active());
        slider->setRange(0, 100);
        slider->setValue(50, LV_ANIM_OFF);
        delete slider;
    }
    
    // Force garbage collection (if needed)
    lv_obj_clean(lv_screen_active());
    
    // Get final heap
    size_t heap_after = esp_get_free_heap_size();
    printf("Final heap: %u bytes\n", heap_after);
    
    // Calculate difference
    int32_t diff = heap_before - heap_after;
    printf("Heap difference: %ld bytes\n", diff);
    
    // Verdict
    if (abs(diff) < 1024) {  // Allow 1KB tolerance
        printf("✅ PASS: No significant memory leak\n");
    } else {
        printf("❌ FAIL: Potential memory leak detected\n");
    }
    
    printf("\n=== Test Complete ===\n\n");
}
```

---

## Test Automation Roadmap

### Phase 2: Manual Testing (Current)
- ✅ Create test cases for each widget
- ✅ Execute manually
- ✅ Document results
- ✅ Fix bugs found

### Phase 3: Semi-Automated Testing
- [ ] Create test helper functions
- [ ] Automate property verification
- [ ] Automate memory leak detection
- [ ] Console output validation

### Phase 4: Full Automation (Future)
- [ ] Integrate unit testing framework (Unity/Google Test)
- [ ] CI/CD pipeline
- [ ] Automated regression testing
- [ ] Code coverage reports

---

## Widget Testing Checklist

### For Each Widget, Test:

**1. Creation & Destruction**
- [ ] Widget creates successfully
- [ ] Widget destructs without crash
- [ ] No memory leak on create/destroy

**2. Properties**
- [ ] Set/Get methods work
- [ ] Boundary values handled correctly
- [ ] Invalid values handled gracefully

**3. Events**
- [ ] Callbacks execute
- [ ] Correct event data passed
- [ ] Multiple callbacks work
- [ ] Callback removal works

**4. Visual**
- [ ] Widget displays correctly
- [ ] Styling applies properly
- [ ] Animations smooth
- [ ] No visual glitches

**5. Performance**
- [ ] Responsive to touch
- [ ] No lag during updates
- [ ] Memory usage stable
- [ ] CPU usage acceptable

---

## Common Issues to Watch For

### Bug Patterns

**1. Memory Leaks**
```cpp
// ❌ BAD: Forgot to delete
LVSlider* slider = new LVSlider(screen);
// ... use slider ...
// Forgot: delete slider;

// ✅ GOOD: Always cleanup
LVSlider* slider = new LVSlider(screen);
// ... use slider ...
delete slider;  // Cleanup
```

**2. Callback Issues**
```cpp
// ❌ BAD: user_data is null
void callback(lv_event_t* e) {
    LVSlider* slider = static_cast<LVSlider*>(lv_event_get_user_data(e));
    slider->getValue();  // CRASH if user_data is null!
}

// ✅ GOOD: Check null
void callback(lv_event_t* e) {
    LVSlider* slider = static_cast<LVSlider*>(lv_event_get_user_data(e));
    if (!slider) return;  // Safety check
    slider->getValue();
}
```

**3. Race Conditions**
```cpp
// ❌ BAD: Deleting widget in its own callback
void onClicked(lv_event_t* e) {
    LVButton* btn = static_cast<LVButton*>(lv_event_get_user_data(e));
    delete btn;  // CRASH! Callback still using this object
}

// ✅ GOOD: Use flag or timer
void onClicked(lv_event_t* e) {
    shouldDeleteButton = true;  // Delete later in main loop
}
```

---

## Test Log Template

**Daily Test Log:**
```markdown
# Test Log - [Date]

## Session Info
- Duration: __ minutes
- Widgets Tested: LVSlider, LVPanel, LVLabel
- Example: Brightness Control

## Tests Executed
1. TC-001: ✅ Pass
2. TC-002: ✅ Pass
3. TC-003: ❌ Fail - [reason]
4. TC-004: ✅ Pass
5. TC-005: ⚠️  Warning - [note]
6. TC-006: ✅ Pass

## Bugs Found
1. **BUG-001**: Slider lags on rapid movement
   - Severity: Medium
   - Reproducible: Yes
   - Workaround: Limit update rate

## Performance Notes
- Free heap before: 150,000 bytes
- Free heap after: 149,800 bytes
- Heap consumed: 200 bytes (acceptable)

## Next Steps
- [ ] Fix BUG-001
- [ ] Retest TC-003
- [ ] Move to next widget: LVSwitch
```

---

## Summary: Testing Philosophy

**Remember:**
1. 🎯 **Test Early** - Don't wait until everything is "done"
2. 🔄 **Test Often** - Every change should be tested
3. 📝 **Document Everything** - Your future self will thank you
4. 🐛 **Bugs Are Normal** - Finding them is SUCCESS, not failure
5. 🚀 **Start Simple** - Manual testing is perfectly fine!

**Quality Formula:**
```
Good Code = Working Code + Tests + Documentation
```

---

**Next Step:** Run Brightness Control example and fill out test cases!

**Last Updated:** January 13, 2026
