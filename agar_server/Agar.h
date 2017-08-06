#ifndef Agar_h
#define Agar_h

#include <string>

using std::string;

enum NodeType
{
    VIRUS,
    FOOD,
    PLAYER,
};

struct Node
{
    uint32_t id;
    float x;
    float y;
    float radius;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    string name;
    NodeType type;
};




#endif /* Agar_h */
