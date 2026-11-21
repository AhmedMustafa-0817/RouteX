(() => {
  'use strict';

  const { api, escapeHTML, toast, setApiState } = window.RouteX;

  // Display-only copy of the graph from server.js (setupRoutes).
  // The route itself is always calculated by the C++ backend.
  const CITIES = {
    Karachi:   [70, 235],
    Hyderabad: [165, 268],
    Quetta:    [135, 85],
    Sukkur:    [255, 165],
    Multan:    [360, 205],
    Lahore:    [455, 150],
    Islamabad: [545, 90],
    Peshawar:  [625, 55]
  };
  const EDGES = [
    ['Karachi', 'Hyderabad', 2], ['Hyderabad', 'Sukkur', 5], ['Sukkur', 'Multan', 6],
    ['Multan', 'Lahore', 4], ['Lahore', 'Islamabad', 4], ['Islamabad', 'Peshawar', 3],
    ['Quetta', 'Sukkur', 7], ['Karachi', 'Quetta', 10]
  ];

  const form = document.getElementById('routeForm');
  const sourceSelect = document.getElementById('routeSource');
  const destinationSelect = document.getElementById('routeDestination');
  const resultBox = document.getElementById('routeResult');
  const subtitle = document.getElementById('routeSubtitle');
  const map = document.getElementById('routeMap');
  const findButton = document.getElementById('findRouteButton');

  const edgeKey = (a, b) => [a, b].sort().join('|');
  const edgeWeights = new Map(EDGES.map(([a, b, w]) => [edgeKey(a, b), w]));

  function fillSelects() {
    const names = Object.keys(CITIES);
    sourceSelect.innerHTML = '<option value="">Select origin</option>' + names.map((n) => `<option>${n}</option>`).join('');
    destinationSelect.innerHTML = '<option value="">Select destination</option>' + names.map((n) => `<option>${n}</option>`).join('');
  }

  function renderMap(path = []) {
    const onPath = new Set();
    for (let i = 0; i < path.length - 1; i += 1) onPath.add(edgeKey(path[i], path[i + 1]));
    const pathNodes = new Set(path);

    const lines = EDGES.map(([a, b, w]) => {
      const [x1, y1] = CITIES[a];
      const [x2, y2] = CITIES[b];
      const active = onPath.has(edgeKey(a, b));
      return `<line class="map-edge${active ? ' is-active' : ''}" x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}"/>` +
        `<g class="map-weight${active ? ' is-active' : ''}" transform="translate(${(x1 + x2) / 2} ${(y1 + y2) / 2})"><rect x="-13" y="-9" width="26" height="18" rx="5"/><text y="4" text-anchor="middle">${w}h</text></g>`;
    }).join('');

    const nodes = Object.entries(CITIES).map(([name, [x, y]]) => {
      const classes = ['map-node'];
      if (pathNodes.has(name)) classes.push('is-active');
      if (name === path[0]) classes.push('is-start');
      if (name === path[path.length - 1] && path.length > 1) classes.push('is-end');
      const labelY = y > 240 ? y + 24 : y - 15;
      return `<g class="${classes.join(' ')}"><circle cx="${x}" cy="${y}" r="7"/><text x="${x}" y="${labelY}" text-anchor="middle">${name}</text></g>`;
    }).join('');

    map.innerHTML = lines + nodes;
  }

  function hours(value) {
    const number = Number(value);
    return Number.isFinite(number) ? number.toLocaleString(undefined, { maximumFractionDigits: 2 }) : '—';
  }

  function renderResult(route, source, destination) {
    const path = Array.isArray(route.path) ? route.path : [];
    subtitle.textContent = `${source} to ${destination}`;

    const steps = path.map((city, index) => {
      const next = path[index + 1];
      const leg = next ? edgeWeights.get(edgeKey(city, next)) : null;
      const role = index === 0 ? 'Start' : index === path.length - 1 ? 'Arrive' : 'Stop';
      return `<li class="route-step"><span class="route-step-dot"></span><div><strong>${escapeHTML(city)}</strong><small>${role}</small></div>${leg != null ? `<span class="route-leg">+${leg}h to ${escapeHTML(next)}</span>` : ''}</li>`;
    }).join('');

    resultBox.innerHTML = `
      <div class="route-total"><strong>${escapeHTML(hours(route.totalHours))}</strong><span>hours total</span><em>${Math.max(path.length - 1, 0)} leg${path.length === 2 ? '' : 's'}</em></div>
      <ol class="route-steps">${steps}</ol>`;
    renderMap(path);
  }

  function renderMessage(text, type = '') {
    resultBox.innerHTML = `<p class="route-empty ${type}">${escapeHTML(text)}</p>`;
  }

  async function findRoute(event) {
    if (event) event.preventDefault();
    const source = sourceSelect.value;
    const destination = destinationSelect.value;
    if (!source || !destination) return;
    if (source === destination) {
      toast('Choose two different cities', 'Source and destination must be different.', 'error');
      return;
    }

    findButton.disabled = true;
    findButton.textContent = 'Calculating…';
    renderMessage('Running Dijkstra on the backend…');
    try {
      const route = await api('/api/routes/fastest', {
        method: 'POST',
        body: JSON.stringify({ source, destination })
      });
      setApiState(true);
      renderResult(route, source, destination);
    } catch (error) {
      renderMap([]);
      if (error.status === 404) {
        subtitle.textContent = `${source} to ${destination}`;
        renderMessage('No route connects these two cities.', 'is-error');
        toast('No route found', `${source} and ${destination} are not connected.`, 'error');
      } else {
        if (!error.status) setApiState(false, error.message);
        renderMessage('Could not reach the C++ API. Start routex_api and try again.', 'is-error');
        toast('Route calculation failed', error.message, 'error');
      }
    } finally {
      findButton.disabled = false;
      findButton.textContent = 'Find fastest route';
    }
  }

  async function checkApi() {
    try { await api('/'); setApiState(true); } catch (error) { setApiState(false, error.message); }
    try {
      const result = await api('/api/shipments');
      const count = document.getElementById('navShipmentCount');
      if (count) count.textContent = String(result.count ?? (result.shipments || []).length);
    } catch (_) { /* nav count is optional */ }
  }

  document.addEventListener('DOMContentLoaded', () => {
    fillSelects();
    renderMap([]);
    checkApi();
    form.addEventListener('submit', findRoute);
    document.getElementById('swapButton').addEventListener('click', () => {
      [sourceSelect.value, destinationSelect.value] = [destinationSelect.value, sourceSelect.value];
    });
    document.querySelectorAll('.route-chip').forEach((chip) => {
      chip.addEventListener('click', () => {
        sourceSelect.value = chip.dataset.from;
        destinationSelect.value = chip.dataset.to;
        findRoute();
      });
    });
  });
})();