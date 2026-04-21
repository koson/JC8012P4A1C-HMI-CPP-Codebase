#!/usr/bin/env python3
"""Check BMP file header"""

import struct

def check_bmp(filename):
    with open(filename, 'rb') as f:
        # Read file header (14 bytes)
        file_header = f.read(14)
        
        if len(file_header) < 14:
            print("File too small!")
            return
        
        # Unpack file header
        signature = file_header[0:2]
        file_size = struct.unpack('<I', file_header[2:6])[0]
        reserved = struct.unpack('<I', file_header[6:10])[0]
        data_offset = struct.unpack('<I', file_header[10:14])[0]
        
        print("=== BMP File Header ===")
        print(f"Signature: {signature}")
        print(f"File size: {file_size} bytes")
        print(f"Reserved: {reserved}")
        print(f"Data offset: {data_offset}")
        
        # Read info header (40 bytes)
        info_header = f.read(40)
        
        if len(info_header) < 40:
            print("Info header too small!")
            return
        
        header_size = struct.unpack('<I', info_header[0:4])[0]
        width = struct.unpack('<i', info_header[4:8])[0]
        height = struct.unpack('<i', info_header[8:12])[0]
        planes = struct.unpack('<H', info_header[12:14])[0]
        bits_per_pixel = struct.unpack('<H', info_header[14:16])[0]
        compression = struct.unpack('<I', info_header[16:20])[0]
        image_size = struct.unpack('<I', info_header[20:24])[0]
        
        print("\n=== BMP Info Header ===")
        print(f"Header size: {header_size}")
        print(f"Width: {width}")
        print(f"Height: {height}")
        print(f"Planes: {planes}")
        print(f"Bits per pixel: {bits_per_pixel}")
        print(f"Compression: {compression}")
        print(f"Image size: {image_size}")
        
        # Check if valid
        print("\n=== Validation ===")
        if signature != b'BM':
            print("❌ Invalid signature! Should be 'BM'")
        else:
            print("✓ Signature OK")
        
        expected_image_size = width * abs(height) * (bits_per_pixel // 8)
        expected_file_size = data_offset + expected_image_size
        
        print(f"Expected image size: {expected_image_size}")
        print(f"Expected file size: {expected_file_size}")
        
        import os
        actual_size = os.path.getsize(filename)
        print(f"Actual file size: {actual_size}")
        
        if actual_size < expected_file_size:
            print(f"❌ File incomplete! Missing {expected_file_size - actual_size} bytes")
        else:
            print("✓ File size OK")

if __name__ == '__main__':
    check_bmp(r'D:\GitHubRepos\__AES\LVGL_V9_OOP\LVGL_CPP_DEV\screenshots\screenshot_20260115_074538.bmp')
