#include <winsock2.h>
#include <ws2tcpip.h>

#include "../include/ShipmentManager.h"
#include "../include/Graph.h"
#include "../include/RouteService.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>

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

// Read a string value from a simple JSON request body.
string getJsonString(const string& body, const string& key)
{
    string search = "\"" + key + "\"";
    size_t pos = body.find(search);

    if (pos == string::npos)
        return "";

    pos = body.find(':', pos + search.size());

    if (pos == string::npos)
        return "";

    pos = body.find('"', pos + 1);

    if (pos == string::npos)
        return "";

    size_t end = body.find('"', pos + 1);

    if (end == string::npos)
        return "";

    return body.substr(pos + 1, end - pos - 1);
}

// Read a numeric value from a simple JSON request body.
bool getJsonNumber(
    const string& body,
    const string& key,
    double& value)
{
    string search = "\"" + key + "\"";
    size_t pos = body.find(search);

    if (pos == string::npos)
        return false;

    pos = body.find(':', pos + search.size());

    if (pos == string::npos)
        return false;

    ++pos;

    while (pos < body.size() &&
           (body[pos] == ' ' || body[pos] == '\t'))
        ++pos;

    char* end = nullptr;
    value = strtod(body.c_str() + pos, &end);

    return end != body.c_str() + pos;
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
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;

    return response.str();
}

bool sendAll(SOCKET client, const string& data)
{
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
        Shipment shipments[500];
        int count = manager.getAllShipments(shipments, 500);

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
        double idValue = 0;
        double weight = 0;

        if (!getJsonNumber(body, "shipmentID", idValue) ||
            !getJsonNumber(body, "weight", weight))
        {
            return httpResponse(
                400,
                "{\"error\":\"shipmentID and weight are required\"}",
                "Bad Request"
            );
        }

        string source = getJsonString(body, "sourceCity");
        string destination = getJsonString(body, "destinationCity");
        string packageType = getJsonString(body, "packageType");
        string status = getJsonString(body, "status");

        if (source.empty() ||
            destination.empty() ||
            packageType.empty() ||
            idValue <= 0 ||
            idValue != static_cast<int>(idValue) ||
            weight <= 0)
        {
            return httpResponse(
                400,
                "{\"error\":\"Invalid shipment fields\"}",
                "Bad Request"
            );
        }

        if (status.empty())
            status = "Pending";

        Shipment shipment(
            static_cast<int>(idValue),
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

        return httpResponse(
            201,
            "{\"message\":\"Shipment created successfully\"}",
            "Created"
        );
    }

    // POST /api/routes/fastest
    if (method == "POST" && path == "/api/routes/fastest")
    {
        string source = getJsonString(body, "source");
        string destination = getJsonString(body, "destination");

        if (source.empty() || destination.empty())
        {
            return httpResponse(
                400,
                "{\"error\":\"source and destination are required\"}",
                "Bad Request"
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
             << "}";

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
             << "}";

        return httpResponse(200, json.str());
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

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET serverSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8081);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );

    if (bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cerr << "Bind failed. Port 8081 may already be in use.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    ShipmentManager manager;
    CityGraph graph;
    setupRoutes(graph);

    cout << "RouteX API running at http://127.0.0.1:8080\n";

    while (true)
    {
        SOCKET clientSocket =
            accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET)
            continue;

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
                size_t contentLengthPos =
                    request.find("Content-Length:");

                if (contentLengthPos != string::npos &&
                    contentLengthPos < headerEnd)
                {
                    contentLengthPos += strlen("Content-Length:");

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

                if (request.size() >=
                    bodyStart + expectedBodyLength)
                    break;
            }

            if (request.size() > 1024 * 1024)
                break;
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
            response = handleRequest(
                method,
                path,
                body,
                manager,
                graph
            );
        }

        sendAll(clientSocket, response);
        shutdown(clientSocket, SD_SEND);
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}