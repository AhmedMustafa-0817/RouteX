#!/usr/bin/env bash
# Build the static RouteX website: the C++ backend compiled to WebAssembly,
# plus the frontend pages. Output: web/dist/ (upload its contents to any
# static host). Requires the Emscripten SDK (em++); see web/README.md.
#
# The output is flat (every file at the top level, no css/ or js/ folders)
# because browser upload forms, such as the Hugging Face one, often drop
# folders and put every file at the root.
#
#   source ~/emsdk/emsdk_env.sh
#   bash web/build.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/web/dist"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

command -v em++ >/dev/null || { echo "em++ not found: run 'source ~/emsdk/emsdk_env.sh' first" >&2; exit 1; }

SOURCES=(
  backend/server/Server.cpp
  backend/src/Order.cpp
  backend/src/Shipment.cpp
  backend/src/Storage.cpp
  backend/src/ShipmentManager.cpp
  backend/src/Queue.cpp
  backend/src/HashTable.cpp
  backend/src/Graph.cpp
  backend/src/MinHeap.cpp
  backend/src/Dijkstra.cpp
  backend/src/Transaction.cpp
  backend/src/TransferEvent.cpp
  backend/src/Block.cpp
  backend/src/Blockchain.cpp
  backend/src/SHA256.cpp
)

cd "$ROOT"
rm -rf "$OUT"
mkdir -p "$OUT"

# Demo data goes into the WebAssembly virtual file system at /data.
mkdir -p "$WORK/data"
cp data/shipments.csv "$WORK/data/shipments.csv"

echo "Compiling C++ to WebAssembly..."
em++ -std=c++17 -O2 -Wall -Wextra \
  -Ibackend/include \
  "${SOURCES[@]}" \
  --embed-file "$WORK/data@/data" \
  -sMODULARIZE=1 \
  -sEXPORT_NAME=createRoutexModule \
  -sENVIRONMENT=web \
  -sALLOW_MEMORY_GROWTH=1 \
  -sSTACK_SIZE=1MB \
  -sEXPORTED_FUNCTIONS=_routex_init,_routex_request \
  -sEXPORTED_RUNTIME_METHODS=ccall,FS \
  -o "$OUT/routex.js"

echo "Copying frontend (flat layout)..."
cp frontend/*.html "$OUT/"
cp frontend/css/*.css "$OUT/"
cp frontend/js/*.js "$OUT/"
cp web/engine.js "$OUT/engine.js"

# No API server in the static build: the engine runs in the page.
cat > "$OUT/config.js" <<'EOF'
// Static WebAssembly build: the C++ backend runs in this page (engine.js).
window.ROUTEX_API_BASE = window.location.origin;
EOF

for page in "$OUT"/*.html; do
  # Flat layout: css/style.css -> style.css, js/app.js -> app.js.
  sed -i -e 's#href="css/#href="#g' -e 's#src="js/#src="#g' "$page"
  # Load the engine before app.js (deferred scripts run in order).
  sed -i 's#<script defer src="config.js"></script>#<script defer src="config.js"></script>\n  <script defer src="routex.js"></script>\n  <script defer src="engine.js"></script>#' "$page"
  grep -q 'src="engine.js"' "$page" || { echo "Could not add the engine to $page" >&2; exit 1; }
  if grep -qE '(href|src)="(css|js)/' "$page"; then echo "Folder path left in $page" >&2; exit 1; fi
done

# Hugging Face static Space settings (harmless on other hosts).
cp web/space-README.md "$OUT/README.md"

echo "Built $(du -sh "$OUT" | cut -f1) into web/dist:"
ls -1 "$OUT"
