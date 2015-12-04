#ifndef __LOCALDLA__
#define __LOCALDLA__

#include <vector>;
#include "Constants.h"
#include "Vec2D.h"
#include "Particle.h"


using namespace std;


/*
    Store local runtime information
*/
class LocalDLA{

private:
    // cluster are list of fixed particles that have already formed cluster
    // particle are list of free particles
    vector<Particle> cluster;
    vector<Particle> particle;

    // box config
    Vec2D lower, upper;
    int ghost_region = GHOST_REGION;

    // ghost particles
    vector<Particle> g_W, g_E, g_N, g_S;


public:
    LocalDLA(vector<Particle> cluster, vector<Particle> particle, Vec2D corner1, Vec2D corner2): 
        cluster(cluster), particle(particle), upper(0,0), lower(0,0)
        {
        lower = min(corner1, corner2);
        upper = max(corner1, corner2);  
        }

    void set_domain(Vec2D corner1, Vec2D corner2){
        lower = min(corner1, corner2);
        upper = max(corner1, corner2); 
    }
    
    // update is the step that updates the each processor's domain's diffusion and cluster formation
    // update contains walk, particle attachment and communication(ghost exchange) stages   
    void update();

    // migrate is the communication process in which every processor have already get the new 
    void migrate();

    // spawn new particles to play with
    // spawn_rate is the probability of spawning at a fixed location given it's a feasible spawn region
    void spawn(float spawn_rate, int rmax, int spawn_rmin, int spawn_rmax);


    void add_particles(vector<Particle>* spawn_p_lst);

    //
    string report_domain();
    string report_particle();
    string report_cluster();
    int local_area(){
        return get_area(upper, lower);
    }

};




#endif //__LOCALDLA__







