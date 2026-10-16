# RouteX API Documentation

Base URL: `http://127.0.0.1:8080`

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
Returns the current in-memory block count.

### `GET /api/blockchain/verify`
Returns `{"valid":true,"error":""}` or a verification error.

## Running

Build the project with CMake, then start `routex_api` from the repository root so the relative path `data/shipments.csv` resolves correctly. The API listens on `http://127.0.0.1:8080` by default. To avoid a port conflict, pass another port such as `18080` as the executable argument and update `ROUTEX_API_BASE` in `frontend/js/app.js` to match. Serve the `frontend/` directory with a local static server (for example, VS Code Live Server) and open `index.html` or `orders.html`.

The current CSV format is intentionally simple: fields must not contain commas or line breaks. The blockchain is currently in memory; shipment rows persist in the CSV, but blockchain history is not persisted between server restarts.
