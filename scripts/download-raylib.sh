#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/.deps"
if [[ ! -f "$root/.deps/raylib/CMakeLists.txt" ]]; then
  git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git "$root/.deps/raylib"
fi
test "$(git -C "$root/.deps/raylib" describe --tags --exact-match)" = "5.5"
echo 'raylib 5.5 ready'
