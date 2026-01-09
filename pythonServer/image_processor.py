from PIL import Image, ImageDraw
import requests
from io import BytesIO
import numpy as np

# Palette for 7-color E-Ink (approximate RGB values)
# Order: Black, White, Green, Blue, Red, Yellow, Orange
PALETTE_RGB = [
    0, 0, 0,        # 0: Black
    255, 255, 255,  # 1: White
    0, 255, 0,      # 2: Green
    0, 0, 255,      # 3: Blue
    255, 0, 0,      # 4: Red
    255, 255, 0,    # 5: Yellow
    255, 165, 0,    # 6: Orange
]

def get_palette_image():
    p_img = Image.new('P', (1, 1))
    # Pad palette to 256 colors (768 ints)
    full_palette = PALETTE_RGB + [0] * (768 - len(PALETTE_RGB))
    p_img.putpalette(full_palette)
    return p_img

def add_rounded_corners(img, radius, bg_color):
    mask = Image.new('L', img.size, 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle((0, 0) + img.size, radius, fill=255)
    
    # Usamos el color dominante como fondo para las esquinas
    bg = Image.new('RGB', img.size, bg_color)
    bg.paste(img, (0, 0), mask)
    return bg

def process_image(url, width=414, height=450):
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        img = Image.open(BytesIO(response.content))
        
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        img_ratio = img.width / img.height
        target_ratio = width / height
        if img_ratio > target_ratio:
            new_height = height
            new_width = int(new_height * img_ratio)
        else:
            new_width = width
            new_height = int(new_width / img_ratio)
        img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
        left = (new_width - width) // 2
        top = (new_height - height) // 2
        img = img.crop((left, top, left + width, top + height))
        
        palette_img = get_palette_image()
        temp_q = img.quantize(palette=palette_img, dither=0)
        pixels_temp = np.array(temp_q, dtype=np.uint8)
        counts = np.bincount(pixels_temp.flatten(), minlength=7)
        counts[1] = 0 # Ignorar blanco
        dom_idx = int(np.argmax(counts))
        
        dom_rgb = tuple(PALETTE_RGB[dom_idx*3 : dom_idx*3 + 3])
        
        img = add_rounded_corners(img, 30, dom_rgb)
        
        img_q = img.quantize(palette=palette_img, dither=1)
        pixels = np.array(img_q, dtype=np.uint8)
        
        pixels_flat = pixels.flatten()
        high = pixels_flat[0::2] << 4
        low = pixels_flat[1::2] & 0x0F
        packed = high | low
        
        return packed.tobytes(), dom_idx
    except:
        print("Errror bleh")
