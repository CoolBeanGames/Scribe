from PIL import Image, ImageDraw

def draw_scribe_icon(size):
    scale = 4
    dim = size * scale
    img = Image.new("RGBA", (dim, dim), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    pad = int(dim * 0.04)
    radius = int(dim * 0.22)
    bg_box = [pad, pad, dim - pad, dim - pad]
    
    draw.rounded_rectangle(bg_box, radius=radius, fill=(14, 17, 23, 255), outline=(45, 53, 72, 255), width=max(1, int(dim * 0.02)))

    accent_box = [pad + int(dim*0.015), pad + int(dim*0.015), dim - pad - int(dim*0.015), dim - pad - int(dim*0.015)]
    draw.rounded_rectangle(accent_box, radius=max(1, radius - int(dim*0.015)), fill=None, outline=(139, 124, 255, 60), width=max(1, int(dim * 0.015)))

    def pt(x, y):
        return (x * dim, y * dim)

    p_tip = pt(0.30, 0.72)
    p_s1 = pt(0.33, 0.55)
    p_w1 = pt(0.42, 0.42)
    p_t1 = pt(0.58, 0.24)
    p_t2 = pt(0.74, 0.40)
    p_w2 = pt(0.58, 0.58)
    p_s2 = pt(0.45, 0.67)

    draw.polygon([p_tip, p_s1, p_w1, p_t1, p_t2, p_w2, p_s2], fill=(139, 124, 255, 255))

    hole_center = pt(0.50, 0.48)
    hole_r = dim * 0.04
    draw.ellipse([hole_center[0] - hole_r, hole_center[1] - hole_r,
                  hole_center[0] + hole_r, hole_center[1] + hole_r],
                 fill=(14, 17, 23, 255))

    draw.line([p_tip, hole_center], fill=(14, 17, 23, 255), width=max(2, int(dim * 0.025)))
    draw.line([pt(0.56, 0.26), pt(0.72, 0.42)], fill=(244, 246, 250, 220), width=max(2, int(dim * 0.025)))

    draw.line([pt(0.24, 0.78), pt(0.38, 0.78)], fill=(97, 175, 239, 230), width=max(2, int(dim * 0.028)))
    draw.line([pt(0.28, 0.84), pt(0.46, 0.84)], fill=(139, 124, 255, 180), width=max(1, int(dim * 0.02)))

    return img.resize((size, size), Image.Resampling.LANCZOS)

sizes = [16, 24, 32, 48, 64, 128, 256]
images = [draw_scribe_icon(s) for s in sizes]

images[-1].save("scribe.png", format="PNG")
images[0].save("scribe.ico", format="ICO", sizes=[(s, s) for s in sizes], append_images=images[1:])
print("Icon created successfully: scribe.ico, scribe.png")
