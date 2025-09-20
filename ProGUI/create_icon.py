#!/usr/bin/env python3
from PIL import Image, ImageDraw, ImageFont
import os

# Create a 512x512 image with a blue-to-purple gradient background
width, height = 512, 512
image = Image.new('RGBA', (width, height), color=(0, 0, 0, 0))
draw = ImageDraw.Draw(image)

# Create gradient background
for y in range(height):
    r = int(59 + (y / height) * 70)  # 59 to 129
    g = int(66 + (y / height) * 40)  # 66 to 106
    b = int(255 - (y / height) * 55)  # 255 to 200
    draw.line([(0, y), (width, y)], fill=(r, g, b, 255), width=1)

# Draw a circular background for the icon
center_x, center_y = width // 2, height // 2
radius = min(width, height) // 2 - 20
draw.ellipse((center_x - radius, center_y - radius, center_x + radius, center_y + radius),
             fill=(240, 240, 240, 230), outline=(255, 255, 255, 255), width=5)

# Draw knob design
inner_radius = radius * 0.8
draw.ellipse((center_x - inner_radius, center_y - inner_radius, 
              center_x + inner_radius, center_y + inner_radius),
             fill=(30, 30, 30, 255), outline=(60, 60, 60, 255), width=3)

# Draw indicator line
from math import sin, cos, pi
angle = pi * 0.7  # Indicator position
line_length = inner_radius * 0.7
x1 = center_x
y1 = center_y
x2 = center_x + line_length * cos(angle)
y2 = center_y + line_length * sin(angle)
draw.line([(x1, y1), (x2, y2)], fill=(255, 255, 255, 255), width=6)

# Add highlight
highlight_radius = inner_radius * 0.3
offset_x, offset_y = -inner_radius * 0.2, -inner_radius * 0.2
draw.ellipse((center_x - highlight_radius + offset_x, center_y - highlight_radius + offset_y,
              center_x + highlight_radius + offset_x, center_y + highlight_radius + offset_y),
             fill=(255, 255, 255, 100))

# Save the image
icons_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'Assets')
if not os.path.exists(icons_dir):
    os.makedirs(icons_dir)

image.save(os.path.join(icons_dir, 'icon.png'))
print(f"Icon saved to {os.path.join(icons_dir, 'icon.png')}")
