// RouteX static build: runs the C++ backend (compiled to WebAssembly by
// web/build.sh) inside the page. app.js sees window.RouteXEngine and sends
// every API call here instead of over HTTP; the C++ handleRequest() answers
// exactly as the routex_api server would.
(() => {
  'use strict';

  const DATA_FILE = '/data/shipments.csv';
  const STORAGE_KEY = 'routex.shipments.csv';

  function readSaved() {
    try { return window.localStorage.getItem(STORAGE_KEY); } catch (_) { return null; }
  }

  function save(text) {
    try { window.localStorage.setItem(STORAGE_KEY, text); } catch (_) { /* storage unavailable */ }
  }

  const ready = window.createRoutexModule({
    print: (text) => console.log('[RouteX C++]', text),
    printErr: (text) => console.warn('[RouteX C++]', text)
  }).then((engine) => {
    // Shipments added in this browser survive a page reload.
    const saved = readSaved();
    if (saved) {
      try { engine.FS.writeFile(DATA_FILE, saved); } catch (_) { /* keep demo data */ }
    }
    engine.ccall('routex_init', 'number', [], []);
    return engine;
  });

  function parseResponse(raw) {
    const split = raw.indexOf('\r\n\r\n');
    const head = split >= 0 ? raw.slice(0, split) : raw;
    const body = split >= 0 ? raw.slice(split + 4) : '';
    const lines = head.split('\r\n');
    const status = Number((lines[0] || '').split(' ')[1]) || 500;
    const typeLine = lines.find((line) => line.toLowerCase().startsWith('content-type:'));
    const contentType = typeLine ? typeLine.slice(13).trim() : 'application/json';
    return { status, contentType, body };
  }

  window.RouteXEngine = {
    ready,

    async request(method, path, body) {
      const engine = await ready;
      const raw = engine.ccall(
        'routex_request',
        'string',
        ['string', 'string', 'string'],
        [method, path, body || '']
      );

      if (method !== 'GET') {
        try { save(engine.FS.readFile(DATA_FILE, { encoding: 'utf8' })); } catch (_) { /* nothing saved yet */ }
      }

      return parseResponse(raw);
    },

    // Forget shipments saved in this browser and start again from the demo data.
    resetDemoData() {
      try { window.localStorage.removeItem(STORAGE_KEY); } catch (_) { /* ignore */ }
      window.location.reload();
    }
  };
})();
