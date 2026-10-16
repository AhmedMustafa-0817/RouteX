(() => {
  'use strict';

  const { api, escapeHTML, shipmentStatus, formatWeight, toast, setApiState } = window.RouteX;
  const tableBody = document.getElementById('shipmentsTableBody');
  const shipmentForm = document.getElementById('shipmentForm');
  const searchForm = document.getElementById('shipmentSearchForm');
  const searchInput = document.getElementById('shipmentSearch');
  const arrivalDialog = document.getElementById('arrivalDialog');
  let allShipments = [];
  let isSearchResult = false;
  let arrivalShipmentID = null;

  function showTableMessage(message) {
    if (!tableBody) return;
    tableBody.innerHTML = `<tr><td colspan="6" class="table-message">${escapeHTML(message)}</td></tr>`;
    document.getElementById('visibleCount').textContent = '0';
  }

  function updateSummary(shipments) {
    const counts = { total: shipments.length, queued: 0, transit: 0, delivered: 0 };
    shipments.forEach((shipment) => {
      const group = shipmentStatus(shipment.status).group;
      if (group === 'queued') counts.queued += 1;
      if (group === 'transit') counts.transit += 1;
      if (group === 'delivered') counts.delivered += 1;
    });
    document.getElementById('ordersTotal').textContent = String(counts.total);
    document.getElementById('ordersQueued').textContent = String(counts.queued);
    document.getElementById('ordersTransit').textContent = String(counts.transit);
    document.getElementById('ordersDelivered').textContent = String(counts.delivered);
    document.getElementById('navShipmentCount').textContent = String(counts.total);
  }

  function nextAvailableID(shipments) {
    return shipments.reduce((maximum, shipment) => Math.max(maximum, Number(shipment.shipmentID) || 0), 100) + 1;
  }

  function updateSuggestedID() {
    const idInput = document.getElementById('shipmentID');
    if (idInput && !idInput.dataset.userEdited) idInput.value = String(nextAvailableID(allShipments));
  }

  function renderShipments(shipments) {
    updateSummary(allShipments);
    document.getElementById('visibleCount').textContent = String(shipments.length);
    document.getElementById('resultSummary').textContent = isSearchResult ? 'Hash Table search result' : `Showing ${shipments.length} record${shipments.length === 1 ? '' : 's'}`;
    document.getElementById('ordersFooterStatus').textContent = `${allShipments.length} shipment record${allShipments.length === 1 ? '' : 's'} loaded from the C++ registry`;

    if (!shipments.length) {
      showTableMessage(isSearchResult ? 'No shipment matched that ID. Try another ID or refresh the registry.' : 'No shipments yet. Use the form above to add your first shipment.');
      return;
    }

    tableBody.innerHTML = shipments.slice().sort((a, b) => Number(b.shipmentID) - Number(a.shipmentID)).map((shipment) => {
      const state = shipmentStatus(shipment.status);
      const id = Number(shipment.shipmentID);
      const avatar = Number.isFinite(id) ? String(id).slice(-2) : 'RX';
      let action = '<span class="result-summary">—</span>';
      if (state.group === 'queued') {
        action = '<span class="result-summary">In queue</span>';
      } else if (state.group === 'transit') {
        action = `<button type="button" class="row-action arrival-action" data-arrival-id="${escapeHTML(shipment.shipmentID)}">Record arrival</button>`;
      } else if (state.group === 'delivered') {
        action = '<span class="status-badge status-delivered">Complete</span>';
      }

      return `<tr>
        <td><div class="shipment-main"><span class="shipment-avatar">${escapeHTML(avatar)}</span><div class="shipment-cell"><strong>#${escapeHTML(shipment.shipmentID)}</strong><small>${escapeHTML(shipment.status || 'Pending')}</small></div></div></td>
        <td class="route-cell">${escapeHTML(shipment.sourceCity)}<span class="route-arrow">→</span>${escapeHTML(shipment.destinationCity)}</td>
        <td>${escapeHTML(shipment.packageType)}</td>
        <td class="weight-cell">${escapeHTML(formatWeight(shipment.weight))}</td>
        <td><span class="status-badge ${state.className}">${escapeHTML(state.label)}</span></td>
        <td class="action-cell">${action}</td>
      </tr>`;
    }).join('');
  }

  async function loadShipments({ keepMessage = false } = {}) {
    if (!keepMessage) showTableMessage('Loading shipment records…');
    try {
      const result = await api('/api/shipments');
      allShipments = Array.isArray(result.shipments) ? result.shipments : [];
      isSearchResult = false;
      setApiState(true);
      renderShipments(allShipments);
      updateSuggestedID();
      return true;
    } catch (error) {
      setApiState(false, error.message);
      showTableMessage('Could not reach the C++ API. Start routex_api and refresh this page.');
      document.getElementById('ordersFooterStatus').textContent = 'API unavailable';
      toast('Unable to load shipments', error.message, 'error');
      return false;
    }
  }

  async function searchShipment(event) {
    event.preventDefault();
    const rawID = searchInput.value.trim();
    if (!rawID) {
      isSearchResult = false;
      renderShipments(allShipments);
      return;
    }
    if (!/^\d+$/.test(rawID) || Number(rawID) <= 0) {
      toast('Enter a shipment ID', 'Search uses the shipment ID so the backend can query its custom Hash Table.', 'error');
      searchInput.focus();
      return;
    }

    const button = searchForm.querySelector('button[type="submit"]');
    button.disabled = true;
    button.textContent = 'Searching…';
    try {
      const shipment = await api('/api/shipments/search', {
        method: 'POST',
        body: JSON.stringify({ shipmentID: Number(rawID) })
      });
      isSearchResult = true;
      renderShipments([shipment]);
      toast('Shipment found', `Shipment #${shipment.shipmentID} was found through the backend registry.`);
    } catch (error) {
      if (error.status === 404) {
        isSearchResult = true;
        renderShipments([]);
        toast('Shipment not found', `No shipment with ID ${rawID} exists in the registry.`, 'error');
      } else {
        toast('Search failed', error.message, 'error');
      }
    } finally {
      button.disabled = false;
      button.textContent = 'Search';
    }
  }

  async function createShipment(event) {
    event.preventDefault();
    const createButton = document.getElementById('createShipmentButton');
    const id = Number(document.getElementById('shipmentID').value);
    const sourceCity = document.getElementById('sourceCity').value;
    const destinationCity = document.getElementById('destinationCity').value;
    const packageType = document.getElementById('packageType').value;
    const weight = Number(document.getElementById('weight').value);

    if (sourceCity === destinationCity) {
      toast('Choose two different cities', 'The source and destination should not be the same for a shipment.', 'error');
      return;
    }
    if (!Number.isInteger(id) || id <= 0 || !Number.isFinite(weight) || weight <= 0) {
      toast('Check the shipment details', 'Enter a positive whole-number ID and a positive package weight.', 'error');
      return;
    }

    createButton.disabled = true;
    createButton.textContent = 'Creating…';
    try {
      const result = await api('/api/shipments', {
        method: 'POST',
        body: JSON.stringify({ shipmentID: id, sourceCity, destinationCity, packageType, weight, status: 'Pending' })
      });
      toast('Shipment created', result.message || `Shipment #${id} was added to the queue.`);
      shipmentForm.reset();
      document.getElementById('shipmentID').dataset.userEdited = '';
      document.getElementById('shipmentID').value = '';
      searchInput.value = '';
      await loadShipments();
      document.getElementById('shipmentID').value = String(nextAvailableID(allShipments));
      delete document.getElementById('shipmentID').dataset.userEdited;
    } catch (error) {
      toast('Could not create shipment', error.message, 'error');
    } finally {
      createButton.disabled = false;
      createButton.innerHTML = '<span class="button-plus">+</span> Create shipment';
    }
  }

  async function processNextShipment() {
    const button = document.getElementById('processNextButton');
    button.disabled = true;
    button.textContent = 'Processing…';
    try {
      const result = await api('/api/shipments/process-next', { method: 'POST', body: JSON.stringify({}) });
      toast('Shipment dispatched', result.message || 'The next shipment was removed from the FIFO queue.');
      searchInput.value = '';
      await loadShipments();
    } catch (error) {
      toast('Could not process queue', error.message, 'error');
    } finally {
      button.disabled = false;
      button.innerHTML = '<span aria-hidden="true">▶</span> Process next';
    }
  }

  function openArrivalDialog(shipmentID) {
    arrivalShipmentID = Number(shipmentID);
    document.getElementById('arrivalShipmentID').value = String(shipmentID);
    document.getElementById('arrivalLocation').value = '';
    if (typeof arrivalDialog.showModal === 'function') arrivalDialog.showModal();
    else arrivalDialog.setAttribute('open', '');
  }

  function closeArrivalDialog() {
    if (typeof arrivalDialog.close === 'function') arrivalDialog.close();
    else arrivalDialog.removeAttribute('open');
    arrivalShipmentID = null;
  }

  async function recordArrival(event) {
    event.preventDefault();
    const location = document.getElementById('arrivalLocation').value;
    if (!arrivalShipmentID || !location) return;
    const button = document.getElementById('confirmArrivalButton');
    button.disabled = true;
    button.textContent = 'Saving…';
    try {
      const result = await api('/api/shipments/arrival', {
        method: 'POST',
        body: JSON.stringify({ shipmentID: arrivalShipmentID, location })
      });
      closeArrivalDialog();
      toast('Arrival recorded', result.message || `Shipment #${arrivalShipmentID} marked as delivered.`);
      searchInput.value = '';
      await loadShipments();
    } catch (error) {
      toast('Could not record arrival', error.message, 'error');
    } finally {
      button.disabled = false;
      button.textContent = 'Record arrival';
    }
  }

  document.addEventListener('DOMContentLoaded', () => {
    loadShipments();
    shipmentForm.addEventListener('submit', createShipment);
    searchForm.addEventListener('submit', searchShipment);
    document.getElementById('refreshShipmentsButton').addEventListener('click', async () => {
      searchInput.value = '';
      await loadShipments();
      toast('Registry refreshed', 'Shipment list reloaded from the backend.');
    });
    document.getElementById('processNextButton').addEventListener('click', processNextShipment);
    document.getElementById('focusCreateButton').addEventListener('click', () => {
      document.getElementById('new-shipment').scrollIntoView({ behavior: 'smooth', block: 'start' });
      window.setTimeout(() => document.getElementById('shipmentID').focus(), 250);
    });
    document.getElementById('shipmentID').addEventListener('input', (event) => {
      event.target.dataset.userEdited = 'true';
    });
    tableBody.addEventListener('click', (event) => {
      const button = event.target.closest('[data-arrival-id]');
      if (button) openArrivalDialog(button.dataset.arrivalId);
    });
    document.getElementById('arrivalForm').addEventListener('submit', recordArrival);
    document.getElementById('closeArrivalDialog').addEventListener('click', closeArrivalDialog);
    document.getElementById('cancelArrivalButton').addEventListener('click', closeArrivalDialog);
    arrivalDialog.addEventListener('click', (event) => {
      if (event.target === arrivalDialog) closeArrivalDialog();
    });
  });
})();
