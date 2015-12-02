#include "AABB.h"
#include "Vec3D.h"
#include "Particle.h"
#include "OctreeVisitor.h"
#include "Constants.h"

#include <vector>

using namespace std;

class PointOctree(AABB){
protected:
    PointOctree* parent;
    char num_children;
    vec<PointOctree*> children;

    // offset is the position of the center of the box
    Vec3D offset;
    
    // Here we assume each side has same length, size is this common side length
    // halfsize is half of it.
    float size;
    float halfsize;

    // System will stop divide further a space if new node will be smaller than this limit
    float minNodeSize = MIN_NODE_SIZE;

    vector<Particle> par_lst;

private:
    int depth = 0;

public:
    PointOctree(PointOctree* parent, Vec3D offset, float halfsize);

    

    void applyVisitor(OctreeVisitor* visitor);
};























