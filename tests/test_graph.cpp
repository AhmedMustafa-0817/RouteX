#include "../include/Graph.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
int main()
{
    CityGraph graph;
    assert(graph.addCity("Karachi"));
    assert(graph.addCity("Hyderabad"));
    assert(graph.addCity("Sukkur"));
    assert(graph.addCity("Multan"));
    assert(graph.addCity("Lahore"));
    assert(graph.addCity("Islamabad"));
    assert(graph.addCity("Faisalabad"));
    assert(graph.addCity("Peshawar"));
    assert(!graph.addCity("Karachi"));
    assert(!graph.addCity(""));
    assert(graph.addEdge("Karachi", "Hyderabad", 4));
    assert(graph.addEdge("Hyderabad", "Sukkur", 5));
    assert(graph.addEdge("Sukkur", "Multan", 6));
    assert(graph.addEdge("Karachi", "Multan", 9));
    assert(graph.addEdge("Multan", "Lahore", 5));
    assert(graph.addEdge("Lahore", "Faisalabad", 2));
    assert(graph.addEdge("Lahore", "Islamabad", 4));
    assert(graph.addEdge("Islamabad", "Peshawar", 2));
    assert(!graph.addEdge("Karachi", "UnknownCity", 3));
    assert(!graph.addEdge("Karachi", "Karachi", 2));
    assert(!graph.addEdge("Karachi", "Hyderabad", 0));
    assert(!graph.addEdge("Karachi", "Hyderabad", -4));
    assert(graph.addEdge("Karachi", "Hyderabad", 7));
    ostringstream output;
    streambuf* oldOutput = cout.rdbuf(output.rdbuf());
    graph.displayGraph();
    cout.rdbuf(oldOutput);
    string result = output.str();
    const string cities[] = {
        "Karachi", "Hyderabad", "Sukkur", "Multan",
        "Lahore", "Islamabad", "Faisalabad", "Peshawar"
    };

    for (const string& city : cities)
    {
        string row = city + " -> ";
        size_t first = result.find(row);

        assert(first != string::npos);
        assert(result.find(row, first + row.length()) == string::npos);
    }
    assert(result.find("Hyderabad (7 hours)") != string::npos);
    assert(result.find("Karachi (7 hours)") != string::npos);
    assert(result.find("Sukkur (5 hours)") != string::npos);
    assert(result.find("Faisalabad (2 hours)") != string::npos);
    assert(result.find("Peshawar (2 hours)") != string::npos);
    cout << "All graph tests passed!" << endl;
    return 0;
}