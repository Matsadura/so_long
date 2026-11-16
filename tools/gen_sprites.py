#!/usr/bin/env python3
"""Generates the lane/arcade sprites in assets/sprites from 16x16 pixel art.

Pure stdlib (zlib + struct) so it runs anywhere. Each sprite is scaled 4x
to 64x64 to match the original so_long art. White parts of the tintable
sprites (cars, splat, splash) are colored at draw time.

    python3 tools/gen_sprites.py
"""
import os
import random
import struct
import zlib

OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "sprites")
SCALE = 4


def png(path, rows):
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


def rgba(hexstr):
    v = int(hexstr.lstrip("#"), 16)
    return ((v >> 16) & 255, (v >> 8) & 255, v & 255, 255)


CLEAR = (0, 0, 0, 0)


def render(name, art, palette):
    assert len(art) == 16 and all(len(r) == 16 for r in art), name
    pal = {k: rgba(v) for k, v in palette.items()}
    pal["."] = CLEAR
    rows = []
    for line in art:
        row = [pal[c] for c in line for _ in range(SCALE)]
        rows.extend([row] * SCALE)
    png(os.path.join(OUT, name + ".png"), rows)


def blank():
    return ["." * 16 for _ in range(16)]


def speckled(base, dots, seed, density=0.12):
    rnd = random.Random(seed)
    return ["".join("s" if rnd.random() < density else base for _ in range(16))
            for _ in range(16)], dots


def overlay(art, marks):
    art = [list(r) for r in art]
    for y, x, c in marks:
        art[y][x] = c
    return ["".join(r) for r in art]


def main():
    os.makedirs(OUT, exist_ok=True)

    # Pavement / median (replaces the old plain-black floor)
    art, _ = speckled("b", None, 1)
    render("floor", art, {"b": "#1b2028", "s": "#252c37"})

    # Road with dashed lane marking along the top edge
    art, _ = speckled("b", None, 2, 0.08)
    art = overlay(art, [(0, x, "d") for x in list(range(1, 7)) + list(range(9, 15))])
    render("road", art, {"b": "#2a2e35", "s": "#32373f", "d": "#7d838e"})

    # Water, two animation frames
    for i, shift in enumerate((0, 5)):
        art = ["b" * 16 for _ in range(16)]
        waves = [(2, 2), (6, 10), (10, 4), (13, 11)]
        marks = [(y, (x + shift + k) % 16, "w") for y, x in waves for k in range(3)]
        render("water_%d" % (i + 1), overlay(art, marks),
               {"b": "#1b4a8a", "w": "#3f86e0"})

    car = [
        "................",
        "................",
        "................",
        "....BBBBBBB.....",
        "...BDDDBDDDB....",
        "..BDDDDBDDDDB...",
        ".BBBBBBBBBBBBBB.",
        ".BBBBBBBBBBBBBL.",
        ".BGGBBBBBBBBBBL.",
        ".BBBBBBBBBBBBBB.",
        "..KKK......KKK..",
        "..KKK......KKK..",
        "................",
        "................",
        "................",
        "................",
    ]
    vehicle = {"B": "#ffffff", "D": "#1a1f27", "G": "#b8bec7",
               "K": "#0b0d10", "L": "#fff4c2"}
    render("car", car, vehicle)

    trailer = [
        "................",
        "................",
        "BBBBBBBBBBBBBBBB",
        "BGGGGGGGGGGGGGGB",
        "BGBBBBBBBBBBBBGB",
        "BGBBBBBBBBBBBBGB",
        "BGBBBBBBBBBBBBGB",
        "BGBBBBBBBBBBBBGB",
        "BGGGGGGGGGGGGGGB",
        "BBBBBBBBBBBBBBBB",
        ".KKK.......KKK..",
        ".KKK.......KKK..",
        "................",
        "................",
        "................",
        "................",
    ]
    render("truck_trailer", trailer, vehicle)

    cab = [
        "................",
        "................",
        "BBBBBBB.........",
        "GGGGGGB.........",
        "BBBBBGBBBBBBB...",
        "BBBBBGBBDDDDBB..",
        "BBBBBGBBDDDDDBB.",
        "BBBBBGBBBBBBBBB.",
        "GGGGGGBBBBBBBBL.",
        "BBBBBBBBBBBBBBB.",
        ".KKK.......KKK..",
        ".KKK.......KKK..",
        "................",
        "................",
        "................",
        "................",
    ]
    render("truck_cab", cab, vehicle)

    wood = {"B": "#8b5a2b", "D": "#5a3818", "L": "#c99256"}
    mid = blank()
    mid[4:11] = [
        "DDDDDDDDDDDDDDDD",
        "BBBBBBBBBBBBBBBB",
        "BBDBBBBBBBBDDBBB",
        "BBBBBBBBBBBBBBBB",
        "BBBBBDDBBBBBBBDB",
        "BBBBBBBBBBBBBBBB",
        "DDDDDDDDDDDDDDDD",
    ]
    render("log_mid", mid, wood)
    left = blank()
    left[4:11] = [
        "...DDDDDDDDDDDDD",
        "..DLLLDBBBBBBBBB",
        ".DLDDLDBBBBDDBBB",
        ".DLDLLDBBBBBBBBB",
        ".DLDDLDBBBBBBBDB",
        "..DLLLDBBBBBBBBB",
        "...DDDDDDDDDDDDD",
    ]
    render("log_left", left, wood)
    render("log_right", [r[::-1] for r in left], wood)

    coin = [
        "................",
        "................",
        "................",
        "......EEEE......",
        "....EEYYYYEE....",
        "...EYYYYYYYYE...",
        "...EYSSYYEYYE...",
        "..EYYSYYYEYYYE..",
        "..EYYYYYYEYYYE..",
        "..EYYYYYYEYYYE..",
        "...EYYYYYEYYE...",
        "...EYYYYYYYYE...",
        "....EEYYYYEE....",
        "......EEEE......",
        "................",
        "................",
    ]
    render("coin", coin, {"E": "#c98a00", "Y": "#ffd23f", "S": "#fff6c8"})

    heart = [
        "................",
        "................",
        "................",
        "...RRR...RRR....",
        "..RRRRR.RRRRR...",
        "..RWRRRRRRRRR...",
        "..RRRRRRRRRRR...",
        "...RRRRRRRRR....",
        "....RRRRRRR.....",
        ".....RRRRR......",
        "......RRR.......",
        ".......R........",
        "................",
        "................",
        "................",
        "................",
    ]
    render("heart", heart, {"R": "#ff4d5e", "W": "#ffc4cb"})

    splat = [
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
    render("splat", splat, {"W": "#ffffff"})

    splash = [
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
    render("splash", splash, {"W": "#ffffff"})


if __name__ == "__main__":
    main()
