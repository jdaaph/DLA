#include "AABB.h"
#include "Vec3D.h"
#include "Particle.h"

#include <vector>

using namespace std;

class PointOctree(AABB){
protected:
	PointOctree parent;
	PointOctree[] children;
	char num_children;
	vector<Particle> par_lst;

private:
	int depth = 0;

public:
	PointOctree(PointOctree _parent, )

};























