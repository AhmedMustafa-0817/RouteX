RouteX API Documentation

Base URL

http://127.0.0.1:8080

1. Get All Shipments

Endpoint: GET /api/shipments

Returns all shipments stored in the backend, along with the total count.

2. Create a Shipment

Endpoint: POST /api/shipments

Creates a shipment in the backend.

Example JSON request:

{
  "shipmentID": 101,
  "sourceCity": "Karachi",
  "destinationCity": "Lahore",
  "packageType": "Documents",
  "weight": 2.5,
  "status": "Pending"
}

3. Find the Fastest Route

Endpoint: POST /api/routes/fastest

Example JSON request:

{
  "source": "Karachi",
  "destination": "Lahore"
}

Returns whether a route was found, the path, and total travel time in hours.

4. Get Blockchain Information

Endpoint: GET /api/blockchain

Returns the blockchain block count.

5. Verify Blockchain

Endpoint: GET /api/blockchain/verify

Example response:

{
  "valid": true,
  "error": ""
}

Running the API

Build and run the routex_api executable, then send HTTP requests to the base URL above.