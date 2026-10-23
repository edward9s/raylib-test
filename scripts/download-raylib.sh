#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
src="$root/.deps/raylib"
commit=02566453880e06bec97f5b8160fa6fe1e98397ca
mkdir -p "$root/.deps"
if [[ ! -d "$src/.git" ]]; then
    if [[ -e "$src" ]]; then echo "Incomplete raylib source directory" >&2; exit 1; fi
    git clone --depth 1 --branch 6.2 https://github.com/raysan5/raylib.git "$src"
fi
actual="$(git -C "$src" rev-parse HEAD)"
if [[ "$actual" != "$commit" ]]; then
    echo "Expected pinned raylib 6.2 commit $commit, got $actual" >&2
    exit 1
fi
echo "raylib 6.2 source ready: $actual"
