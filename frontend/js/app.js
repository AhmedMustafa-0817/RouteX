(() => {
  'use strict';

  // Keep the browser layer thin: all shipment/DSA operations stay in the C++ API.
  const API_BASE = (window.ROUTEX_API_BASE || 'http://127.0.0.1:8080').replace(/\/$/, '');

  async function api(path, options = {}) {
    const response = await fetch(`${API_BASE}${path}`, {
      ...options,
      headers: {
        ...(options.body ? { 'Content-Type': 'application/json' } : {}),
        ...(options.headers || {})
      }
    });

    const contentType = response.headers.get('content-type') || '';
    const payload = contentType.includes('application/json')
      ? await response.json()
      : { message: await response.text() };

    if (!response.ok) {
      const error = new Error(payload.error || payload.message || `Request failed (${response.status})`);
      error.status = response.status;
      error.payload = payload;
      throw error;
    }
    return payload;
  }

  function escapeHTML(value) {
    return String(value ?? '').replace(/[&<>"']/g, (character) => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[character]));
  }

  function shipmentStatus(status) {
    const normalized = String(status || '').trim().toUpperCase();
    if (['CREATED', 'PENDING', 'QUEUED'].includes(normalized)) {
      return { label: 'Queued', className: 'status-queued', group: 'queued' };
    }
    if (['DISPATCHED', 'IN TRANSIT', 'IN_TRANSIT'].includes(normalized)) {
      return { label: 'In transit', className: 'status-transit', group: 'transit' };
    }
    if (['ARRIVED', 'DELIVERED', 'COMPLETED'].includes(normalized)) {
      return { label: 'Delivered', className: 'status-delivered', group: 'delivered' };
    }
    return { label: status || 'Unknown', className: 'status-unknown', group: 'other' };
  }

  function formatWeight(value) {
    const number = Number(value);
    return Number.isFinite(number) ? `${number.toLocaleString(undefined, { maximumFractionDigits: 2 })} kg` : '—';
  }

  function toast(title, message, type = 'success') {
    const region = document.getElementById('toastRegion');
    if (!region) return;
    const item = document.createElement('div');
    item.className = `toast${type === 'error' ? ' error' : ''}`;
    item.innerHTML = `<span class="toast-icon" aria-hidden="true">${type === 'error' ? '!' : '✓'}</span><div><strong>${escapeHTML(title)}</strong><p>${escapeHTML(message)}</p></div>`;
    region.appendChild(item);
    window.setTimeout(() => item.remove(), 4800);
  }

  function setApiState(online, message) {
    const pill = document.getElementById('apiPill');
    const pillText = document.getElementById('apiPillText');
    const sidebarText = document.getElementById('sidebarApiText');
    const footerText = document.getElementById('footerApiStatus');
    if (pill) {
      pill.classList.toggle('is-online', online);
      pill.classList.toggle('is-offline', !online);
    }
    if (pillText) pillText.textContent = online ? 'API connected' : 'API offline';
    if (sidebarText) sidebarText.textContent = online ? 'Connected · localhost:8080' : 'Start the C++ API server';
    if (footerText) footerText.textContent = online ? 'C++ API connected' : 'C++ API unavailable';
    const statusDot = document.querySelector('.sidebar-system-card .system-orb > span');
    if (statusDot) {
      statusDot.style.background = online ? '#45d2a1' : '#e77986';
      statusDot.style.boxShadow = online ? '0 0 0 4px rgba(69,210,161,.12)' : '0 0 0 4px rgba(231,121,134,.12)';
    }
    if (!online && message) {
      const footer = document.getElementById('registryFooter') || document.getElementById('ordersFooterStatus');
      if (footer) footer.textContent = 'Start routex_api to load live records';
    }
  }

  function renderDashboardShipments(shipments) {
    const body = document.getElementById('recentShipments');
    if (!body) return;
    const recent = shipments.slice().reverse().slice(0, 5);
    if (!recent.length) {
      body.innerHTML = '<tr><td colspan="4" class="table-message">No shipments yet. Create your first shipment to get started.</td></tr>';
      return;
    }
    body.innerHTML = recent.map((shipment) => {
      const state = shipmentStatus(shipment.status);
      const id = Number(shipment.shipmentID);
      const initials = Number.isFinite(id) ? String(id).slice(-2) : 'RX';
      return `<tr>
        <td><div class="shipment-main"><span class="shipment-avatar">${escapeHTML(initials)}</span><div class="shipment-cell"><strong>#${escapeHTML(shipment.shipmentID)}</strong><small>${escapeHTML(shipment.packageType)}</small></div></div></td>
        <td class="route-cell">${escapeHTML(shipment.sourceCity)}<span class="route-arrow">→</span>${escapeHTML(shipment.destinationCity)}</td>
        <td><span class="status-badge ${state.className}">${escapeHTML(state.label)}</span></td>
        <td class="weight-cell">${escapeHTML(formatWeight(shipment.weight))}</td>
      </tr>`;
    }).join('');
  }

  function updateDashboardStats(shipments) {
    const counts = { total: shipments.length, queued: 0, transit: 0, delivered: 0 };
    shipments.forEach((shipment) => {
      const group = shipmentStatus(shipment.status).group;
      if (group === 'queued') counts.queued += 1;
      else if (group === 'transit') counts.transit += 1;
      else if (group === 'delivered') counts.delivered += 1;
    });

    const assign = (id, value) => {
      const element = document.getElementById(id);
      if (element) element.textContent = String(value);
    };
    assign('statTotal', counts.total);
    assign('statQueued', counts.queued);
    assign('statTransit', counts.transit);
    assign('statDelivered', counts.delivered);
    assign('distributionTotal', counts.total);
    assign('legendQueued', counts.queued);
    assign('legendTransit', counts.transit);
    assign('legendDelivered', counts.delivered);
    assign('navShipmentCount', counts.total);
    assign('registryFooter', `${counts.total} shipment record${counts.total === 1 ? '' : 's'} in the live registry`);

    const bar = document.getElementById('distributionBar');
    if (bar) {
      const segments = [
        { selector: '.bar-queued', value: counts.queued },
        { selector: '.bar-transit', value: counts.transit },
        { selector: '.bar-delivered', value: counts.delivered }
      ];
      segments.forEach(({ selector, value }) => {
        const element = bar.querySelector(selector);
        if (element) {
          element.style.flex = String(Math.max(value, counts.total === 0 ? 1 : 0));
          element.style.opacity = value === 0 ? '.2' : '1';
        }
      });
    }
  }

  async function loadDashboard() {
    try {
      const result = await api('/api/shipments');
      const shipments = Array.isArray(result.shipments) ? result.shipments : [];
      setApiState(true);
      renderDashboardShipments(shipments);
      updateDashboardStats(shipments);

      try {
        const chain = await api('/api/blockchain');
        const blockCount = document.getElementById('blockCount');
        if (blockCount) blockCount.textContent = String(chain.blockCount ?? '—');
      } catch (_) {
        const blockCount = document.getElementById('blockCount');
        if (blockCount) blockCount.textContent = '—';
      }
      await verifyBlockchain(false);
    } catch (error) {
      setApiState(false, error.message);
      const recent = document.getElementById('recentShipments');
      if (recent) recent.innerHTML = '<tr><td colspan="4" class="table-message">Could not reach the C++ API. Start <strong>routex_api</strong>, then refresh this page.</td></tr>';
      updateDashboardStats([]);
      const chainStatus = document.getElementById('chainStatus');
      if (chainStatus) chainStatus.textContent = 'API unavailable';
    }
  }

  async function verifyBlockchain(showToast = true) {
    const button = document.getElementById('verifyChainButton');
    if (button) button.disabled = true;
    const status = document.getElementById('chainStatus');
    try {
      const result = await api('/api/blockchain/verify');
      if (status) {
        status.textContent = result.valid ? 'Integrity valid' : 'Integrity invalid';
        status.classList.toggle('valid', Boolean(result.valid));
        status.classList.toggle('invalid', !result.valid);
      }
      if (showToast) {
        toast(result.valid ? 'Blockchain verified' : 'Verification failed', result.valid ? 'The backend reports that the chain is valid.' : (result.error || 'The backend reports an invalid chain.'), result.valid ? 'success' : 'error');
      }
    } catch (error) {
      if (status) {
        status.textContent = 'Could not verify';
        status.classList.remove('valid');
        status.classList.add('invalid');
      }
      if (showToast) toast('Verification unavailable', error.message, 'error');
    } finally {
      if (button) button.disabled = false;
    }
  }

  function setUpNavigation() {
    const toggle = document.getElementById('menuToggle');
    const sidebar = document.getElementById('sidebar');
    if (!toggle || !sidebar) return;
    toggle.addEventListener('click', () => {
      const isOpen = sidebar.classList.toggle('is-open');
      toggle.setAttribute('aria-expanded', String(isOpen));
      toggle.setAttribute('aria-label', isOpen ? 'Close navigation' : 'Open navigation');
    });
    document.addEventListener('click', (event) => {
      if (window.innerWidth <= 760 && sidebar.classList.contains('is-open') && !sidebar.contains(event.target) && !toggle.contains(event.target)) {
        sidebar.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      }
    });
  }

  document.addEventListener('DOMContentLoaded', () => {
    setUpNavigation();
    const page = document.body.dataset.page;
    if (page === 'dashboard') {
      loadDashboard();
      const verifyButton = document.getElementById('verifyChainButton');
      if (verifyButton) verifyButton.addEventListener('click', () => verifyBlockchain(true));
    }
  });

  window.RouteX = { API_BASE, api, escapeHTML, shipmentStatus, formatWeight, toast, setApiState };
})();
