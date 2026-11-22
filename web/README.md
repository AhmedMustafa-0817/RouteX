# RouteX static build (C++ → WebAssembly)

Free server hosting now requires a credit card almost everywhere, so RouteX
can also run as a **static website**: the same C++ backend
(`backend/server/Server.cpp` → `handleRequest()`, the queue, hash table,
graph + Dijkstra and blockchain) is compiled to WebAssembly with
Emscripten and runs inside the visitor's browser. No server is needed, so
any free static host works (Hugging Face static Space, Vercel, Netlify,
GitHub Pages).

How it fits together:

| File | Role |
| --- | --- |
| `backend/server/Server.cpp` | Under `__EMSCRIPTEN__` it exports `routex_init()` / `routex_request()` instead of starting a socket server |
| `web/engine.js` | Loads the WebAssembly module, exposes `window.RouteXEngine.request()` |
| `frontend/js/app.js` | `api()` uses `RouteXEngine` when present, otherwise HTTP |
| `web/build.sh` | Compiles everything into `web/dist/` |

Behaviour of the static build:

* Each visitor has their own copy of the data, starting from
  `data/shipments.csv`. Shipments they add are saved in their browser
  (`localStorage`) and survive a reload. To start over, run
  `RouteXEngine.resetDemoData()` in the browser console.
* The blockchain is rebuilt from the shipments on every page load, just
  as the server rebuilds it on restart.

## Build

Needs Linux, macOS or WSL with the Emscripten SDK:

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
~/emsdk/emsdk install 3.1.74 && ~/emsdk/emsdk activate 3.1.74
source ~/emsdk/emsdk_env.sh

bash web/build.sh          # output: web/dist/
```

Preview locally (WebAssembly does not load from `file://`):

```bash
python3 -m http.server 8000 --directory web/dist   # open http://127.0.0.1:8000
```

## Deploy to a Hugging Face static Space (free, no card)

1. Create a Space at https://huggingface.co/new-space with **SDK: Static**,
   **Template: Blank**, free hardware, **Public**.
2. **Files → + Contribute → Upload files**, drag in **everything inside**
   `web/dist/` (the `.html` files, `README.md`, and the `css/` and `js/`
   folders), then **Commit**. Uploading `README.md` replaces the default
   one with the Space settings (`sdk: static`).
3. The site is live at `https://<username>-<space>.static.hf.space`, and
   also on the Space page.

To update: run `web/build.sh` again and upload the changed files
(usually `js/routex.js`, `js/routex.wasm` and any edited pages).

`web/dist/` is a build output and is not committed to git.
