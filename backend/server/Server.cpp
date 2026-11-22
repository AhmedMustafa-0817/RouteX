#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
#define CLOSE_SOCKET closesocket
#define SHUTDOWN_SEND SD_SEND
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
using SocketHandle = int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define CLOSE_SOCKET close
#define SHUTDOWN_SEND SHUT_WR
#define WSACleanup() ((void)0)
#endif

#include "../include/ShipmentManager.h"
#include "../include/Graph.h"
#include "../include/RouteService.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>

using std::string;
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;

// Escape special characters for JSON.
string jsonEscape(const string& value)
{
    string result;

    for (char c : value)
    {
        switch (c)
        {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) >= 32)
                result += c;
        }
    }

    return result;
}

string jsonString(const string& value)
{
    return "\"" + jsonEscape(value) + "\"";
}

bool isJsonSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Position of the value that follows "key": in a flat JSON object,
// or string::npos. Only matches a quoted key that is followed by a colon,
// so a string *value* that happens to equal the key is not mistaken for it.
size_t findJsonValue(const string& body, const string& key)
{
    string search = "\"" + key + "\"";
    size_t pos = 0;

    while ((pos = body.find(search, pos)) != string::npos)
    {
        size_t after = pos + search.size();

        while (after < body.size() && isJsonSpace(body[after]))
            ++after;

        if (after < body.size() && body[after] == ':')
        {
            ++after;
            while (after < body.size() && isJsonSpace(body[after]))
                ++after;
            return after;
        }

        pos = after;
    }

    return string::npos;
}

void appendUtf8(string& out, unsigned int codePoint)
{
    if (codePoint < 0x80)
    {
        out += static_cast<char>(codePoint);
    }
    else if (codePoint < 0x800)
    {
        out += static_cast<char>(0xC0 | (codePoint >> 6));
        out += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else if (codePoint < 0x10000)
    {
        out += static_cast<char>(0xE0 | (codePoint >> 12));
        out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else
    {
        out += static_cast<char>(0xF0 | (codePoint >> 18));
        out += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}

bool readHex4(const string& s, size_t pos, unsigned int& value)
{
    if (pos + 4 > s.size())
        return false;

    value = 0;
    for (size_t i = pos; i < pos + 4; ++i)
    {
        char c = s[i];
        value <<= 4;
        if (c >= '0' && c <= '9') value |= static_cast<unsigned int>(c - '0');
        else if (c >= 'a' && c <= 'f') value |= static_cast<unsigned int>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') value |= static_cast<unsigned int>(c - 'A' + 10);
        else return false;
    }
    return true;
}

// Read a string value from a simple JSON request body, decoding escapes
// (\" \\ \/ \b \f \n \r \t \uXXXX). Returns "" if missing or not a string.
string getJsonString(const string& body, const string& key)
{
    size_t pos = findJsonValue(body, key);

    if (pos == string::npos || pos >= body.size() || body[pos] != '"')
        return "";

    string result;

    for (size_t i = pos + 1; i < body.size(); ++i)
    {
        char c = body[i];

        if (c == '"')
            return result;

        if (c != '\\')
        {
            result += c;
            continue;
        }

        if (++i >= body.size())
            return "";

        switch (body[i])
        {
        case '"':  result += '"';  break;
        case '\\': result += '\\'; break;
        case '/':  result += '/';  break;
        case 'b':  result += '\b'; break;
        case 'f':  result += '\f'; break;
        case 'n':  result += '\n'; break;
        case 'r':  result += '\r'; break;
        case 't':  result += '\t'; break;
        case 'u':
        {
            unsigned int code = 0;
            if (!readHex4(body, i + 1, code))
                return "";
            i += 4;

            // Combine a UTF-16 surrogate pair into one code point.
            unsigned int low = 0;
            if (code >= 0xD800 && code <= 0xDBFF &&
                i + 2 < body.size() && body[i + 1] == '\\' && body[i + 2] == 'u' &&
                readHex4(body, i + 3, low) && low >= 0xDC00 && low <= 0xDFFF)
            {
                code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                i += 6;
            }

            appendUtf8(result, code);
            break;
        }
        default:
            return "";
        }
    }

    return ""; // unterminated string
}

// Read a finite numeric value from a simple JSON request body.
bool getJsonNumber(
    const string& body,
    const string& key,
    double& value)
{
    size_t pos = findJsonValue(body, key);

    if (pos == string::npos || pos >= body.size())
        return false;

    // strtod also accepts "inf", "nan" and hex floats; JSON numbers
    // always start with '-' or a digit.
    char first = body[pos];
    if (first != '-' && (first < '0' || first > '9'))
        return false;

    char* end = nullptr;
    value = strtod(body.c_str() + pos, &end);

    return end != body.c_str() + pos && std::isfinite(value);
}

// Read a positive whole-number ID that fits in an int.
bool getJsonPositiveInt(const string& body, const string& key, int& value)
{
    double number = 0;

    if (!getJsonNumber(body, key, number) ||
        number < 1 || number > INT_MAX || number != std::floor(number))
        return false;

    value = static_cast<int>(number);
    return true;
}

const size_t MAX_TEXT_FIELD_LENGTH = 64;

// Trim surrounding spaces and reject empty, overlong or control-character
// text, so every stored value round-trips safely through CSV and JSON.
bool cleanTextField(string& value)
{
    size_t start = value.find_first_not_of(" \t");
    size_t end = value.find_last_not_of(" \t");

    if (start == string::npos)
    {
        value.clear();
        return false;
    }

    value = value.substr(start, end - start + 1);

    if (value.size() > MAX_TEXT_FIELD_LENGTH)
        return false;

    for (char c : value)
    {
        unsigned char u = static_cast<unsigned char>(c);
        if (u < 32 || u == 127)
            return false;
    }

    return true;
}

// Cities in the sample route network (see setupRoutes).
const char* const ROUTE_CITIES[] = {
    "Karachi", "Hyderabad", "Sukkur", "Quetta",
    "Multan", "Lahore", "Islamabad", "Peshawar"
};

bool equalsIgnoreCase(const string& a, const string& b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }

    return true;
}

// Map user input such as " lahore " to the graph's spelling "Lahore".
// Returns "" when the city is not part of the network.
string canonicalCity(string name)
{
    if (!cleanTextField(name))
        return "";

    for (const char* city : ROUTE_CITIES)
    {
        if (equalsIgnoreCase(name, city))
            return city;
    }

    return "";
}

// Build an HTTP response.
string httpResponse(
    int status,
    const string& body,
    const string& statusText = "OK")
{
    stringstream response;

    response << "HTTP/1.1 " << status << " "
             << statusText << "\r\n";
    response << "Content-Type: application/json; charset=utf-8\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With, Access-Control-Request-Private-Network\r\n";
    response << "Access-Control-Allow-Private-Network: true\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;

    return response.str();
}

bool sendAll(SocketHandle client, const string& data)
{
#ifndef _WIN32
    // On macOS, writing to a disconnected client can otherwise raise SIGPIPE
    // and terminate the whole API process. Keep the failure local to this request.
    int suppressSigPipe = 1;
    setsockopt(
        client,
        SOL_SOCKET,
        SO_NOSIGPIPE,
        &suppressSigPipe,
        sizeof(suppressSigPipe)
    );
#endif

    size_t sent = 0;

    while (sent < data.size())
    {
        int result = send(
            client,
            data.c_str() + sent,
            static_cast<int>(data.size() - sent),
            0
        );

        if (result <= 0)
            return false;

        sent += static_cast<size_t>(result);
    }

    return true;
}

// API endpoints.
string handleRequest(
    const string& method,
    const string& path,
    const string& body,
    ShipmentManager& manager,
    CityGraph& graph)
{
    if (method == "OPTIONS")
        return httpResponse(200, "{\"ok\":true}");

    if (method == "GET" && path == "/")
    {
        return httpResponse(
            200,
            "{\"name\":\"RouteX API\",\"status\":\"running\"}"
        );
    }

    // GET /api/shipments
    if (method == "GET" && path == "/api/shipments")
    {
        std::vector<Shipment> shipments(manager.getShipmentCount());
        int count = manager.getAllShipments(
            shipments.data(),
            static_cast<int>(shipments.size()));

        stringstream json;
        json << "{\"shipments\":[";

        for (int i = 0; i < count; ++i)
        {
            if (i > 0)
                json << ",";

            json << "{"
                 << "\"shipmentID\":"
                 << shipments[i].getShipmentID() << ","
                 << "\"sourceCity\":"
                 << jsonString(shipments[i].getSourceCity()) << ","
                 << "\"destinationCity\":"
                 << jsonString(shipments[i].getDestinationCity()) << ","
                 << "\"packageType\":"
                 << jsonString(shipments[i].getPackageType()) << ","
                 << "\"weight\":"
                 << shipments[i].getWeight() << ","
                 << "\"status\":"
                 << jsonString(shipments[i].getStatus())
                 << "}";
        }

        json << "],\"count\":" << count << "}";
        return httpResponse(200, json.str());
    }

    // POST /api/shipments
    if (method == "POST" && path == "/api/shipments")
    {
        int shipmentID = 0;
        double weight = 0;

        if (!getJsonPositiveInt(body, "shipmentID", shipmentID) ||
            !getJsonNumber(body, "weight", weight))
        {
            return httpResponse(
                400,
                "{\"error\":\"A positive whole-number shipmentID and a numeric weight are required\"}",
                "Bad Request"
            );
        }

        string source = getJsonString(body, "sourceCity");
        string destination = getJsonString(body, "destinationCity");
        string packageType = getJsonString(body, "packageType");
        string status = getJsonString(body, "status");

        if (!cleanTextField(source) ||
            !cleanTextField(destination) ||
            !cleanTextField(packageType) ||
            weight <= 0 || weight > 100000)
        {
            return httpResponse(
                400,
                "{\"error\":\"Invalid shipment fields: text must be 1-64 characters without line breaks, weight must be between 0 and 100000\"}",
                "Bad Request"
            );
        }

        if (status.empty())
            status = "Pending";
        else if (!cleanTextField(status))
            return httpResponse(400, "{\"error\":\"Invalid status\"}", "Bad Request");

        Shipment shipment(
            shipmentID,
            source,
            destination,
            packageType,
            weight,
            status
        );

        if (!manager.createShipment(shipment))
        {
            return httpResponse(
                409,
                "{\"error\":\"Shipment could not be created; ID may already exist\"}",
                "Conflict"
            );
        }

        if (!manager.saveAll("data/shipments.csv"))
        {
            return httpResponse(
                500,
                "{\"error\":\"Shipment created in memory but CSV persistence failed\"}",
                "Internal Server Error"
            );
        }

        return httpResponse(
            201,
            "{\"message\":\"Shipment created successfully\"}",
            "Created"
        );
    }

    // POST /api/shipments/search
    // Uses ShipmentManager::searchShipment(), which delegates to the custom Hash Table.
    if (method == "POST" && path == "/api/shipments/search")
    {
        int shipmentID = 0;

        if (!getJsonPositiveInt(body, "shipmentID", shipmentID))
        {
            return httpResponse(
                400,
                "{\"error\":\"A positive integer shipmentID is required\"}",
                "Bad Request"
            );
        }

        Shipment* shipment = manager.searchShipment(shipmentID);
        if (shipment == nullptr)
        {
            return httpResponse(404, "{\"error\":\"Shipment not found\"}", "Not Found");
        }

        stringstream json;
        json << "{"
             << "\"shipmentID\":" << shipment->getShipmentID() << ","
             << "\"sourceCity\":" << jsonString(shipment->getSourceCity()) << ","
             << "\"destinationCity\":" << jsonString(shipment->getDestinationCity()) << ","
             << "\"packageType\":" << jsonString(shipment->getPackageType()) << ","
             << "\"weight\":" << shipment->getWeight() << ","
             << "\"status\":" << jsonString(shipment->getStatus())
             << "}";
        return httpResponse(200, json.str());
    }

    // POST /api/shipments/process-next
    if (method == "POST" && path == "/api/shipments/process-next")
    {
        if (!manager.processNextShipment())
        {
            return httpResponse(
                409,
                "{\"error\":\"The processing queue is empty\"}",
                "Conflict"
            );
        }

        if (!manager.saveAll("data/shipments.csv"))
        {
            return httpResponse(
                500,
                "{\"error\":\"Shipment processed in memory but CSV persistence failed\"}",
                "Internal Server Error"
            );
        }

        return httpResponse(200, "{\"message\":\"Next shipment processed\"}");
    }

    // POST /api/shipments/arrival
    // Body: { "shipmentID": 103, "location": "Hyderabad" }
    if (method == "POST" && path == "/api/shipments/arrival")
    {
        int shipmentID = 0;
        string location = getJsonString(body, "location");

        if (!getJsonPositiveInt(body, "shipmentID", shipmentID) ||
            !cleanTextField(location))
        {
            return httpResponse(
                400,
                "{\"error\":\"A valid shipmentID and location are required\"}",
                "Bad Request"
            );
        }

        if (!manager.recordArrival(shipmentID, location))
        {
            return httpResponse(
                409,
                "{\"error\":\"Shipment was not found or is not DISPATCHED\"}",
                "Conflict"
            );
        }

        if (!manager.saveAll("data/shipments.csv"))
        {
            return httpResponse(
                500,
                "{\"error\":\"Arrival recorded in memory but CSV persistence failed\"}",
                "Internal Server Error"
            );
        }

        return httpResponse(200, "{\"message\":\"Shipment arrival recorded\"}");
    }

    // POST /api/routes/fastest
    if (method == "POST" && path == "/api/routes/fastest")
    {
        string rawSource = getJsonString(body, "source");
        string rawDestination = getJsonString(body, "destination");

        if (rawSource.empty() || rawDestination.empty())
        {
            return httpResponse(
                400,
                "{\"error\":\"source and destination are required\"}",
                "Bad Request"
            );
        }

        string source = canonicalCity(rawSource);
        string destination = canonicalCity(rawDestination);

        if (source.empty() || destination.empty())
        {
            return httpResponse(
                404,
                "{\"error\":\"Unknown city. Valid cities: Karachi, Hyderabad, Sukkur, Quetta, Multan, Lahore, Islamabad, Peshawar\"}",
                "Not Found"
            );
        }

        RouteService routeService(graph);
        RouteResult route =
            routeService.fastestRoute(source, destination);

        if (!route.found)
        {
            return httpResponse(
                404,
                "{\"error\":\"No route found\"}",
                "Not Found"
            );
        }

        stringstream json;
        json << "{\"found\":true,\"totalHours\":"
             << route.totalHours << ",\"path\":[";

        for (int i = 0; i < route.pathLength; ++i)
        {
            if (i > 0)
                json << ",";

            json << jsonString(route.path[i]);
        }

        json << "]}";
        return httpResponse(200, json.str());
    }

    // GET /api/blockchain
    if (method == "GET" && path == "/api/blockchain")
    {
        stringstream json;
        json << "{\"blockCount\":"
             << manager.getBlockchainBlockCount()
             << ",\"blocks\":[";

        Block* block = manager.getBlockchainHead();
        bool firstBlock = true;

        while (block != nullptr)
        {
            if (!firstBlock)
                json << ",";
            firstBlock = false;

            json << "{"
                 << "\"index\":" << block->getIndex() << ","
                 << "\"timestamp\":" << jsonString(block->getTimestamp()) << ","
                 << "\"previousHash\":" << jsonString(block->getPreviousHash()) << ","
                 << "\"currentHash\":" << jsonString(block->getCurrentHash()) << ","
                 << "\"eventCount\":" << block->getEventCount() << ","
                 << "\"transactions\":[";

            for (int i = 0; i < block->getEventCount(); ++i)
            {
                if (i > 0)
                    json << ",";

                TransferEvent* ev = block->getEventAt(i);
                if (ev != nullptr)
                {
                    json << "{"
                         << "\"transactionID\":" << ev->getTransactionID() << ","
                         << "\"shipmentID\":" << ev->getShipmentID() << ","
                         << "\"eventType\":" << jsonString(ev->getEventType()) << ","
                         << "\"location\":" << jsonString(ev->getLocation()) << ","
                         << "\"timestamp\":" << jsonString(ev->getTimestamp()) << ","
                         << "\"status\":" << jsonString(ev->getStatus())
                         << "}";
                }
            }

            json << "]}";
            block = block->getNext();
        }

        json << "]}";
        return httpResponse(200, json.str());
    }

    // GET /api/blockchain/verify
    if (method == "GET" && path == "/api/blockchain/verify")
    {
        bool valid = manager.verifyBlockchain();

        stringstream json;
        json << "{\"valid\":"
             << (valid ? "true" : "false")
             << ",\"error\":"
             << jsonString(
                    valid ? "" :
                    manager.getBlockchainVerificationError()
                )
             << ",\"isTampered\":"
             << (manager.isTampered() ? "true" : "false")
             << "}";

        return httpResponse(200, json.str());
    }

    // POST /api/blockchain/tamper
    if (method == "POST" && path == "/api/blockchain/tamper")
    {
        if (!manager.tamperFirstEvent())
        {
            return httpResponse(
                500,
                "{\"error\":\"Failed to tamper with blockchain state\"}",
                "Internal Server Error"
            );
        }

        return httpResponse(
            200,
            "{\"message\":\"Blockchain event tampered in memory without updating SHA-256 hashes.\",\"tampered\":true}"
        );
    }

    // POST /api/blockchain/register (or /api/blockchain/events)
    if (method == "POST" && (path == "/api/blockchain/register" || path == "/api/blockchain/events"))
    {
        int shipmentID = 0;
        string eventType = getJsonString(body, "eventType");
        string location = getJsonString(body, "location");
        string status = getJsonString(body, "status");

        if (!getJsonPositiveInt(body, "shipmentID", shipmentID) ||
            !cleanTextField(eventType))
        {
            return httpResponse(
                400,
                "{\"error\":\"A valid positive shipmentID and eventType are required\"}",
                "Bad Request"
            );
        }

        if (manager.searchShipment(shipmentID) == nullptr)
        {
            return httpResponse(
                404,
                "{\"error\":\"Shipment not found; create it before registering events\"}",
                "Not Found"
            );
        }

        if (location.empty())
            location = "Warehouse Hub";
        else if (!cleanTextField(location))
            return httpResponse(400, "{\"error\":\"Invalid location\"}", "Bad Request");

        if (status.empty())
            status = eventType;
        else if (!cleanTextField(status))
            return httpResponse(400, "{\"error\":\"Invalid status\"}", "Bad Request");

        // A manual registration always seals its own block, so the explorer
        // shows a new block for every "Record & Register Block" action.
        if (!manager.recordEvent(shipmentID, eventType, location, status, true))
        {
            return httpResponse(
                500,
                "{\"error\":\"Failed to record event to blockchain\"}",
                "Internal Server Error"
            );
        }

        stringstream json;
        json << "{\"message\":\"Event recorded in blockchain successfully\","
             << "\"blockCount\":" << manager.getBlockchainBlockCount() << ","
             << "\"blockIndex\":" << manager.getBlockchainBlockCount() - 1
             << "}";

        return httpResponse(201, json.str(), "Created");
    }

    // POST /api/blockchain/restore or /api/blockchain/reset
    if (method == "POST" && (path == "/api/blockchain/restore" || path == "/api/blockchain/reset"))
    {
        manager.restoreTamper();
        return httpResponse(
            200,
            "{\"message\":\"Blockchain restored to original untampered state.\",\"tampered\":false}"
        );
    }

    return httpResponse(
        404,
        "{\"error\":\"Endpoint not found\"}",
        "Not Found"
    );
}

// Sample route network.
void setupRoutes(CityGraph& graph)
{
    graph.addCity("Karachi");
    graph.addCity("Hyderabad");
    graph.addCity("Sukkur");
    graph.addCity("Quetta");
    graph.addCity("Multan");
    graph.addCity("Lahore");
    graph.addCity("Islamabad");
    graph.addCity("Peshawar");

    graph.addEdge("Karachi", "Hyderabad", 2);
    graph.addEdge("Hyderabad", "Sukkur", 5);
    graph.addEdge("Sukkur", "Multan", 6);
    graph.addEdge("Multan", "Lahore", 4);
    graph.addEdge("Lahore", "Islamabad", 4);
    graph.addEdge("Islamabad", "Peshawar", 3);
    graph.addEdge("Quetta", "Sukkur", 7);
    graph.addEdge("Karachi", "Quetta", 10);
}


void processClient(
    SocketHandle clientSocket,
    ShipmentManager& manager,
    CityGraph& graph,
    std::mutex& managerMutex)
{
#ifdef _WIN32
    DWORD timeoutMs = 3000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
#else
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif

    string request;
    char buffer[4096];
    int received = 0;

    size_t headerEnd = string::npos;
    size_t expectedBodyLength = 0;

    // Read HTTP headers and the declared request body.
    while ((received = recv(
                clientSocket,
                buffer,
                sizeof(buffer),
                0)) > 0)
    {
        request.append(buffer, received);

        headerEnd = request.find("\r\n\r\n");

        if (headerEnd != string::npos)
        {
            size_t contentLengthPos = string::npos;
            for (size_t i = 0; i + 15 <= headerEnd; ++i)
            {
                if ((request[i] == 'C' || request[i] == 'c') &&
                    (request[i+1] == 'o' || request[i+1] == 'O') &&
                    (request[i+2] == 'n' || request[i+2] == 'N') &&
                    (request[i+3] == 't' || request[i+3] == 'T') &&
                    (request[i+4] == 'e' || request[i+4] == 'E') &&
                    (request[i+5] == 'n' || request[i+5] == 'N') &&
                    (request[i+6] == 't' || request[i+6] == 'T') &&
                    request[i+7] == '-' &&
                    (request[i+8] == 'l' || request[i+8] == 'L') &&
                    (request[i+9] == 'e' || request[i+9] == 'E') &&
                    (request[i+10] == 'n' || request[i+10] == 'N') &&
                    (request[i+11] == 'g' || request[i+11] == 'G') &&
                    (request[i+12] == 't' || request[i+12] == 'T') &&
                    (request[i+13] == 'h' || request[i+13] == 'H') &&
                    request[i+14] == ':')
                {
                    contentLengthPos = i + 15;
                    break;
                }
            }

            if (contentLengthPos != string::npos)
            {
                while (contentLengthPos < headerEnd &&
                       (request[contentLengthPos] == ' ' ||
                        request[contentLengthPos] == '\t'))
                    ++contentLengthPos;

                expectedBodyLength = static_cast<size_t>(
                    strtoul(
                        request.c_str() + contentLengthPos,
                        nullptr,
                        10
                    )
                );
            }

            size_t bodyStart = headerEnd + 4;

            if (request.size() >= bodyStart + expectedBodyLength)
                break;
        }

        if (request.size() > 1024 * 1024)
            break;
    }

    if (request.empty())
    {
        CLOSE_SOCKET(clientSocket);
        return;
    }

    string method;
    string path;
    string body;

    size_t firstSpace = request.find(' ');
    size_t secondSpace =
        firstSpace == string::npos
            ? string::npos
            : request.find(' ', firstSpace + 1);

    headerEnd = request.find("\r\n\r\n");

    if (firstSpace != string::npos &&
        secondSpace != string::npos &&
        headerEnd != string::npos)
    {
        method = request.substr(0, firstSpace);
        path = request.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1
        );

        body = request.substr(headerEnd + 4);

        size_t queryPos = path.find('?');

        if (queryPos != string::npos)
            path = path.substr(0, queryPos);
    }

    string response;

    if (method.empty() || path.empty())
    {
        response = httpResponse(
            400,
            "{\"error\":\"Malformed HTTP request\"}",
            "Bad Request"
        );
    }
    else
    {
        std::lock_guard<std::mutex> lock(managerMutex);
        response = handleRequest(
            method,
            path,
            body,
            manager,
            graph
        );
    }

    sendAll(clientSocket, response);

    shutdown(clientSocket, SHUTDOWN_SEND);
#ifdef _WIN32
    DWORD drainTimeout = 50;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&drainTimeout), sizeof(drainTimeout));
#endif
    char drain[512];
    while (recv(clientSocket, drain, sizeof(drain), 0) > 0) {}
    CLOSE_SOCKET(clientSocket);
}

int main(int argc, char* argv[])
{
    int serverPort = 8090;
    if (argc > 1)
    {
        char* end = nullptr;
        long requestedPort = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || requestedPort < 1024 || requestedPort > 65535)
        {
            cerr << "Usage: routex_api [port (1024-65535)]\n";
            return 1;
        }
        serverPort = static_cast<int>(requestedPort);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "Winsock initialization failed.\n";
        return 1;
    }
#endif

    SocketHandle serverSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<unsigned short>(serverPort));

    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int reuseAddress = 1;
    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuseAddress),
        sizeof(reuseAddress)
    );

    if (::bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)) == -1)
    {
        cerr << "Bind failed. Port " << serverPort << " may already be in use.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Listen failed.\n";
        CLOSE_SOCKET(serverSocket);
        WSACleanup();
        return 1;
    }

    ShipmentManager manager;
    if (!manager.loadAll("data/shipments.csv"))
        cout << "Warning: could not load data/shipments.csv; starting with an empty registry.\n";

    CityGraph graph;
    setupRoutes(graph);

    cout << "RouteX API running at http://127.0.0.1:" << serverPort << std::endl;

    std::mutex managerMutex;

    while (true)
    {
        SocketHandle clientSocket =
            accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET)
            continue;

        std::thread clientThread(
            processClient,
            clientSocket,
            std::ref(manager),
            std::ref(graph),
            std::ref(managerMutex)
        );
        clientThread.detach();
    }

    CLOSE_SOCKET(serverSocket);
    WSACleanup();
    return 0;
}
