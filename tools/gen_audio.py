#!/usr/bin/env python3
"""Generates the game's music and sound effects into assets/sounds.

The sound mirrors the 1-bit art: only pulse (square) waves and LFSR noise,
like a PC speaker or an early handheld. Everything is synthesised here
from note data, so the audio is original and reproducible.

Output: 22050 Hz mono WAV; music 8-bit, effects 16-bit (the in-game mixer
converts both to its device format on load).
Pure stdlib:  python3 tools/gen_audio.py
"""
import math
import os
import struct
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "sounds")

# ---------------------------------------------------------------- synthesis

NOTE_INDEX = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6,
              "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}


def freq(note):
    """'A4' -> 440.0, 'C#5' -> ...; None/'.' -> None (rest)."""
    if note in (None, "."):
        return None
    name, octave = note[:-1], int(note[-1])
    semis = NOTE_INDEX[name] + 12 * (octave + 1) - 69
    return 440.0 * 2 ** (semis / 12.0)


def buffer(seconds):
    return [0.0] * int(seconds * RATE)


def add_pulse(buf, start, dur, f, vol=0.3, duty=0.5, attack=0.004,
              release=0.03, slide_to=None, vibrato=0.0):
    """Square wave with a tiny attack/release to avoid clicks."""
    if f is None:
        return
    s0, n = int(start * RATE), int(dur * RATE)
    phase = 0.0
    for i in range(n):
        t = i / RATE
        cur = f if slide_to is None else f + (slide_to - f) * (i / n)
        if vibrato:
            cur *= 1.0 + vibrato * math.sin(2 * math.pi * 6.0 * t)
        phase = (phase + cur / RATE) % 1.0
        env = min(1.0, t / attack) if attack else 1.0
        tail = dur - t
        if tail < release:
            env *= max(0.0, tail / release)
        idx = s0 + i
        if 0 <= idx < len(buf):
            buf[idx] += vol * env * (1.0 if phase < duty else -1.0)


def add_noise(buf, start, dur, vol=0.3, period=1, decay=None, short=False,
              period_to=None):
    """NES-style 15-bit LFSR noise. `period` = samples per LFSR step (pitch);
    `short` gives the metallic 93-step mode; `decay` is an exp time constant."""
    s0, n = int(start * RATE), int(dur * RATE)
    lfsr, out, counter = 1, 1.0, 0.0
    for i in range(n):
        p = period if period_to is None else period + (period_to - period) * i / n
        counter += 1.0
        while counter >= p:
            counter -= p
            bit = (lfsr ^ (lfsr >> (6 if short else 1))) & 1
            lfsr = (lfsr >> 1) | (bit << 14)
            out = 1.0 if lfsr & 1 else -1.0
        t = i / RATE
        env = math.exp(-t / decay) if decay else 1.0
        tail = dur - t
        if tail < 0.01:
            env *= tail / 0.01
        idx = s0 + i
        if 0 <= idx < len(buf):
            buf[idx] += vol * env * out


def write_wav(name, buf, peak=0.85, bits=16):
    """Normalises to `peak`. Music uses 8-bit (half the size; the extra
    grit suits the chiptune sound), effects use 16-bit."""
    top = max(1e-9, max(abs(x) for x in buf))
    scale = min(1.0, peak / top)
    clamp = [max(-1.0, min(1.0, x * scale)) for x in buf]
    if bits == 8:
        frames = bytes(int(round(x * 127)) + 128 for x in clamp)
    else:
        frames = b"".join(struct.pack("<h", int(x * 32767)) for x in clamp)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(bits // 8)
        w.setframerate(RATE)
        w.writeframes(frames)
    return len(buf) / RATE


# ---------------------------------------------------------------- sequencing

def play_line(buf, notes, step, start=0.0, **kw):
    """notes: list of (note, steps). Each step lasts `step` seconds."""
    t = start
    for note, steps in notes:
        add_pulse(buf, t, steps * step * 0.92, freq(note), **kw)
        t += steps * step
    return t


CHORDS = {  # root, chord tones (for bass and arpeggio)
    "C": ("C", ["C", "E", "G"]),
    "Am": ("A", ["A", "C", "E"]),
    "F": ("F", ["F", "A", "C"]),
    "G": ("G", ["G", "B", "D"]),
    "Em": ("E", ["E", "G", "B"]),
    "Dm": ("D", ["D", "F", "A"]),
}


def add_bass(buf, chords, bar, start=0.0, vol=0.22):
    eighth = bar / 8
    for b, name in enumerate(chords):
        root = CHORDS[name][0]
        for k in range(8):
            octave = 3 if k % 2 else 2
            add_pulse(buf, start + b * bar + k * eighth, eighth * 0.8,
                      freq(root + str(octave)), vol=vol, duty=0.5)


def add_arp(buf, chords, bar, start=0.0, vol=0.07, octave=4):
    sixteenth = bar / 16
    for b, name in enumerate(chords):
        tones = CHORDS[name][1]
        seq = [tones[0] + str(octave), tones[1] + str(octave),
               tones[2] + str(octave), tones[0] + str(octave + 1)]
        for k in range(16):
            add_pulse(buf, start + b * bar + k * sixteenth, sixteenth * 0.7,
                      freq(seq[k % 4]), vol=vol, duty=0.125, release=0.01)


def add_drums(buf, bars, bar, start=0.0, hats_only=False):
    sixteenth = bar / 16
    for b in range(bars):
        t0 = start + b * bar
        for k in range(16):
            t = t0 + k * sixteenth
            if not hats_only and k in (0, 8):          # kick: low pitched noise thump
                add_pulse(buf, t, 0.09, 110, vol=0.35, slide_to=40, release=0.05)
                add_noise(buf, t, 0.03, vol=0.10, period=24, decay=0.015)
            if not hats_only and k in (4, 12):         # snare
                add_noise(buf, t, 0.14, vol=0.22, period=3, decay=0.05)
            if k % 4 == 2:                             # closed hat
                add_noise(buf, t, 0.04, vol=0.08, period=1, decay=0.012, short=True)


# ---------------------------------------------------------------- music

def music_game():
    """'Crosswalk Rush', 150 BPM, 16 bars in C major, loops seamlessly."""
    bpm = 150
    bar = 4 * 60.0 / bpm
    e = bar / 8                        # one eighth note
    chords = ["C", "Am", "F", "G", "C", "Am", "F", "G",
              "F", "G", "Em", "Am", "F", "G", "C", "G"]
    melody = [
        # A: C Am F G
        ("E5", 1), ("G5", 1), ("C6", 2), ("B5", 1), ("C6", 1), ("G5", 2),
        ("A5", 2), ("E5", 1), ("A5", 1), ("C6", 2), ("B5", 1), ("A5", 1),
        ("F5", 1), ("A5", 1), ("C6", 2), ("A5", 1), ("F5", 1), ("A5", 2),
        ("G5", 2), ("B5", 1), ("D6", 1), ("B5", 2), ("G5", 2),
        # A': C Am F G
        ("E5", 1), ("G5", 1), ("C6", 2), ("D6", 1), ("E6", 1), ("D6", 2),
        ("C6", 2), ("A5", 1), ("C6", 1), ("E6", 2), ("D6", 1), ("C6", 1),
        ("A5", 1), ("C6", 1), ("A5", 1), ("F5", 1), ("G5", 2), ("A5", 2),
        ("B5", 2), ("D6", 2), ("G5", 4),
        # B: F G Em Am F G C G
        ("C6", 3), ("A5", 1), ("F5", 2), ("A5", 2),
        ("D6", 3), ("B5", 1), ("G5", 2), ("B5", 2),
        ("E6", 2), ("D6", 1), ("B5", 1), ("G5", 2), ("E5", 2),
        ("A5", 2), ("B5", 1), ("C6", 1), ("E6", 2), ("C6", 2),
        ("F5", 1), ("A5", 1), ("C6", 1), ("A5", 1), ("C6", 2), ("D6", 1), ("C6", 1),
        ("B5", 2), ("G5", 1), ("B5", 1), ("D6", 4),
        ("E6", 2), ("D6", 1), ("C6", 1), ("G5", 2), ("E5", 2),
        ("G5", 2), ("B5", 2), ("D6", 4),
    ]
    assert sum(s for _, s in melody) == 8 * len(chords)
    buf = buffer(bar * len(chords))
    play_line(buf, melody, e, vol=0.20, duty=0.25, vibrato=0.004)
    add_bass(buf, chords, bar)
    add_arp(buf, chords, bar)
    add_drums(buf, len(chords), bar)
    return buf


def music_title():
    """'Sidewalk Lights', 112 BPM, 8 bars in A minor, gentle loop."""
    bpm = 112
    bar = 4 * 60.0 / bpm
    q = bar / 4
    chords = ["Am", "F", "C", "G", "Am", "F", "Dm", "Em"]
    melody = [
        ("E5", 2), ("C5", 1), ("A4", 1),
        ("F5", 2), ("E5", 1), ("C5", 1),
        ("G5", 3), ("E5", 1),
        ("D5", 2), ("B4", 2),
        ("E5", 1), ("A5", 1), ("G5", 1), ("E5", 1),
        ("F5", 2), ("A5", 1), ("C6", 1),
        ("A5", 2), ("F5", 1), ("D5", 1),
        ("E5", 3), (".", 1),
    ]
    assert sum(s for _, s in melody) == 4 * len(chords)
    buf = buffer(bar * len(chords))
    play_line(buf, melody, q, vol=0.18, duty=0.5, vibrato=0.006, release=0.08)
    add_bass(buf, chords, bar, vol=0.16)
    add_arp(buf, chords, bar, vol=0.06)
    add_drums(buf, len(chords), bar, hats_only=True)
    return buf


# ---------------------------------------------------------------- jingles

def jingle_stage_clear():
    buf = buffer(1.6)
    s = 0.075
    t = play_line(buf, [("C5", 1), ("E5", 1), ("G5", 1), ("C6", 1),
                        ("E5", 1), ("G5", 1), ("C6", 1), ("E6", 1)], s,
                  vol=0.25, duty=0.25)
    add_pulse(buf, t, 0.12, freq("D6"), vol=0.25, duty=0.25)
    add_pulse(buf, t + 0.14, 0.7, freq("E6"), vol=0.25, duty=0.25,
              vibrato=0.01, release=0.3)
    for i, n in enumerate(["C3", "G3", "C4"]):
        add_pulse(buf, i * 0.2, 0.18, freq(n), vol=0.2)
    add_pulse(buf, t + 0.14, 0.7, freq("C4"), vol=0.18, release=0.3)
    return buf


def jingle_game_over():
    buf = buffer(2.2)
    notes = [("G4", 0.22), ("F#4", 0.22), ("F4", 0.22), ("E4", 0.5)]
    t = 0.0
    for n, d in notes:
        add_pulse(buf, t, d * 0.9, freq(n), vol=0.25, duty=0.5, vibrato=0.01)
        add_pulse(buf, t, d * 0.9, freq(n[:-1] + "2"), vol=0.18)
        t += d
    add_pulse(buf, t, 0.8, freq("C4"), vol=0.25, slide_to=freq("C3"), release=0.4)
    add_pulse(buf, t, 0.8, freq("C2"), vol=0.2, release=0.4)
    return buf


def jingle_victory():
    buf = buffer(3.2)
    s = 0.12
    lead = [("G4", 1), ("C5", 1), ("E5", 1), ("G5", 2), ("E5", 1), ("G5", 4),
            ("A4", 1), ("C5", 1), ("F5", 1), ("A5", 2), ("F5", 1), ("A5", 4),
            ("B5", 1), ("B5", 1), ("B5", 1), ("C6", 8)]
    play_line(buf, lead, s, vol=0.22, duty=0.25)
    bass = [("C3", 6), ("C3", 4), ("F3", 6), ("F3", 4), ("G3", 3), ("C3", 8)]
    play_line(buf, bass, s, vol=0.2, duty=0.5)
    add_noise(buf, 20 * s, 0.6, vol=0.12, period=2, decay=0.25)
    return buf


# ---------------------------------------------------------------- effects

def sfx_hop():
    buf = buffer(0.07)
    add_pulse(buf, 0, 0.06, 380, vol=0.3, duty=0.25, slide_to=760, release=0.02)
    return buf


def sfx_coin(level):
    """Two-tone chime; each combo level is a whole tone higher."""
    up = 2 ** ((level - 1) * 2 / 12.0)
    buf = buffer(0.28)
    add_pulse(buf, 0, 0.06, freq("B5") * up, vol=0.28, duty=0.5)
    add_pulse(buf, 0.06, 0.2, freq("E6") * up, vol=0.28, duty=0.5, release=0.15)
    return buf


def sfx_goal():
    buf = buffer(0.42)
    play_line(buf, [("C5", 1), ("E5", 1), ("G5", 1), ("C6", 3)], 0.055,
              vol=0.28, duty=0.25)
    return buf


def sfx_splat():
    buf = buffer(0.45)
    add_noise(buf, 0, 0.4, vol=0.45, period=6, period_to=30, decay=0.12)
    add_pulse(buf, 0, 0.25, 220, vol=0.3, slide_to=55, release=0.1)
    return buf


def sfx_splash():
    buf = buffer(0.6)
    add_noise(buf, 0, 0.55, vol=0.4, period=1, period_to=10, decay=0.2)
    add_pulse(buf, 0.02, 0.18, 900, vol=0.12, duty=0.125, slide_to=300)
    return buf


def sfx_caught():
    buf = buffer(0.5)
    add_pulse(buf, 0, 0.45, 660, vol=0.28, duty=0.5, slide_to=110,
              vibrato=0.05, release=0.1)
    return buf


def sfx_time_up():
    buf = buffer(0.75)
    for i in range(3):
        add_pulse(buf, i * 0.24, 0.1, 880, vol=0.25, duty=0.5)
        add_pulse(buf, i * 0.24 + 0.11, 0.1, 660, vol=0.25, duty=0.5)
    return buf


def sfx_start():
    buf = buffer(0.25)
    play_line(buf, [("G5", 1), ("C6", 3)], 0.05, vol=0.28, duty=0.25)
    return buf


def sfx_pause():
    buf = buffer(0.16)
    play_line(buf, [("C6", 1), ("G5", 1)], 0.06, vol=0.22, duty=0.5)
    return buf


def main():
    os.makedirs(OUT, exist_ok=True)
    sounds = {
        "music_game": music_game(),
        "music_title": music_title(),
        "jingle_stage_clear": jingle_stage_clear(),
        "jingle_game_over": jingle_game_over(),
        "jingle_victory": jingle_victory(),
        "sfx_hop": sfx_hop(),
        "sfx_goal": sfx_goal(),
        "sfx_splat": sfx_splat(),
        "sfx_splash": sfx_splash(),
        "sfx_caught": sfx_caught(),
        "sfx_time_up": sfx_time_up(),
        "sfx_start": sfx_start(),
        "sfx_pause": sfx_pause(),
    }
    for level in range(1, 6):
        sounds["sfx_coin_%d" % level] = sfx_coin(level)
    for name, buf in sorted(sounds.items()):
        secs = write_wav(name, buf, bits=8 if name.startswith("music") else 16)
        print("%-22s %6.2fs" % (name + ".wav", secs))


if __name__ == "__main__":
    main()
