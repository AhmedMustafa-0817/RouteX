// Randomised checks: MinHeap against std::sort, and Dijkstra against
// Floyd-Warshall (brute-force all-pairs shortest paths).
#include "MinHeap.h"
#include "Graph.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

static int failures = 0;

static void fail(const string& message)
{
    if (failures < 10)
        cout << "FAIL: " << message << endl;
    ++failures;
}

static void testMinHeap(mt19937& rng)
{
    for (int trial = 0; trial < 200; ++trial)
    {
        MinHeap heap(1 + trial % 5); // tiny capacity forces grow()
        vector<double> reference;
        int pushes = static_cast<int>(rng() % 300);

        for (int i = 0; i < pushes; ++i)
        {
            double value = (rng() % 1000) / 10.0;
            heap.push(value, i);
            reference.push_back(value);

            // Interleave pops with pushes.
            if (rng() % 4 == 0)
            {
                sort(reference.begin(), reference.end());
                HeapNode node;
                if (!heap.pop(node) || node.distance != reference.front())
                    fail("interleaved pop must return the minimum");
                reference.erase(reference.begin());
            }
        }

        sort(reference.begin(), reference.end());
        for (double expected : reference)
        {
            HeapNode node;
            if (!heap.pop(node) || node.distance != expected)
                fail("pop order must match sorted order");
        }

        HeapNode node;
        if (!heap.isEmpty() || heap.pop(node))
            fail("drained heap must be empty and pop must return false");
    }

    cout << "MinHeap: 200 randomised trials done\n";
}

static void testDijkstra(mt19937& rng)
{
    const double INF = 1e18;

    for (int trial = 0; trial < 300; ++trial)
    {
        int n = 2 + static_cast<int>(rng() % 40);
        CityGraph graph;
        vector<string> names;

        for (int i = 0; i < n; ++i)
        {
            names.push_back("C" + to_string(i));
            graph.addCity(names.back());
        }

        vector<vector<double>> weight(n, vector<double>(n, INF));
        for (int i = 0; i < n; ++i)
            weight[i][i] = 0;

        int edges = static_cast<int>(rng() % (n * 3));
        for (int e = 0; e < edges; ++e)
        {
            int a = static_cast<int>(rng() % n);
            int b = static_cast<int>(rng() % n);
            int t = static_cast<int>(rng() % 20); // 0 is rejected by addEdge

            // Undirected; adding an existing edge overwrites its weight.
            if (graph.addEdge(names[a], names[b], t))
                weight[a][b] = weight[b][a] = t;
        }

        vector<vector<double>> dist = weight;
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (dist[i][k] + dist[k][j] < dist[i][j])
                        dist[i][j] = dist[i][k] + dist[k][j];

        for (int q = 0; q < 10; ++q)
        {
            int s = static_cast<int>(rng() % n);
            int t = static_cast<int>(rng() % n);
            RouteResult route = graph.dijkstra(names[s], names[t]);
            bool reachable = dist[s][t] < INF / 2;

            if (route.found != reachable)
            {
                fail("reachability mismatch " + names[s] + "->" + names[t]);
                continue;
            }

            if (!reachable)
                continue;

            if (fabs(route.totalHours - dist[s][t]) > 1e-9)
                fail("wrong distance " + names[s] + "->" + names[t]);

            if (route.pathLength < 1 ||
                route.path[0] != names[s] ||
                route.path[route.pathLength - 1] != names[t])
            {
                fail("path must start at source and end at destination");
                continue;
            }

            double sum = 0;
            for (int i = 0; i + 1 < route.pathLength; ++i)
            {
                int a = stoi(route.path[i].substr(1));
                int b = stoi(route.path[i + 1].substr(1));
                sum += weight[a][b];
            }

            if (fabs(sum - route.totalHours) > 1e-9)
                fail("path edges must add up to totalHours");
        }
    }

    cout << "Dijkstra: 300 random graphs checked against Floyd-Warshall\n";
}

int main()
{
    mt19937 rng(12345);

    testMinHeap(rng);
    testDijkstra(rng);

    cout << (failures == 0 ? "\nAll MinHeap/Dijkstra tests passed.\n"
                           : "\nMinHeap/Dijkstra tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}
