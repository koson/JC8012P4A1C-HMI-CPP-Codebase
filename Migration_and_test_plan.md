## สถานการณ์ปัจจุบัน

เรามีโปรเจค LabBuddy-JC8012P4-HMI ที่รันบน ESP32P4 โดยใช้จอ JC8012P4 โดยใช้ LVGL 9
แต่ในโลกนี้ยังมีโปรเจคที่สามารถจำลอง LVGL รันบน  Windows ให้มาทดสอบได้ ซุ่งจะช่วยให้ round trip ของการพัฒนาเร็วขึ้นมาก




**คำถาม:**
1. เราจะสามารถย้ายหรือรวมโปรเจคนี้ไปยัง  Windows ได้อย่างไร โดยที่ยังคงใช้ LVGL 9 และ HMI  (GUI) เป็นหลัก
2. ระหว่างโปรเจค visual studio กับ visual studio code ควรเลือกใช้อะไรดี ที่ให้ agy สามารถช่วยพัฒนาและ migrate ได้อย่างสมบูรณ์แบบ

2.1 https://github.com/lvgl/lv_port_pc_visual_studio

2.2 https://github.com/lvgl/lv_port_pc_vscode


3. การรันบน visual studio สามารถ debug ได้ง่ายและสะดวก แต่ไม่มี AGY ในตัว
4. การรันบน antigravity  โดยใช้  port ของ lvgl ที่เป็น visual studio code สามารถใช้งาน AGY ได้ดีเยี่ยมแต่ debug ลำบาก



**เป้าหมาย:**
1. พัฒนาโปรแกรมบน Windows ให้ได้ HMI ตามที่ออกแบบ
2. สามารถรันโปรแกรมบน ESP32P4 และจอ JC8012P4 ได้
3. สามารถสลับการทำงานระหว่าง Windows และ ESP32P4 ได้ โดยที่ HMI ยังคงเหมือนเดิม

**ข้อจำกัด:**
1. ต้องใช้ LVGL 9
2. ต้องใช้ ESP32P4
3. ต้องใช้จอ JC8012P4
4. ต้องรันบน Windows ได้
