# RouteX API Documentation

Base URL: `http://127.0.0.1:8090`

The API is a lightweight C++ HTTP server. The browser UI sends requests here; shipment registry, queue, and blockchain operations remain in the C++ backend.

## Shipment operations

### `GET /api/shipments`
Returns `{"shipments":[...],"count":2}` with the current shipment registry.

### `POST /api/shipments`
Creates a shipment, adds it to the custom Hash Table and FIFO queue, records a CREATED transaction, and persists the shipment CSV.

Example JSON body:

```json
{
  "shipmentID": 105,
  "sourceCity": "Karachi",
  "destinationCity": "Lahore",
  "packageType": "Documents",
  "weight": 2.5,
  "status": "Pending"
}
```

### `POST /api/shipments/search`
Searches by shipment ID through `ShipmentManager::searchShipment()` and the custom Hash Table.

```json
{"shipmentID":105}
```

Returns one shipment record, or `404` when no match exists.

### `POST /api/shipments/process-next`
Removes the next shipment from the FIFO processing queue, changes its status to `DISPATCHED`, records a dispatch transaction, and persists its updated status. Returns `409` when the queue is empty.

Request body: `{}`.

### `POST /api/shipments/arrival`
Records the ARRIVED event and changes the status to `ARRIVED`. The shipment must currently be `DISPATCHED`.

```json
{"shipmentID":105,"location":"Lahore"}
```

The updated shipment is persisted to `data/shipments.csv`.

## Route operations

### `POST /api/routes/fastest`
Request body:

```json
{"source":"Karachi","destination":"Lahore"}
```

Returns `found`, a city `path`, and `totalHours` from the graph/Dijkstra implementation. Returns `404` if no route is found.

## Blockchain operations

### `GET /api/blockchain`
Returns `{"blockCount": N, "blocks": [...]}` containing all blocks and their transactions serialized from the in-memory linked-list blockchain.

Example response:
```json
{
  "blockCount": 2,
  "blocks": [
    {
      "index": 0,
      "timestamp": "GENESIS",
      "previousHash": "0",
      "currentHash": "de382a9...",
      "eventCount": 0,
      "transactions": []
    },
    {
      "index": 1,
      "timestamp": "2026-11-03 11:05:26",
      "previousHash": "de382a9...",
      "currentHash": "30d10ea...",
      "eventCount": 2,
      "transactions": [
        {
          "transactionID": 1,
          "shipmentID": 103,
          "eventType": "CREATED",
          "location": "Karachi",
          "timestamp": "2026-11-03 11:05:26",
          "status": "CREATED"
        }
      ]
    }
  ]
}
```

### `GET /api/blockchain/verify`
Runs `verifyChain()` across the entire linked list, recalculating SHA-256 hashes for each block and verifying `previousHash` links.
Returns `{"valid": true, "error": "", "isTampered": false}` or `{"valid": false, "error": "...", "isTampered": true}`.

### `POST /api/blockchain/register`
Records a new transfer event directly into the blockchain from the frontend explorer, appending a new block when batch size is reached.

Example request body:
```json
{
  "shipmentID": 105,
  "eventType": "CREATED",
  "location": "Karachi",
  "status": "CREATED"
}
```

### `POST /api/blockchain/tamper`
Simulates unauthorized tampering by modifying an event's field in memory without updating block cryptographic hashes. Used to demonstrate real-time tamper detection in the explorer UI.

### `POST /api/blockchain/restore` (or `/reset`)
Restores the original blockchain event state so verification passes again.

## Running

Build the project with CMake, then start `routex_api` from the repository root so the relative path `data/shipments.csv` resolves correctly. The API listens on `http://127.0.0.1:8090` by default. To specify a custom port, pass it as an argument (e.g. `./build/routex_api.exe 8090`). Serve the `frontend/` directory with a local static server (for example, VS Code Live Server) and open `index.html`, `orders.html`, `route.html`, or `blockchain.html`.

The current CSV format is intentionally simple: fields must not contain commas or line breaks. The blockchain is maintained in memory during runtime with verified SHA-256 block linking.
