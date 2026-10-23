#!/usr/bin/env python3
"""Pin and prepare the exact CC0 image maps used in filament-test."""
import hashlib
import io
import json
from pathlib import Path
from urllib.request import urlopen, Request
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / '.generated/assets/forest'
CACHE = ROOT / '.deps/scene'

def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    CACHE.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((ROOT / 'assets/sources.json').read_text())
    for entry in manifest:
        path = CACHE / (entry['name'] + Path(entry['url']).suffix)
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
            with urlopen(Request(entry['url'], headers={'User-Agent': 'raylib-forest-test/1.0'}), timeout=90) as response:
                content = response.read()
            if hashlib.sha256(content).hexdigest() != entry['sha256']:
                raise RuntimeError('SHA-256 mismatch: ' + entry['name'])
            path.write_bytes(content)
        if entry['name'] == 'environment':
            # Filament's KTX cubemap cannot be used directly by raylib 5.5.
            # The environment is retained in the source manifest for provenance.
            continue
        with Image.open(path) as source:
            image = source.convert('RGBA')
            if image.size != (1024, 1024):
                raise RuntimeError('Unexpected resolution: ' + entry['name'])
            if 'crop' in entry:
                image = image.crop(tuple(entry['crop'])).resize((1024, 1024), Image.Resampling.LANCZOS)
            image.save(OUTPUT / (entry['name'] + '.png'), optimize=True)
        print('Prepared', entry['name'], flush=True)
    (OUTPUT / 'CREDITS.md').write_bytes((ROOT / 'assets/CREDITS.md').read_bytes())

if __name__ == '__main__':
    main()
