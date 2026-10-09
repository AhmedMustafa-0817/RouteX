#ifndef GRAPH_H
#define GRAPH_H

#include <string>
class CityGraph
{
private:
    struct CityNode;
    struct EdgeNode
    {
        struct CityNode* destination;
        int travelTime;
        EdgeNode* next;

        EdgeNode(struct CityNode* city, int time);
    };
    struct CityNode
    {
        std::string name;
        EdgeNode* neighbors;
        CityNode* next;

        CityNode(const std::string& cityName);
    };

    CityNode* head;

    CityNode* findCity(const std::string& name) const;
    EdgeNode* findEdge(CityNode* city, CityNode* destination) const;

public:
    CityGraph();
    ~CityGraph();
    CityGraph(const CityGraph&) = delete;
    CityGraph& operator=(const CityGraph&) = delete;

    bool addCity(const std::string& name);
    bool addEdge(const std::string& city1,
                 const std::string& city2,
                 int travelTime);

    void displayGraph() const;
};

#endif