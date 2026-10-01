import os
from PIL import Image, ImageOps

source_image_path = r"C:\Users\xiphos-pc1\.gemini\antigravity-ide\brain\6de7ef09-6296-4e8b-9b16-d598d08c91fb\retrohub_pwa_icon_1790887338451.jpg"
output_dir = r"c:\Projects\DevilutionX\Packaging\emscripten\assets\icons"

os.makedirs(output_dir, exist_ok=True)

img = Image.open(source_image_path).convert("RGBA")

# 1. Standard Square Icons
sizes = [32, 64, 128, 180, 192, 256, 512]
for s in sizes:
    resized = img.resize((s, s), Image.Resampling.LANCZOS)
    if s == 180:
        resized.save(os.path.join(output_dir, "apple-touch-icon.png"), "PNG")
    elif s == 32:
        resized.save(os.path.join(output_dir, "favicon.png"), "PNG")
        resized.save(os.path.join(output_dir, "icon-32.png"), "PNG")
    else:
        resized.save(os.path.join(output_dir, f"icon-{s}.png"), "PNG")

# 2. Maskable Icons (Android PWA adaptive icons require 10-15% padding so corners aren't clipped)
for s in [192, 512]:
    inner_size = int(s * 0.8)
    margin = (s - inner_size) // 2
    # Sample background color from outer border
    bg_color = (12, 16, 26, 255)
    maskable = Image.new("RGBA", (s, s), bg_color)
    inner_img = img.resize((inner_size, inner_size), Image.Resampling.LANCZOS)
    maskable.paste(inner_img, (margin, margin), inner_img)
    maskable.save(os.path.join(output_dir, f"icon-maskable-{s}.png"), "PNG")

print("Successfully generated all RetroHub PWA icons in:", output_dir)
