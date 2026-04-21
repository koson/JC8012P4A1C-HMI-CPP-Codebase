# Screenshot Feature - Quick Guide

## วิธีใช้งาน Screenshot

### 1. รัน Python Script บน PC

เปิด Terminal/Command Prompt และรัน:

```bash
cd D:\GitHubRepos\__AES\LVGL_V9_OOP\LVGL_CPP_DEV\tools
python screenshot_receiver.py COM3
```

**หมายเหตุ:** เปลี่ยน `COM3` เป็น port ที่ ESP32 เชื่อมต่ออยู่

### 2. กดปุ่ม Screenshot บน ESP32

- หน้าจอจะมีปุ่ม "📷 Screenshot" ที่ด้านล่าง
- กดปุ่มนี้เมื่อต้องการ capture หน้าจอ
- ภาพจะถูกส่งผ่าน USB มายัง PC

### 3. ภาพจะถูกบันทึกอัตโนมัติ

- ไฟล์จะถูกบันทึกใน folder `screenshots/`
- ชื่อไฟล์: `screenshot_YYYYMMDD_HHMMSS.bmp`
- ตัวอย่าง: `screenshot_20260115_143052.bmp`

## ติดตั้ง Dependencies

ถ้ายังไม่มี pyserial:

```bash
pip install pyserial
```

## การใช้งานบน Linux/Mac

```bash
python3 screenshot_receiver.py /dev/ttyUSB0
```

## Features

- ✅ รับภาพ BMP 24-bit RGB
- ✅ รองรับหน้าจอขนาดใดก็ได้
- ✅ Auto-save พร้อม timestamp
- ✅ แสดง progress ขณะรับข้อมูล
- ✅ รอรับภาพหลายภาพต่อเนื่อง

## ขนาดไฟล์โดยประมาณ

- 1024×600: ~1.8 MB
- 800×480: ~1.1 MB
- 480×320: ~450 KB
