#ifndef ROUTE_SERVICE_H
#define ROUTE_SERVICE_H

#include "Graph.h"
#include <string>

class RouteService
{
private:
    const CityGraph& graph;

public:
    explicit RouteService(const CityGraph& cityGraph)
        : graph(cityGraph)
    {
    }

    RouteResult fastestRoute(
        const std::string& source,
        const std::string& destination
    ) const
    {
        return graph.dijkstra(source, destination);
    }
};

#endif