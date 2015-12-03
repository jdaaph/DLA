/*
    Particle class is the interface that the Octree datastructure would deal with, it contains position and other information (to be added)
*/

#ifndef __PARTICLE__
#define __PARTICLE__

#include "Vec2D.h"


class Particle{
public:
    // position
    Vec2D pos;

    Particle(Vec2D pos): pos(pos)
    {
    }
};




#endif //__PARTICLE__






