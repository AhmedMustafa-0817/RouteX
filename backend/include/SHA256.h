#ifndef SHA256_H
#define SHA256_H

#include <string>
using namespace std;

class SHA256
{
public:
    static string hash(const string& input);
};

#endif