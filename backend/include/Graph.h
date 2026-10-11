#ifndef GRAPH_H
#define GRAPH_H

#include <string>

constexpr int MAX_GRAPH_CITIES = 100;

struct RouteResult
{
    bool found;
    double totalHours;
    int pathLength;
    std::string path[MAX_GRAPH_CITIES];

    RouteResult()
    {
        found = false;
        totalHours = 0.0;
        pathLength = 0;
    }
};

class CityGraph
{
private:
    struct EdgeNode;

    struct CityNode
    {
        std::string name;
        EdgeNode* neighbors;
        CityNode* next;
        int index;

        CityNode(const std::string& cityName);
    };

    struct EdgeNode
    {
        CityNode* destination;
        int travelTime;
        EdgeNode* next;

        EdgeNode(CityNode* city, int time);
    };

    CityNode* head;
    int cityCount;

    CityNode* findCity(const std::string& name) const;

    EdgeNode* findEdge(
        CityNode* city,
        CityNode* destination
    ) const;

public:
    CityGraph();
    ~CityGraph();

    CityGraph(const CityGraph&) = delete;
    CityGraph& operator=(const CityGraph&) = delete;

    bool addCity(const std::string& name);

    bool addEdge(
        const std::string& city1,
        const std::string& city2,
        int travelTime
    );

    void displayGraph() const;

    RouteResult dijkstra(
        const std::string& source,
        const std::string& destination
    ) const;
};

#endif