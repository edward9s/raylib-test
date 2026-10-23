#!/usr/bin/env python3
"""SHA-pinned forest textures and a lightweight raylib HDR panorama."""
import hashlib
import json
import math
from pathlib import Path
from urllib.request import urlopen, Request
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=ROOT/'.generated/assets/forest'
CACHE=ROOT/'.deps/scene'

def hdr_pixels(data):
    # Radiance RLE RGBE, e.g. -Y 512 +X 1024. Reject unexpected variants.
    marker=data.index(b'\n\n')+2
    resolution_end=data.index(b'\n',marker)
    header=data[marker:resolution_end].strip().decode('ascii').split()
    if len(header)!=4 or header[0]!='-Y' or header[2]!='+X':
        raise RuntimeError('Unsupported Radiance HDR resolution/orientation')
    height,width=int(header[1]),int(header[3])
    if width<8 or width>32767 or height<=0:raise RuntimeError('Invalid HDR dimensions')
    offset=resolution_end+1
    output=Image.new('RGB',(width,height))
    pixels=output.load()
    for y in range(height):
        if data[offset:offset+2]!=b'\x02\x02' or int.from_bytes(data[offset+2:offset+4],'big')!=width:
            raise RuntimeError('Invalid Radiance RLE scanline')
        offset+=4
        planes=[]
        for _ in range(4):
            plane=bytearray()
            while len(plane)<width:
                if offset>=len(data):raise RuntimeError('Truncated Radiance image')
                code=data[offset]
                offset+=1
                if code>128:
                    count=code-128
                    if count==0 or offset>=len(data):raise RuntimeError('Invalid HDR RLE run')
                    plane.extend(bytes([data[offset]])*count);offset+=1
                elif code:
                    if offset+code>len(data):raise RuntimeError('Truncated HDR RLE literal')
                    plane.extend(data[offset:offset+code]);offset+=code
                else:raise RuntimeError('Invalid HDR RLE length')
            if len(plane)!=width:raise RuntimeError('HDR scanline width mismatch')
            planes.append(plane)
        for x in range(width):
            e=planes[3][x]
            if not e:pixels[x,y]=(0,0,0);continue
            scale=math.ldexp(1.0,e-136)
            linear=[planes[k][x]*scale for k in range(3)]
            # Stable display-referred approximation. Preserve outdoor highlights.
            pixels[x,y]=tuple(round(255*pow(min(1.0,max(0.0,v*0.55)/(1+v*0.55)),1/2.2)) for v in linear)
    return output

def main():
    OUTPUT.mkdir(parents=True,exist_ok=True)
    CACHE.mkdir(parents=True,exist_ok=True)
    for entry in json.loads((ROOT/'assets/sources.json').read_text()):
        name=entry['name']
        source=CACHE/(name+Path(entry['url']).suffix)
        if not source.is_file() or hashlib.sha256(source.read_bytes()).hexdigest()!=entry['sha256']:
            with urlopen(Request(entry['url'],headers={'User-Agent':'raylib-forest/6.2'}),timeout=90) as stream:
                data=stream.read()
            if hashlib.sha256(data).hexdigest()!=entry['sha256']:
                raise RuntimeError('Texture SHA-256 mismatch: '+name)
            source.write_bytes(data)
        if name=='environment':
            image=hdr_pixels(source.read_bytes())
        else:
            with Image.open(source) as file:
                image=file.convert('RGBA')
                if image.size!=(1024,1024):raise RuntimeError('Unexpected source dimensions: '+name)
                if 'crop' in entry:
                    image=image.crop(tuple(entry['crop'])).resize((1024,1024),Image.Resampling.LANCZOS)
        image.save(OUTPUT/(name+'.png'),optimize=True)
        print('Prepared',name,flush=True)
    (OUTPUT/'CREDITS.md').write_bytes((ROOT/'assets/CREDITS.md').read_bytes())

if __name__=='__main__':
    main()
