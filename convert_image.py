#!/usr/bin/env python3
"""
Simple script to convert PNG to PCX format for DevilutionX
"""
from PIL import Image
import sys
import os

def convert_png_to_pcx(png_path, pcx_path):
    try:
        # Open the PNG image
        img = Image.open(png_path)
        
        # Convert to RGB if it has transparency
        if img.mode in ('RGBA', 'LA'):
            # Create a black background
            background = Image.new('RGB', img.size, (0, 0, 0))
            if img.mode == 'RGBA':
                background.paste(img, mask=img.split()[-1])  # Use alpha channel as mask
            else:
                background.paste(img, mask=img.split()[-1])
            img = background
        elif img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Resize to common Diablo resolution if needed (640x480 or similar)
        # Keep aspect ratio
        max_width, max_height = 640, 480
        img.thumbnail((max_width, max_height), Image.Resampling.LANCZOS)
        
        # Save as PCX
        img.save(pcx_path, 'PCX')
        print(f"Successfully converted {png_path} to {pcx_path}")
        return True
        
    except Exception as e:
        print(f"Error converting image: {e}")
        return False

if __name__ == "__main__":
    png_file = "de905cb9-1c04-4fc4-a24d-91399764d69f.png"
    pcx_file = "assets/ui_art/title.pcx"
    
    if os.path.exists(png_file):
        convert_png_to_pcx(png_file, pcx_file)
    else:
        print(f"PNG file {png_file} not found")