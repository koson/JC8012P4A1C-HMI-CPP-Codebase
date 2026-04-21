#!/usr/bin/env python3
"""
Screenshot Receiver for ESP32 LVGL
Receives BMP image data via USB/UART and saves to file
"""

import serial
import serial.tools.list_ports
import sys
from datetime import datetime
import os

def receive_screenshot(port='COM49', baudrate=115200, output_dir='screenshots'):
    """
    Receive screenshot from ESP32 via serial port
    
    Args:
        port: Serial port (e.g., 'COM49' on Windows, '/dev/ttyUSB0' on Linux)
        baudrate: Serial baudrate (default: 115200)
        output_dir: Directory to save screenshots
    """
    
    # Create output directory if not exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    print(f"Opening serial port {port} at {baudrate} baud...")
    
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port}")
        print("Waiting for screenshot data...")
        print("Press Ctrl+C to exit\n")
        
        receiving = False
        data = bytearray()
        
        while True:
            try:
                line = ser.readline()
                
                if not receiving:
                    # Look for start marker
                    if b'>>>SCREENSHOT_START<<<' in line:
                        print("Screenshot transfer started...")
                        receiving = True
                        data = bytearray()
                    else:
                        # Print other serial output
                        try:
                            print(line.decode('utf-8', errors='ignore').rstrip())
                        except:
                            pass
                else:
                    # Check for end marker
                    if b'>>>SCREENSHOT_END<<<' in line:
                        print("Screenshot transfer completed!")
                        
                        # Generate filename with timestamp
                        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
                        filename = os.path.join(output_dir, f'screenshot_{timestamp}.bmp')
                        
                        # Save to file
                        with open(filename, 'wb') as f:
                            f.write(data)
                        
                        file_size = len(data)
                        print(f"Saved to: {filename}")
                        print(f"File size: {file_size:,} bytes ({file_size/1024:.1f} KB)\n")
                        print("Waiting for next screenshot...")
                        
                        receiving = False
                        data = bytearray()
                    else:
                        # Accumulate data
                        data.extend(line)
                        
                        # Show progress every 10KB
                        if len(data) % 10240 < 256:
                            print(f"\rReceiving... {len(data):,} bytes", end='', flush=True)
                
            except KeyboardInterrupt:
                print("\n\nExiting...")
                break
            except Exception as e:
                print(f"\nError: {e}")
                receiving = False
                data = bytearray()
        
        ser.close()
        print("Serial port closed")
        
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        print("\nAvailable ports:")
        available_ports = serial.tools.list_ports.comports()
        for p in available_ports:
            print(f"  {p.device} - {p.description}")
        sys.exit(1)

if __name__ == '__main__':
    # Default settings
    port = 'COM3'  # Change this to your ESP32's port
    baudrate = 115200
    
    # Parse command line arguments
    if len(sys.argv) > 1:
        port = sys.argv[1]
    if len(sys.argv) > 2:
        baudrate = int(sys.argv[2])
    
    print("=" * 60)
    print("ESP32 LVGL Screenshot Receiver")
    print("=" * 60)
    print(f"Port: {port}")
    print(f"Baudrate: {baudrate}")
    print("=" * 60)
    print()
    
    receive_screenshot(port, baudrate)
