#include "../include/Graph.h"
#include <iostream>
using namespace std;
CityGraph::EdgeNode::EdgeNode(CityNode* city, int time)
{
    destination = city;
    travelTime = time;
    next = nullptr;
}

CityGraph::CityNode::CityNode(const string& cityName)
{
    name = cityName;
    neighbors = nullptr;
    next = nullptr;
}
CityGraph::CityGraph()
{
    head = nullptr;
}
CityGraph::CityNode* CityGraph::findCity(
    const string& name) const
{
    CityNode* current = head;

    while (current != nullptr)
    {
        if (current->name == name)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}
CityGraph::EdgeNode* CityGraph::findEdge(
    CityNode* city, CityNode* destination) const
{
    if (city == nullptr || destination == nullptr)
    {
        return nullptr;
    }

    EdgeNode* current = city->neighbors;

    while (current != nullptr)
    {
        if (current->destination == destination)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}
bool CityGraph::addCity(const string& name)
{
    if (name.empty() || findCity(name) != nullptr)
    {
        return false;
    }

    CityNode* newCity = new CityNode(name);

    if (head == nullptr)
    {
        head = newCity;
        return true;
    }

    CityNode* current = head;

    while (current->next != nullptr)
    {
        current = current->next;
    }

    current->next = newCity;
    return true;
}
bool CityGraph::addEdge(
    const string& city1,
    const string& city2,
    int travelTime)
{
    if (travelTime <= 0 || city1 == city2)
    {
        return false;
    }

    CityNode* firstCity = findCity(city1);
    CityNode* secondCity = findCity(city2);

    if (firstCity == nullptr || secondCity == nullptr)
    {
        return false;
    }

    EdgeNode* firstEdge = findEdge(firstCity, secondCity);
    EdgeNode* secondEdge = findEdge(secondCity, firstCity);
        if (firstEdge != nullptr || secondEdge != nullptr)
    {
        if (firstEdge != nullptr)
        {
            firstEdge->travelTime = travelTime;
        }

        if (secondEdge != nullptr)
        {
            secondEdge->travelTime = travelTime;
        }

        return true;
    }
    EdgeNode* forward = new EdgeNode(secondCity, travelTime);

    EdgeNode* backward = nullptr;

    try
    {
        backward = new EdgeNode(firstCity, travelTime);
    }
    catch (...)
    {
        delete forward;
        throw;
    }

    forward->next = firstCity->neighbors;
    firstCity->neighbors = forward;

    backward->next = secondCity->neighbors;
    secondCity->neighbors = backward;

    return true;
}
void CityGraph::displayGraph() const
{
    CityNode* currentCity = head;

    while (currentCity != nullptr)
    {
        cout << currentCity->name << " -> ";

        EdgeNode* currentEdge = currentCity->neighbors;

        if (currentEdge == nullptr)
        {
            cout << "(no routes)";
        }

        while (currentEdge != nullptr)
        {
            cout << currentEdge->destination->name
                 << " (" << currentEdge->travelTime
                 << " hours)";

            if (currentEdge->next != nullptr)
            {
                cout << " -> ";
            }

            currentEdge = currentEdge->next;
        }

        cout << endl;
        currentCity = currentCity->next;
    }
}
CityGraph::~CityGraph()
{
    CityNode* currentCity = head;

    while (currentCity != nullptr)
    {
        EdgeNode* currentEdge = currentCity->neighbors;

        while (currentEdge != nullptr)
        {
            EdgeNode* tempEdge = currentEdge;
            currentEdge = currentEdge->next;
            delete tempEdge;
        }

        CityNode* tempCity = currentCity;
        currentCity = currentCity->next;
        delete tempCity;
    }
}