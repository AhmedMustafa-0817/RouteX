# RouteX API Documentation

Base URL: `http://127.0.0.1:8090`

The API is a lightweight C++ HTTP server. The browser UI sends requests here; shipment registry, queue, and blockchain operations remain in the C++ backend.

## Input rules (all endpoints)

- IDs (`shipmentID`) must be JSON numbers that are whole, positive and fit in a 32-bit int. Strings such as `"105"`, fractions, `NaN` or infinity return `400`.
- Text fields (`sourceCity`, `destinationCity`, `packageType`, `status`, `location`, `eventType`) are trimmed and must be 1–64 characters without line breaks or other control characters. Commas, quotes and Unicode are allowed (standard JSON escapes such as `\"` and `é` are decoded).
- `weight` must be a finite number greater than 0 and at most 100000.
- Errors always come back as `{"error":"..."}` with a 4xx/5xx status.

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

Returns `found`, a city `path`, and `totalHours` from the graph/Dijkstra implementation. City names are matched case-insensitively (`"lahore"` works). Returns `404` for a city outside the network (Karachi, Hyderabad, Sukkur, Quetta, Multan, Lahore, Islamabad, Peshawar) or if no route is found.

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
Records a new transfer event directly into the blockchain from the frontend explorer. The shipment must already exist (`404` otherwise). Each registration always seals its own new block. Automatic lifecycle events (shipment created, dispatched or arrived) still batch up to 2 events per block.

Example response (`201 Created`):
```json
{ "message": "Event recorded in blockchain successfully", "blockCount": 4, "blockIndex": 3 }
```

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

Build the project with CMake, then start `routex_api` from the repository root so the relative path `data/shipments.csv` resolves correctly. On macOS or Linux, run `./build/routex_api`; on Windows, run `build\\Debug\\routex_api.exe` for a Visual Studio generator or `build\\routex_api.exe` for a single-configuration generator. The API listens on `http://127.0.0.1:8090` by default. To specify a custom port, pass it as an argument (for example, `./build/routex_api 8091` on macOS/Linux). Serve the `frontend/` directory with a local static server (for example, VS Code Live Server) and open `index.html`, `orders.html`, `route.html`, or `blockchain.html`.

`data/shipments.csv` uses standard CSV quoting, so fields containing commas or quotes are stored safely. Each save writes a temporary file and then replaces the original in one step, so a crash mid-save cannot truncate the data. There is no limit on the number of shipments.

The blockchain is kept in memory during runtime with verified SHA-256 block linking. It is **not** persisted: on restart it is rebuilt with one `CREATED` event per shipment in the CSV, so earlier DISPATCHED/ARRIVED/registered events and their original timestamps are not kept.

Run the unit tests with `ctest --test-dir build --output-on-failure`.
