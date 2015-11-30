#include "Vec3D.h"


// Constructor from two opposite corners of the box
AABB::AABB(Vec3D a, Vec3D b){
    Vec3D lower = Vec3D.min(a, b);
    Vec3D upper = Vec3D.max(a, b);
    return 
}