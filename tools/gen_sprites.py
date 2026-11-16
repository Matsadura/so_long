#!/usr/bin/env python3
"""Generates the lane sprites in assets/sprites from 16x16 pixel art.

The art follows the original so_long sprites exactly: a 16x16 grid drawn
at 4px per pixel (64x64 PNG), flat 1-bit shapes with no shading, white
silhouettes and outlines on pure black, and only the two accent colors
already used by the original enemies (red #C50404, yellow #F2F000).
Vehicles and effects are white so the renderer can tint them with those
accents.

Pure stdlib (zlib + struct):  python3 tools/gen_sprites.py
          verify palette:     python3 tools/gen_sprites.py --check
"""
import os
import struct
import sys
import zlib

OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "sprites")
SCALE = 4

# The complete original palette (+ transparency).
PALETTE = {
    ".": (0, 0, 0, 0),
    "W": (255, 255, 255, 255),
    "K": (0, 0, 0, 255),
    "R": (0xC5, 0x04, 0x04, 255),
    "Y": (0xF2, 0xF0, 0x00, 255),
}


def png_write(path, rows):
    h, w = len(rows), len(rows[0])
    raw = b"".join(b"\x00" + b"".join(bytes(px) for px in row) for row in rows)

    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def render(name, art):
    assert len(art) == 16 and all(len(r) == 16 for r in art), name
    rows = []
    for line in art:
        row = [PALETTE[c] for c in line for _ in range(SCALE)]
        rows.extend([row] * SCALE)
    png_write(os.path.join(OUT, name + ".png"), rows)


def grid(fill="."):
    return [[fill] * 16 for _ in range(16)]


def put(g, y, x, c):
    if 0 <= y < 16:
        g[y][x % 16] = c


def text(g):
    return ["".join(r) for r in g]


SPRITES = {}


def sprite(name):
    def deco(fn):
        SPRITES[name] = fn
        return fn
    return deco


@sprite("floor")
def floor():
    # Identical to the original bg.xpm: solid black.
    return text(grid("K"))


@sprite("road")
def road():
    # Black asphalt with a dashed lane line and sparse grit, in the
    # spirit of the specks inside the original wall tile.
    g = grid("K")
    for x in list(range(1, 6)) + list(range(9, 14)):
        put(g, 0, x, "W")
    for y, x in ((5, 3), (9, 11), (12, 6)):
        put(g, y, x, "W")
    return text(g)


def water(shift):
    # Black water with small white wave crests (two-pixel humps).
    g = grid("K")
    for y, x in ((3, 1), (6, 9), (10, 3), (13, 11)):
        x += shift
        put(g, y, x, "W")
        put(g, y - 1, x + 1, "W")
        put(g, y - 1, x + 2, "W")
        put(g, y, x + 3, "W")
    return text(g)


@sprite("water_1")
def water_1():
    return water(0)


@sprite("water_2")
def water_2():
    return water(4)


@sprite("car")
def car():
    # White silhouette with window cut-outs, like the player sprite's gaps.
    return [
        "................",
        "................",
        "................",
        "....WWWWWWW.....",
        "...W...W...W....",
        "..W....W....W...",
        ".WWWWWWWWWWWWWW.",
        ".WWWWWWWWWWWWWW.",
        ".WWWWWWWWWWWWWW.",
        ".WWWWWWWWWWWWWW.",
        "...WW......WW...",
        "...WW......WW...",
        "................",
        "................",
        "................",
        "................",
    ]


@sprite("truck_trailer")
def truck_trailer():
    # Outlined box, the same construction as the original door / wall.
    return [
        "................",
        "................",
        "WWWWWWWWWWWWWWWW",
        "W..............W",
        "W..W........W..W",
        "W..............W",
        "W..............W",
        "W.....W..W.....W",
        "W..............W",
        "WWWWWWWWWWWWWWWW",
        ".WW.........WW..",
        ".WW.........WW..",
        "................",
        "................",
        "................",
        "................",
    ]


@sprite("truck_cab")
def truck_cab():
    return [
        "................",
        "................",
        "WWWWWW..........",
        "W....W..........",
        "W....W.WWWWW....",
        "W....W.W...WW...",
        "W....W.W....WW..",
        "W....W.WWWWWWWW.",
        "W....W.WWWWWWWW.",
        "WWWWWW.WWWWWWWW.",
        ".WW........WW...",
        ".WW........WW...",
        "................",
        "................",
        "................",
        "................",
    ]


LOG_LEFT = [
    "................",
    "................",
    "................",
    "................",
    "...WWWWWWWWWWWWW",
    "..WKKKWKKKKKKKKK",
    ".WKWWKWKKWWWKKKK",
    ".WKWKKWKKKKKKKKK",
    ".WKWWKWKKKKKKWWK",
    "..WKKKWKKKKKKKKK",
    "...WWWWWWWWWWWWW",
    "................",
    "................",
    "................",
    "................",
    "................",
]


@sprite("log_left")
def log_left():
    return LOG_LEFT


@sprite("log_right")
def log_right():
    return [r[::-1] for r in LOG_LEFT]


@sprite("log_mid")
def log_mid():
    g = [list(r) for r in ["." * 16] * 16]
    body = [
        "WWWWWWWWWWWWWWWW",
        "KKKKKKKKKKKKKKKK",
        "KKWWWKKKKKKWWKKK",
        "KKKKKKKKKKKKKKKK",
        "KKKKKKWWWKKKKKKK",
        "KKKKKKKKKKKKKKKK",
        "WWWWWWWWWWWWWWWW",
    ]
    for i, r in enumerate(body):
        g[4 + i] = list(r)
    return text(g)


@sprite("splat")
def splat():
    # Drawn tinted with the original enemy red.
    return [
        "................",
        "................",
        ".......W........",
        "..W....W....W...",
        "...W..WWW..W....",
        "....WWWWWWW.....",
        ".W.WWWWWWWWW.W..",
        "..WWWWWWWWWWW...",
        "WWWWWWWWWWWWWWW.",
        "..WWWWWWWWWWW...",
        ".W.WWWWWWWWW.W..",
        "....WWWWWWW.....",
        "...W..WWW..W....",
        "..W....W....W...",
        ".......W........",
        "................",
    ]


@sprite("splash")
def splash():
    return [
        "................",
        "................",
        "................",
        ".....WWWWW......",
        "...WW.....WW....",
        "..W..WWWWW..W...",
        ".W..W.....W..W..",
        ".W.W..WWW..W.W..",
        ".W.W..W.W..W.W..",
        ".W.W..WWW..W.W..",
        ".W..W.....W..W..",
        "..W..WWWWW..W...",
        "...WW.....WW....",
        ".....WWWWW......",
        "................",
        "................",
    ]


def png_pixels(path):
    """Minimal PNG reader for RGBA8 / RGB8 / palette-free files we ship."""
    with open(path, "rb") as f:
        data = f.read()
    pos, idat, w, h, ctype = 8, b"", 0, 0, 6
    while pos < len(data):
        n = struct.unpack(">I", data[pos:pos + 4])[0]
        tag, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + n]
        if tag == b"IHDR":
            w, h, depth, ctype = struct.unpack(">IIBB", body[:10])
            assert depth == 8 and ctype in (2, 6), (path, depth, ctype)
        elif tag == b"IDAT":
            idat += body
        pos += 12 + n
    bpp = 4 if ctype == 6 else 3
    raw, stride, prev, out = zlib.decompress(idat), w * bpp, None, set()
    for y in range(h):
        ft, line = raw[y * (stride + 1)], bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i] if prev else 0
            c = prev[i - bpp] if prev and i >= bpp else 0
            if ft == 1:
                line[i] = (line[i] + a) & 255
            elif ft == 2:
                line[i] = (line[i] + b) & 255
            elif ft == 3:
                line[i] = (line[i] + (a + b) // 2) & 255
            elif ft == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        prev = line
        for x in range(w):
            px = tuple(line[x * bpp:(x + 1) * bpp]) + ((255,) if bpp == 3 else ())
            out.add((0, 0, 0, 0) if px[3] == 0 else px)
    return out


def check():
    allowed = set(PALETTE.values())
    bad = 0
    for name in sorted(os.listdir(OUT)):
        if name.endswith(".png"):
            extra = png_pixels(os.path.join(OUT, name)) - allowed
            status = "ok" if not extra else "OFF-PALETTE %s" % sorted(extra)[:3]
            bad += bool(extra)
            print("%-20s %s" % (name, status))
    return bad == 0


def main():
    if "--check" in sys.argv:
        sys.exit(0 if check() else 1)
    os.makedirs(OUT, exist_ok=True)
    for name, fn in SPRITES.items():
        render(name, fn())


if __name__ == "__main__":
    main()
