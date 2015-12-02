#include "AABB.h"
#include "Vec3D.h"
#include "Particle.h"
#include "OctreeVisitor.h"

#include <vector>

using namespace std;


PointOctree::PointOctree(PointOctree* parent, Vec3D offset, float halfsize)
    :   parent(parent), halfsize(halfsize), size(2.0 * halfsize), num_children(0),
{
    if (parent != NULL){
        depth = parent -> depth + 1;
        minNodeSize = parent -> minNodeSize;
    }
}


void PointOctree::applyVisitor(OctreeVisitor* visitor){
    visitor->visitNode(this);
    if (num_children > 0){
        for(vector<PointOctree*>::iterator it = children.begin(); it != children.end(); ++it) {
            // !!!!!!!
            // Does we need a sanity check? I'll put one here
            assert(it);
            it -> applyVisitor(visitor);
        }
    }
}












