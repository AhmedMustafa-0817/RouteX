// Where the pages find the RouteX API.
//
// - Opened through routex_api itself (http://127.0.0.1:8090/ locally, or the
//   deployed Render URL): the server replaces this file with one that uses the
//   page's own origin, so nothing needs to be configured.
// - Opened through VS Code Live Server or another static server: this default
//   points at the API running locally.
window.ROUTEX_API_BASE = window.ROUTEX_API_BASE || 'http://127.0.0.1:8090';
