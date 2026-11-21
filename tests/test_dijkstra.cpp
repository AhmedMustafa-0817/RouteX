#include "../backend/include/Graph.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;

int main()
{
    CityGraph graph;

    // Create sample cities.
    assert(graph.addCity("Karachi"));
    assert(graph.addCity("Hyderabad"));
    assert(graph.addCity("Sukkur"));
    assert(graph.addCity("Multan"));
    assert(graph.addCity("Lahore"));
    assert(graph.addCity("Islamabad"));

    // Create routes with travel time in hours.
    assert(graph.addEdge("Karachi", "Hyderabad", 2.0));
    assert(graph.addEdge("Karachi", "Sukkur", 9.0));
    assert(graph.addEdge("Hyderabad", "Sukkur", 3.0));
    assert(graph.addEdge("Hyderabad", "Multan", 7.0));
    assert(graph.addEdge("Sukkur", "Multan", 2.0));
    assert(graph.addEdge("Multan", "Lahore", 3.0));
    assert(graph.addEdge("Sukkur", "Lahore", 10.0));

    // Test 1: Find the fastest route.
    RouteResult route = graph.dijkstra("Karachi", "Lahore");

    assert(route.found);
    assert(fabs(route.totalHours - 10.0) < 0.000001);

    string expectedPath[] = {
        "Karachi",
        "Hyderabad",
        "Sukkur",
        "Multan",
        "Lahore"
    };

    assert(route.pathLength == 5);

    for (int i = 0; i < route.pathLength; i++)
    {
        assert(route.path[i] == expectedPath[i]);
    }

    cout << "Test 1 passed: fastest route found." << endl;
    cout << "Route: ";

    for (int i = 0; i < route.pathLength; i++)
    {
        cout << route.path[i];

        if (i < route.pathLength - 1)
            cout << " -> ";
    }

    cout << "\nTotal travel time: "
         << route.totalHours << " hours\n\n";

    // Test 2: Source and destination are the same.
    RouteResult sameCity = graph.dijkstra("Karachi", "Karachi");

    assert(sameCity.found);
    assert(sameCity.totalHours == 0.0);
    assert(sameCity.pathLength == 1);
    assert(sameCity.path[0] == "Karachi");

    cout << "Test 2 passed: source equals destination." << endl;

    // Test 3: Destination is unreachable.
    RouteResult unreachable =
        graph.dijkstra("Karachi", "Islamabad");

    assert(!unreachable.found);

    cout << "Test 3 passed: unreachable destination handled."
         << endl;

    // Test 4: Negative travel time must be rejected.
    bool negativeEdgeAdded =
        graph.addEdge("Karachi", "Islamabad", -2.0);

    assert(!negativeEdgeAdded);

    cout << "Test 4 passed: negative travel time rejected."
         << endl;

    // Test 5: A city that does not exist.
    RouteResult invalidCity =
        graph.dijkstra("Peshawar", "Lahore");

    assert(!invalidCity.found);

    cout << "Test 5 passed: invalid city handled." << endl;

    // Test 6: Duplicate city rejected.
    assert(!graph.addCity("Karachi"));

    cout << "Test 6 passed: duplicate city rejected." << endl;

    cout << "\nAll Dijkstra tests passed." << endl;

    return 0;
}