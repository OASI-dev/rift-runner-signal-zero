#!/usr/bin/env python3
"""Check that the packaged audio assets are ordinary PCM RIFF/WAVE sound effects and ambience files."""
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "assets"
EXPECTED = (
    "space_ambience.wav",
    "rift_shift.wav",
    "void_whisper.wav",
    "deep_rumble.wav",
    "enemy_down.wav",
    "impact.wav",
    "pickup.wav",
    "surge.wav",
    "shot.wav",
)

def main() -> int:
    errors = []
    for name in EXPECTED:
        path = ASSET_DIR / name
        if not path.is_file():
            errors.append(f"missing: {path.relative_to(ROOT)}")
            continue
        blob = path.read_bytes()
        if len(blob) < 44 or blob[:4] != b"RIFF" or blob[8:12] != b"WAVE":
            errors.append(f"not a complete RIFF/WAVE file: {name}")
            continue
        # Scan chunks rather than assuming fmt/data occupy fixed offsets.
        offset = 12
        fmt = None
        data_size = None
        while offset + 8 <= len(blob):
            chunk_id = blob[offset:offset + 4]
            chunk_size = struct.unpack_from("<I", blob, offset + 4)[0]
            start = offset + 8
            end = start + chunk_size
            if end > len(blob):
                errors.append(f"truncated chunk in {name}")
                break
            if chunk_id == b"fmt " and chunk_size >= 16:
                fmt = struct.unpack_from("<HHIIHH", blob, start)
            elif chunk_id == b"data":
                data_size = chunk_size
            offset = end + (chunk_size & 1)
        if fmt is None or data_size is None:
            errors.append(f"missing fmt/data chunk in {name}")
            continue
        encoding, channels, rate, _byte_rate, _align, bits = fmt
        if encoding != 1:
            errors.append(f"expected PCM encoding 1 in {name}, got {encoding}")
        if channels not in (1, 2) or rate <= 0 or bits not in (8, 16, 24, 32):
            errors.append(f"unexpected audio format in {name}: channels={channels}, rate={rate}, bits={bits}")
        if data_size == 0:
            errors.append(f"empty audio data in {name}")
    if errors:
        print("Asset validation FAILED:", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        return 1
    print(f"Asset validation passed: {len(EXPECTED)} PCM WAV files found.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
