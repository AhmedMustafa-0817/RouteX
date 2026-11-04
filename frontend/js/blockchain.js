(() => {
  'use strict';

  const RouteX = window.RouteX || {};
  const { api, escapeHTML, shipmentStatus, toast, setApiState } = RouteX;
  const onReconnect = RouteX.onReconnect;

  let chainBlocks = [];
  let isChainValid = true;
  let chainError = '';
  let isTamperedState = false;
  let activeShipmentFilter = null;
  const expandedBlockSet = new Set();

  const verifyBtn = document.getElementById('verifyBlockchainBtn');
  const tamperBtn = document.getElementById('tamperBtn');
  const restoreBtn = document.getElementById('restoreBtn');
  const refreshBtn = document.getElementById('refreshBlocksBtn');
  const toggleExpandAllBtn = document.getElementById('toggleExpandAllBtn');
  const filterInput = document.getElementById('shipmentFilterInput');
  const clearFilterBtn = document.getElementById('clearFilterBtn');
  const shipmentChipsContainer = document.getElementById('shipmentChips');
  const blockListElement = document.getElementById('blockList');
  const chainRibbon = document.getElementById('chainRibbon');
  const auditCard = document.getElementById('shipmentAuditCard');
  const auditTimeline = document.getElementById('auditTimeline');
  const auditTitle = document.getElementById('auditTitle');
  const auditSubtitle = document.getElementById('auditSubtitle');
  const closeAuditBtn = document.getElementById('closeAuditBtn');

  // Register Modal Elements
  const openRegisterModalBtn = document.getElementById('openRegisterModalBtn');
  const registerModal = document.getElementById('registerModal');
  const closeRegisterModalBtn = document.getElementById('closeRegisterModalBtn');
  const cancelRegisterModalBtn = document.getElementById('cancelRegisterModalBtn');
  const registerBlockForm = document.getElementById('registerBlockForm');
  const regShipmentID = document.getElementById('regShipmentID');
  const regEventType = document.getElementById('regEventType');
  const regLocation = document.getElementById('regLocation');
  const regStatus = document.getElementById('regStatus');
  const presetCreatedBtn = document.getElementById('presetCreatedBtn');
  const presetDispatchedBtn = document.getElementById('presetDispatchedBtn');
  const presetArrivedBtn = document.getElementById('presetArrivedBtn');

  function truncateHash(hash, chars = 10) {
    if (!hash || hash === '0') return hash || '0';
    if (hash.length <= chars * 2) return hash;
    return `${hash.slice(0, chars)}…${hash.slice(-chars)}`;
  }

  function formatTime(timestamp) {
    if (!timestamp || timestamp === 'GENESIS') return timestamp || '—';
    return timestamp;
  }

  function copyToClipboard(text, label = 'Hash') {
    if (!navigator.clipboard) {
      toast('Copied', text, 'success');
      return;
    }
    navigator.clipboard.writeText(text)
      .then(() => toast('Copied to clipboard', `${label} copied successfully.`, 'success'))
      .catch(() => toast('Copy failed', text, 'error'));
  }

  function updateIntegrityBanner(valid, error = '', isTampered = false) {
    const banner = document.getElementById('integrityBanner');
    const icon = document.getElementById('integrityIcon');
    const title = document.getElementById('integrityTitle');
    const badge = document.getElementById('integrityBadge');
    const desc = document.getElementById('integrityDesc');
    const lastChecked = document.getElementById('lastVerifiedText');

    isChainValid = valid;
    chainError = error;
    isTamperedState = isTampered;

    const now = new Date().toLocaleTimeString();
    if (lastChecked) lastChecked.textContent = `Last checked: ${now}`;

    if (valid) {
      if (banner) {
        banner.className = 'panel integrity-banner is-valid';
      }
      if (icon) icon.textContent = '✓';
      if (title) title.textContent = 'Blockchain Integrity Verified';
      if (badge) {
        badge.className = 'integrity-badge badge-valid';
        badge.textContent = 'VALID';
      }
      if (desc) {
        desc.textContent = 'All SHA-256 block hashes and sequential previousHash pointers match cryptographic digests across memory.';
      }
      if (restoreBtn) restoreBtn.classList.add('is-hidden');
    } else {
      if (banner) {
        banner.className = 'panel integrity-banner is-invalid';
      }
      if (icon) icon.textContent = '⚠';
      if (title) title.textContent = 'Tampering / Integrity Violation Detected';
      if (badge) {
        badge.className = 'integrity-badge badge-invalid';
        badge.textContent = 'INVALID';
      }
      if (desc) {
        desc.innerHTML = `<strong>Backend Verification Failure:</strong> ${escapeHTML(error || 'Hash mismatch detected in linked list.')}`;
      }
      if (restoreBtn) restoreBtn.classList.remove('is-hidden');
    }

    const stateValue = document.getElementById('statChainState');
    const stateIcon = document.getElementById('statStateIcon');
    const stateFoot = document.getElementById('statChainStateFoot');

    if (stateValue) stateValue.textContent = valid ? 'Valid' : 'Tampered';
    if (stateIcon) {
      stateIcon.className = `stat-icon ${valid ? 'stat-icon-green' : 'stat-icon-red'}`;
      stateIcon.textContent = valid ? '✓' : '⚠';
    }
    if (stateFoot) {
      stateFoot.innerHTML = valid
        ? '<span class="stat-foot-dot green-dot"></span> Cryptographically verified'
        : '<span class="stat-foot-dot red-dot"></span> Hash mismatch detected';
    }
  }

  function updateMetrics(blocks) {
    const blockCountEl = document.getElementById('statBlockCount');
    const txCountEl = document.getElementById('statTxCount');

    let totalEvents = 0;
    blocks.forEach((b) => {
      totalEvents += (b.eventCount ?? (b.transactions ? b.transactions.length : 0));
    });

    if (blockCountEl) blockCountEl.textContent = String(blocks.length);
    if (txCountEl) txCountEl.textContent = String(totalEvents);
  }

  function renderChainRibbon(blocks) {
    if (!chainRibbon) return;

    if (!blocks || !blocks.length) {
      chainRibbon.innerHTML = '<div class="ribbon-empty">No node blocks registered yet.</div>';
      return;
    }

    chainRibbon.innerHTML = blocks.map((block, i) => {
      const isGenesis = block.index === 0;
      const isLast = i === blocks.length - 1;
      const eventCount = block.eventCount ?? (block.transactions ? block.transactions.length : 0);
      const isCompromised = !isChainValid && chainError.includes(`Block ${block.index}:`);

      return `
        <div class="ribbon-node${isGenesis ? ' is-genesis' : ''}${isCompromised ? ' is-compromised' : ''}" data-block-id="${block.index}">
          <div class="ribbon-node-header">
            <span class="ribbon-node-title">${isGenesis ? 'Genesis Node (#0)' : `Block Node #${block.index}`}</span>
            <span class="ribbon-node-count">${eventCount} tx</span>
          </div>
          <div class="ribbon-node-hash" title="${escapeHTML(block.currentHash)}">
            <code>${truncateHash(block.currentHash, 6)}</code>
          </div>
          <div class="ribbon-node-status">
            ${isCompromised ? '<span class="is-error">⚠ Mismatch</span>' : '<span class="is-valid">✓ Linked</span>'}
          </div>
        </div>
        ${!isLast ? '<div class="ribbon-connector"><span class="ribbon-arrow">→</span><small>SHA-256</small></div>' : ''}
      `;
    }).join('');

    chainRibbon.querySelectorAll('.ribbon-node').forEach((node) => {
      node.addEventListener('click', () => {
        const id = Number(node.dataset.blockId);
        const target = document.getElementById(`block-${id}`);
        if (target) {
          expandedBlockSet.add(id);
          renderBlocks();
          target.scrollIntoView({ behavior: 'smooth', block: 'center' });
        }
      });
    });
  }

  function renderFilterChips(blocks) {
    if (!shipmentChipsContainer) return;
    const shipmentIds = new Set();

    blocks.forEach((b) => {
      if (Array.isArray(b.transactions)) {
        b.transactions.forEach((tx) => {
          if (tx.shipmentID != null) shipmentIds.add(Number(tx.shipmentID));
        });
      }
    });

    const sortedIds = Array.from(shipmentIds).sort((a, b) => a - b);
    if (!sortedIds.length) {
      shipmentChipsContainer.innerHTML = '<span class="chips-empty">No shipment IDs recorded yet</span>';
      return;
    }

    shipmentChipsContainer.innerHTML = sortedIds.map((id) => {
      const active = activeShipmentFilter === id;
      return `<button type="button" class="route-chip${active ? ' is-active' : ''}" data-shipment="${id}">#${id}</button>`;
    }).join('');

    shipmentChipsContainer.querySelectorAll('.route-chip').forEach((chip) => {
      chip.addEventListener('click', () => {
        const id = Number(chip.dataset.shipment);
        if (activeShipmentFilter === id) {
          clearFilter();
        } else {
          applyFilter(id);
        }
      });
    });
  }

  function renderShipmentAudit(shipmentId) {
    if (!auditCard || !auditTimeline) return;
    if (shipmentId == null) {
      auditCard.classList.add('is-hidden');
      return;
    }

    const events = [];
    chainBlocks.forEach((block) => {
      if (Array.isArray(block.transactions)) {
        block.transactions.forEach((tx) => {
          if (Number(tx.shipmentID) === Number(shipmentId)) {
            events.push({ ...tx, blockIndex: block.index, blockHash: block.currentHash });
          }
        });
      }
    });

    if (!events.length) {
      auditCard.classList.remove('is-hidden');
      if (auditTitle) auditTitle.textContent = `Shipment #${shipmentId}`;
      if (auditSubtitle) auditSubtitle.textContent = `No recorded transfer events found for #${shipmentId}`;
      auditTimeline.innerHTML = '<p class="route-empty">No transactions found for this shipment ID.</p>';
      return;
    }

    auditCard.classList.remove('is-hidden');
    if (auditTitle) auditTitle.textContent = `Lifecycle Audit Trail · Shipment #${shipmentId}`;
    if (auditSubtitle) auditSubtitle.textContent = `${events.length} chronological event${events.length === 1 ? '' : 's'} recorded across blocks in the blockchain.`;

    auditTimeline.innerHTML = events.map((ev, index) => {
      const isLast = index === events.length - 1;
      const typeLower = (ev.eventType || '').toLowerCase();
      let dotColor = '#7163ee';
      if (typeLower.includes('create')) dotColor = '#4387d8';
      else if (typeLower.includes('dispatch') || typeLower.includes('transit')) dotColor = '#bf8121';
      else if (typeLower.includes('arrive') || typeLower.includes('deliver')) dotColor = '#189b72';

      return `
        <div class="audit-step${isLast ? ' is-last' : ''}">
          <div class="audit-step-dot" style="background:${dotColor};border-color:${dotColor}"></div>
          <div class="audit-step-card">
            <div class="audit-step-header">
              <div class="audit-step-type">
                <span class="status-badge ${typeLower.includes('create') ? 'status-queued' : typeLower.includes('dispatch') ? 'status-transit' : 'status-delivered'}">${escapeHTML(ev.eventType)}</span>
                <strong>Transaction #${escapeHTML(ev.transactionID)}</strong>
              </div>
              <span class="audit-step-time">${escapeHTML(formatTime(ev.timestamp))}</span>
            </div>
            <div class="audit-step-details">
              <div><span>Location:</span> <strong>${escapeHTML(ev.location)}</strong></div>
              <div><span>Status:</span> <em>${escapeHTML(ev.status)}</em></div>
              <div><span>Recorded in:</span> <a href="#block-${ev.blockIndex}" class="audit-block-link">Block #${ev.blockIndex}</a></div>
            </div>
          </div>
        </div>
      `;
    }).join('');
  }

  function renderBlocks() {
    if (!blockListElement) return;

    if (!chainBlocks.length) {
      blockListElement.innerHTML = `
        <div class="block-empty-state">
          <p>No blocks found. Start <strong>routex_api</strong> and create shipments to build the blockchain.</p>
        </div>`;
      return;
    }

    const filterId = activeShipmentFilter;
    const isFiltered = filterId != null;

    blockListElement.innerHTML = chainBlocks.map((block, i) => {
      const isGenesis = block.index === 0;
      const eventCount = block.eventCount ?? (block.transactions ? block.transactions.length : 0);
      const isExpanded = expandedBlockSet.has(block.index);

      // Check if block has transactions matching filter
      const matchingTxs = isFiltered
        ? (block.transactions || []).filter((tx) => Number(tx.shipmentID) === filterId)
        : (block.transactions || []);

      const blockHasMatch = !isFiltered || matchingTxs.length > 0;
      const dimClass = isFiltered && !blockHasMatch ? ' is-dimmed' : '';

      // Check if this block failed verification
      const isFailedBlock = !isChainValid && chainError.includes(`Block ${block.index}:`);

      // Link indicator with previous block
      const prevBlock = i > 0 ? chainBlocks[i - 1] : null;
      const isHashLinked = prevBlock ? block.previousHash === prevBlock.currentHash : true;

      const txListHtml = (block.transactions && block.transactions.length > 0)
        ? `
          <div class="block-tx-section">
            <div class="block-tx-header">
              <span>TRANSACTIONS (${block.transactions.length})</span>
              <small>Preserves FIFO order</small>
            </div>
            <div class="table-wrap">
              <table class="data-table block-tx-table">
                <thead>
                  <tr>
                    <th>TX ID</th>
                    <th>SHIPMENT</th>
                    <th>EVENT</th>
                    <th>LOCATION</th>
                    <th>TIMESTAMP</th>
                    <th>STATUS</th>
                  </tr>
                </thead>
                <tbody>
                  ${block.transactions.map((tx) => {
                    const isTxMatch = isFiltered && Number(tx.shipmentID) === filterId;
                    const type = (tx.eventType || '').toUpperCase();
                    let badgeClass = 'status-queued';
                    if (type === 'DISPATCHED') badgeClass = 'status-transit';
                    if (type === 'ARRIVED') badgeClass = 'status-delivered';

                    return `
                      <tr class="${isTxMatch ? 'is-highlighted-tx' : ''}">
                        <td><span class="tx-badge">#TX-${escapeHTML(tx.transactionID)}</span></td>
                        <td>
                          <button type="button" class="shipment-id-btn" data-shipment="${escapeHTML(tx.shipmentID)}" title="Filter audit trail for #${escapeHTML(tx.shipmentID)}">
                            #${escapeHTML(tx.shipmentID)}
                          </button>
                        </td>
                        <td><span class="status-badge ${badgeClass}">${escapeHTML(tx.eventType)}</span></td>
                        <td><strong>${escapeHTML(tx.location)}</strong></td>
                        <td class="time-cell">${escapeHTML(formatTime(tx.timestamp))}</td>
                        <td><span class="tx-status-pill">${escapeHTML(tx.status)}</span></td>
                      </tr>`;
                  }).join('')}
                </tbody>
              </table>
            </div>
          </div>`
        : `
          <div class="block-tx-empty">
            ${isGenesis ? 'Genesis root block — initializes the cryptographic chain.' : 'No transactions recorded in this block.'}
          </div>`;

      return `
        <article class="block-card panel${isGenesis ? ' is-genesis' : ''}${isFailedBlock ? ' is-compromised' : ''}${dimClass}" id="block-${block.index}">
          <div class="block-card-header" data-block-index="${block.index}">
            <div class="block-title-group">
              <span class="block-index-badge">${isGenesis ? 'Genesis Block' : `Block #${block.index}`}</span>
              <span class="block-event-pill">${eventCount} event${eventCount === 1 ? '' : 's'}</span>
              ${isFailedBlock ? '<span class="block-flag-compromised">⚠ Current hash mismatch</span>' : ''}
              ${!isHashLinked ? '<span class="block-flag-broken">⛓ Broken chain link</span>' : ''}
            </div>
            <div class="block-header-right">
              <span class="block-timestamp">${escapeHTML(formatTime(block.timestamp))}</span>
              <button type="button" class="block-expand-btn" aria-label="${isExpanded ? 'Collapse block' : 'Expand block'}" aria-expanded="${isExpanded}">
                <span class="chevron">${isExpanded ? '▲' : '▼'}</span>
              </button>
            </div>
          </div>

          <div class="block-card-body">
            <div class="block-hashes-grid">
              <div class="hash-field">
                <div class="hash-label">
                  <span>PREVIOUS HASH</span>
                  <button type="button" class="hash-copy-btn" data-copy="${escapeHTML(block.previousHash)}" title="Copy previous hash">📋 Copy</button>
                </div>
                <div class="hash-box" title="${escapeHTML(block.previousHash)}">
                  <code>${escapeHTML(block.previousHash)}</code>
                </div>
                ${!isGenesis ? `<div class="hash-link-note ${isHashLinked ? 'is-valid' : 'is-broken'}">
                  ${isHashLinked ? '✓ Matches Block #' + (block.index - 1) + ' currentHash' : '⚠ Link mismatch with Block #' + (block.index - 1)}
                </div>` : '<div class="hash-link-note is-genesis">Genesis root identifier (0)</div>'}
              </div>

              <div class="hash-field">
                <div class="hash-label">
                  <span>CURRENT HASH (SHA-256)</span>
                  <button type="button" class="hash-copy-btn" data-copy="${escapeHTML(block.currentHash)}" title="Copy current hash">📋 Copy</button>
                </div>
                <div class="hash-box hash-box-current" title="${escapeHTML(block.currentHash)}">
                  <code>${escapeHTML(block.currentHash)}</code>
                </div>
                <div class="hash-link-note">
                  ${isFailedBlock ? '<span class="is-error">⚠ Recalculated hash differs from stored hash</span>' : '<span class="is-valid">✓ SHA-256 digest verified from block contents</span>'}
                </div>
              </div>
            </div>

            <div class="block-collapsible${isExpanded ? ' is-open' : ''}">
              ${txListHtml}
            </div>
          </div>
        </article>
      `;
    }).join('');

    // Attach listeners for expand/collapse
    blockListElement.querySelectorAll('.block-card-header').forEach((hdr) => {
      hdr.addEventListener('click', () => {
        const idx = Number(hdr.dataset.blockIndex);
        if (expandedBlockSet.has(idx)) {
          expandedBlockSet.delete(idx);
        } else {
          expandedBlockSet.add(idx);
        }
        renderBlocks();
      });
    });

    // Copy buttons
    blockListElement.querySelectorAll('.hash-copy-btn').forEach((btn) => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        copyToClipboard(btn.dataset.copy, 'Block Hash');
      });
    });

    // Shipment click buttons inside table
    blockListElement.querySelectorAll('.shipment-id-btn').forEach((btn) => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        applyFilter(Number(btn.dataset.shipment));
      });
    });
  }

  function applyFilter(shipmentId) {
    if (!shipmentId || isNaN(shipmentId)) {
      clearFilter();
      return;
    }
    activeShipmentFilter = shipmentId;
    if (filterInput) filterInput.value = String(shipmentId);
    if (clearFilterBtn) clearFilterBtn.classList.remove('is-hidden');

    renderFilterChips(chainBlocks);
    renderShipmentAudit(shipmentId);
    renderBlocks();

    // Auto-expand blocks that contain this shipment
    chainBlocks.forEach((b) => {
      const hasMatch = (b.transactions || []).some((tx) => Number(tx.shipmentID) === shipmentId);
      if (hasMatch) expandedBlockSet.add(b.index);
    });
    renderBlocks();

    toast('Filter applied', `Showing audit records for Shipment #${shipmentId}`, 'success');
  }

  function clearFilter() {
    activeShipmentFilter = null;
    if (filterInput) filterInput.value = '';
    if (clearFilterBtn) clearFilterBtn.classList.add('is-hidden');
    renderFilterChips(chainBlocks);
    renderShipmentAudit(null);
    renderBlocks();
  }

  async function loadBlockchainData(silent = false) {
    try {
      const result = await api('/api/blockchain');
      chainBlocks = Array.isArray(result.blocks) ? result.blocks : [];
      setApiState(true);

      // Default: Expand non-genesis blocks if small chain, or expand first 2
      if (expandedBlockSet.size === 0) {
        chainBlocks.forEach((b) => {
          if (b.index <= 2) expandedBlockSet.add(b.index);
        });
      }

      updateMetrics(chainBlocks);
      renderChainRibbon(chainBlocks);
      renderFilterChips(chainBlocks);
      renderBlocks();
      if (activeShipmentFilter != null) {
        renderShipmentAudit(activeShipmentFilter);
      }

      await checkVerification(silent);
    } catch (error) {
      if (typeof setApiState === 'function') setApiState(false, error.message);
      if (!silent) {
        updateIntegrityBanner(false, `API Error: ${error.message}`);
      }
      if (blockListElement && (!chainBlocks || chainBlocks.length === 0)) {
        blockListElement.innerHTML = `
          <div class="block-empty-state">
            <p>Could not reach the C++ API. Start <strong>routex_api</strong> and refresh.</p>
          </div>`;
      }
    }
  }

  async function checkVerification(showToast = false) {
    if (verifyBtn) verifyBtn.disabled = true;
    try {
      const result = await api('/api/blockchain/verify');
      const valid = Boolean(result.valid);
      const errorMsg = result.error || '';
      const isTampered = Boolean(result.isTampered);

      updateIntegrityBanner(valid, errorMsg, isTampered);
      renderChainRibbon(chainBlocks);
      renderBlocks();

      if (showToast) {
        if (valid) {
          toast('Integrity Verified', 'All cryptographic blocks in the chain are valid.', 'success');
        } else {
          toast('Integrity Alert', errorMsg || 'Chain verification failed.', 'error');
        }
      }
    } catch (error) {
      updateIntegrityBanner(false, error.message);
      if (showToast) {
        toast('Verification error', error.message, 'error');
      }
    } finally {
      if (verifyBtn) verifyBtn.disabled = false;
    }
  }

  async function simulateTampering() {
    if (tamperBtn) tamperBtn.disabled = true;
    try {
      await api('/api/blockchain/tamper', { method: 'POST', body: '{}' });
      toast('Tampering Simulated', 'Modified block data in memory without recalculating SHA-256 hashes.', 'error');
      // Reload and verify
      const result = await api('/api/blockchain');
      chainBlocks = Array.isArray(result.blocks) ? result.blocks : [];
      updateMetrics(chainBlocks);
      renderFilterChips(chainBlocks);
      await checkVerification(false);
      renderChainRibbon(chainBlocks);
      renderBlocks();
    } catch (error) {
      toast('Tampering Simulation Failed', error.message, 'error');
    } finally {
      if (tamperBtn) tamperBtn.disabled = false;
    }
  }

  async function restoreBlockchain() {
    if (restoreBtn) restoreBtn.disabled = true;
    try {
      await api('/api/blockchain/restore', { method: 'POST', body: '{}' });
      toast('Blockchain Restored', 'Reverted memory modifications to original state.', 'success');
      // Reload and verify
      const result = await api('/api/blockchain');
      chainBlocks = Array.isArray(result.blocks) ? result.blocks : [];
      updateMetrics(chainBlocks);
      renderFilterChips(chainBlocks);
      await checkVerification(false);
      renderChainRibbon(chainBlocks);
      renderBlocks();
    } catch (error) {
      toast('Restore Failed', error.message, 'error');
    } finally {
      if (restoreBtn) restoreBtn.disabled = false;
    }
  }

  function toggleAllBlocks() {
    const allExpanded = chainBlocks.every((b) => expandedBlockSet.has(b.index));
    if (allExpanded) {
      expandedBlockSet.clear();
      if (toggleExpandAllBtn) toggleExpandAllBtn.textContent = 'Expand all blocks';
    } else {
      chainBlocks.forEach((b) => expandedBlockSet.add(b.index));
      if (toggleExpandAllBtn) toggleExpandAllBtn.textContent = 'Collapse all blocks';
    }
    renderBlocks();
  }

  async function updateNavCount() {
    try {
      const result = await api('/api/shipments');
      const count = document.getElementById('navShipmentCount');
      if (count) count.textContent = String(result.count ?? (result.shipments || []).length);
    } catch (_) {}
  }

  // Register Modal Handlers
  function openModal() {
    if (!registerModal) return;
    // Suggest next shipment ID
    let maxId = 100;
    chainBlocks.forEach((b) => {
      (b.transactions || []).forEach((tx) => {
        maxId = Math.max(maxId, Number(tx.shipmentID) || 0);
      });
    });
    if (regShipmentID) regShipmentID.value = String(maxId + 1);
    if (regStatus) regStatus.value = 'Registered event';

    if (typeof registerModal.showModal === 'function') {
      registerModal.showModal();
    } else {
      registerModal.setAttribute('open', '');
    }
  }

  function closeModal() {
    if (!registerModal) return;
    if (typeof registerModal.close === 'function') {
      registerModal.close();
    } else {
      registerModal.removeAttribute('open');
    }
  }

  async function handleRegisterSubmit(e) {
    if (e) e.preventDefault();
    const shipmentID = Number(regShipmentID.value);
    const eventType = regEventType.value;
    const location = regLocation.value;
    const status = regStatus.value || eventType;

    if (!shipmentID || !eventType || !location) {
      toast('Form Error', 'Please fill in all required fields.', 'error');
      return;
    }

    const submitBtn = document.getElementById('submitRegisterBtn');
    if (submitBtn) submitBtn.disabled = true;

    try {
      await api('/api/blockchain/register', {
        method: 'POST',
        body: JSON.stringify({ shipmentID, eventType, location, status })
      });

      closeModal();
      toast('Block Event Registered', `${eventType} event recorded for Shipment #${shipmentID}`, 'success');
      await loadBlockchainData(false);
    } catch (error) {
      toast('Registration Failed', error.message, 'error');
    } finally {
      if (submitBtn) submitBtn.disabled = false;
    }
  }

  document.addEventListener('DOMContentLoaded', () => {
    loadBlockchainData(false).then(() => updateNavCount());

    // Hook auto-reconnect from app.js
    if (typeof onReconnect === 'function') {
      onReconnect(() => {
        loadBlockchainData(false);
        updateNavCount();
      });
    }

    if (verifyBtn) verifyBtn.addEventListener('click', () => checkVerification(true));
    if (tamperBtn) tamperBtn.addEventListener('click', simulateTampering);
    if (restoreBtn) restoreBtn.addEventListener('click', restoreBlockchain);
    if (refreshBtn) refreshBtn.addEventListener('click', () => loadBlockchainData(true));
    if (toggleExpandAllBtn) toggleExpandAllBtn.addEventListener('click', toggleAllBlocks);

    // Modal listeners
    if (openRegisterModalBtn) openRegisterModalBtn.addEventListener('click', openModal);
    if (closeRegisterModalBtn) closeRegisterModalBtn.addEventListener('click', closeModal);
    if (cancelRegisterModalBtn) cancelRegisterModalBtn.addEventListener('click', closeModal);
    if (registerBlockForm) registerBlockForm.addEventListener('submit', handleRegisterSubmit);

    // Modal quick presets
    if (presetCreatedBtn) {
      presetCreatedBtn.addEventListener('click', () => {
        if (regEventType) regEventType.value = 'CREATED';
        if (regLocation) regLocation.value = 'Karachi';
        if (regStatus) regStatus.value = 'Shipment created at origin';
      });
    }
    if (presetDispatchedBtn) {
      presetDispatchedBtn.addEventListener('click', () => {
        if (regEventType) regEventType.value = 'DISPATCHED';
        if (regLocation) regLocation.value = 'Sukkur';
        if (regStatus) regStatus.value = 'Package dispatched to transit hub';
      });
    }
    if (presetArrivedBtn) {
      presetArrivedBtn.addEventListener('click', () => {
        if (regEventType) regEventType.value = 'ARRIVED';
        if (regLocation) regLocation.value = 'Lahore';
        if (regStatus) regStatus.value = 'Arrived at destination warehouse';
      });
    }

    // Filter listeners
    if (filterInput) {
      filterInput.addEventListener('input', (e) => {
        const val = e.target.value.trim();
        if (val) {
          applyFilter(Number(val));
        } else {
          clearFilter();
        }
      });
    }

    if (clearFilterBtn) clearFilterBtn.addEventListener('click', clearFilter);
    if (closeAuditBtn) closeAuditBtn.addEventListener('click', clearFilter);
  });
})();
