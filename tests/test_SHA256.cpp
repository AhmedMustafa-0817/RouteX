#include "SHA256.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
    string emptyHash = SHA256::hash("");
    string abcHash = SHA256::hash("abc");

    cout << "Testing SHA-256...\n\n";

    cout << "Empty string hash:\n" << emptyHash << "\n\n";
    cout << "ABC hash:\n" << abcHash << "\n\n";

    string expectedEmpty =
        "e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855";

    string expectedABC =
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad";

    if (emptyHash != expectedEmpty)
    {
        cout << "FAIL: Empty string test failed.\n";
        return 1;
    }

    if (abcHash != expectedABC)
    {
        cout << "FAIL: ABC test failed.\n";
        return 1;
    }

    cout << "PASS: All SHA-256 tests passed.\n";

    return 0;
}