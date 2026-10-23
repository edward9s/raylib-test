# Forest Ruins — raylib 6.2

Android arm64 C++ forest-ruins renderer ported from
[filament-test](https://github.com/edward9s/filament-test).

## Visual comparison

The forest generation algorithm, scene seed, material UVs, stone geometry, 1K
photographic textures, camera position/FOV, daylight direction, animation
frequencies, and scene scale are ported from Filament.

The rendering path uses **raylib 6.2**, with custom GLES 2.0 shaders for
photo-textured terrain / stones / bark, tangent-space normal mapping,
alpha-tested pine needles, soft (3×3 PCF) directional shadows, leaf sway,
a tonemapped HDR panorama, light haze and FPS text. Sunlight has a visible
0.53° solar disk and atmospheric halo; shadow-map ray marching produces warm
light shafts through sunlit gaps, with stronger highlights on ruins and ground.
These light shafts use six samples per shaded pixel and may lower FPS on older
phones.

**Pixel-for-pixel equivalence is not possible:** Filament used a much more
complex rendering pipeline (physically based BRDF and image-based lighting,
shadow cascades, SSAO, temporal AA and bloom), while raylib uses a custom
lightweight forward renderer. Compare the two on the same target device to
determine whether the visible fidelity is sufficient.

The original content consists of procedurally created scenery, not scanned
tree or ruin models. See `assets/CREDITS.md` for CC0 texture source credits.

## Android test APK

Use **Actions → Android test build → Run workflow**. Builds are manual; no
commit-triggered builds. A successful build replaces the same
`test-build` prerelease and `RaylibForest-test-arm64.apk` file. No Actions
Artifacts.

Configure four Secrets in **this** repository. They are not inherited from
`filament-test`:

- `KEYSTORE_BASE64`
- `STORE_PASSWORD`
- `KEY_ALIAS`
- `KEY_PASSWORD`

No signing keys are generated. Missing secrets fail the build early.

The runner clones the raylib `6.2` tag and compiles it from source, installs
NDK `27.2.12479018`, downloads SHA-256-verified Poly Haven textures,
converts HDR to PNG, then builds a signed Android APK with Gradle/CMake.
Generated sources are not committed.

Local build (requires JDK 21, SDK 35, NDK, CMake, Gradle, Python with Pillow):

```bash
bash scripts/download-raylib.sh
python3 -m pip install Pillow==11.3.0
python3 scripts/prepare-scene.py
export KEYSTORE_PATH=/path/to/your/existing.jks
export STORE_PASSWORD=... KEY_ALIAS=... KEY_PASSWORD=...
cd android && gradle :app:assembleRelease
```

One finger or left mouse button rotates the orbit camera; pinch or wheel zooms.
Two-finger vertical drag adjusts look height, so the sun above the tree canopy
can be brought into view without sending the camera underground.
The starting orbit camera values match Filament (yaw 0.24, pitch 0.12, radius 13).
FPS is shown in the top-left corner.

This repository deliberately shares only data and geometry with Filament. No
Filament SDK, rendering code, or runtime is used.
