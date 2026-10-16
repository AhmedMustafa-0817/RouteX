#include "../include/Graph.h"
#include <limits>
#include <algorithm>

RouteResult CityGraph::dijkstra(
    const std::string& source,
    const std::string& destination) const
{
    RouteResult result;

    CityNode* start = findCity(source);
    CityNode* finish = findCity(destination);

    if (start == nullptr || finish == nullptr)
        return result;

    if (cityCount <= 0 || cityCount > MAX_GRAPH_CITIES)
        return result;

    CityNode* cities[MAX_GRAPH_CITIES] = {};
    double distance[MAX_GRAPH_CITIES];
    int previous[MAX_GRAPH_CITIES];
    bool visited[MAX_GRAPH_CITIES] = {};

    const double INF = std::numeric_limits<double>::infinity();

    for (int i = 0; i < cityCount; ++i)
    {
        distance[i] = INF;
        previous[i] = -1;
    }

    for (CityNode* city = head; city != nullptr; city = city->next)
    {
        if (city->index < 0 || city->index >= cityCount)
            return result;

        cities[city->index] = city;
    }

    distance[start->index] = 0.0;

    for (int step = 0; step < cityCount; ++step)
    {
        int current = -1;
        double bestDistance = INF;

        for (int i = 0; i < cityCount; ++i)
        {
            if (!visited[i] && distance[i] < bestDistance)
            {
                bestDistance = distance[i];
                current = i;
            }
        }

        if (current == -1)
            break;

        if (current == finish->index)
            break;

        visited[current] = true;

        CityNode* currentCity = cities[current];

        if (currentCity == nullptr)
            continue;

        for (EdgeNode* edge = currentCity->neighbors;
             edge != nullptr;
             edge = edge->next)
        {
            if (edge->destination == nullptr ||
                edge->travelTime <= 0)
                continue;

            int next = edge->destination->index;

            if (next < 0 || next >= cityCount || visited[next])
                continue;

            double newDistance =
                distance[current] + edge->travelTime;

            if (newDistance < distance[next])
            {
                distance[next] = newDistance;
                previous[next] = current;
            }
        }
    }

    int destinationIndex = finish->index;

    if (distance[destinationIndex] == INF)
        return result;

    int reversedPath[MAX_GRAPH_CITIES];
    int pathCount = 0;
    int current = destinationIndex;

    while (current != -1 && pathCount < MAX_GRAPH_CITIES)
    {
        reversedPath[pathCount++] = current;

        if (current == start->index)
            break;

        current = previous[current];
    }

    if (pathCount == 0 ||
        reversedPath[pathCount - 1] != start->index)
        return result;

    result.found = true;
    result.totalHours = distance[destinationIndex];
    result.pathLength = pathCount;

    for (int i = 0; i < pathCount; ++i)
    {
        int cityIndex = reversedPath[pathCount - 1 - i];

        if (cities[cityIndex] == nullptr)
            return RouteResult();

        result.path[i] = cities[cityIndex]->name;
    }

    return result;
}