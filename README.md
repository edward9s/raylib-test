# Forest Ruins — raylib 6.2

Android arm64 C++ forest-ruins renderer ported from
[filament-test](https://github.com/edward9s/filament-test).

## Visual comparison

The forest generation algorithm, scene seed, material UVs, stone geometry, 1K
photographic textures, camera position/FOV, daylight direction, animation
frequencies, and scene scale are ported from Filament.

The rendering path uses **raylib 6.2**, with custom GLES 2.0 shaders for
photo-textured terrain / stones / bark, tangent-space normal mapping,
alpha-tested pine needles and a dynamic 2048² directional shadow map.
The original working single-map pipeline was restored after the experimental
two-cascade renderer caused the tree-leaf shadows to disappear. The light
frustum now covers 52 m instead of 76 m, shadows use sharper 2×2 PCF
and a smaller depth bias to preserve near-ground grass details. Tree branches,
pine needles, ferns, ivy and small grass blades keep their original animated
vertex displacement **in both the color and shadow passes**. This produces
animated, physically consistent cast shadows rather than frozen outlines.
The scene also retains the panorama sky, haze and FPS text.

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
Each Actions run increases Android `versionCode`, while keeping the package ID
`com.edward9s.raylibforesttest` unchanged. NDK r27 native libraries
are linked for 16 KiB Android pages; the workflow verifies APK ZIP alignment,
ELF LOAD alignment, signing scheme and package metadata.

If Android reports `INSTALL_FAILED_UPDATE_INCOMPATIBLE`, the installed
package has a different signing certificate: configure the original keystore,
or uninstall the old test app **after backing up its data**. If it reports
`INSTALL_FAILED_VERSION_DOWNGRADE`, install a newer build. If it reports
`INSTALL_FAILED_NO_MATCHING_ABIS`, this APK only supports arm64-v8a.

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
The starting orbit camera values match Filament (yaw 0.24, pitch 0.12, radius 13).
FPS is shown in the top-left corner.

This repository deliberately shares only data and geometry with Filament. No
Filament SDK, rendering code, or runtime is used.
