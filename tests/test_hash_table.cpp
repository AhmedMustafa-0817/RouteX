#include "../backend/include/HashTable.h"
#include <iostream>

using namespace std;

int main()
{
    cout << "TEST STARTED" << endl;
    HashTable table(10);

    Shipment s1(103, "Lahore", "Karachi", "Electronics", 5.5, "Pending");
    Shipment s2(113, "Islamabad", "Lahore", "Documents", 2.0, "Pending");
    Shipment s3(127, "Karachi", "Islamabad", "Clothing", 4.0, "Pending");
    Shipment s4(123, "Lahore", "Islamabad", "Books", 3.0, "Pending");

    cout << "Inserting shipments..." << endl;

    cout << "Insert 103: " << (table.insert(s1) ? "Success" : "Failed") << endl;
    cout << "Insert 113: " << (table.insert(s2) ? "Success" : "Failed") << endl;
    cout << "Insert 127: " << (table.insert(s3) ? "Success" : "Failed") << endl;
    cout << "Insert 123: " << (table.insert(s4) ? "Success" : "Failed") << endl;

    cout << endl;

    cout << "Hash Table:" << endl;
    table.display();

    cout << endl;

    Shipment *result = table.search(113);

    if (result != nullptr)
    {
        cout << "Shipment 113 found." << endl;
        result->display();
    }
    else
    {
        cout << "Shipment 113 not found." << endl;
    }

    cout << endl;

    result = table.search(999);

    if (result != nullptr)
    {
        cout << "Shipment 999 found." << endl;
    }
    else
    {
        cout << "Shipment 999 not found." << endl;
    }

    cout << endl;

    cout << "Duplicate insert 103: "
         << (table.insert(s1) ? "Success" : "Rejected")
         << endl;

    cout << endl;

    cout << "Removing shipment 113: "
         << (table.remove(113) ? "Success" : "Failed")
         << endl;

    cout << endl;

    cout << "Hash Table after removal:" << endl;
    table.display();

    return 0;
}