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

    // ghost cluster

    // a helper that only deals with one neighboring side
    void help_migrate_one_side(int rank_E, vector<Particle>& c_E, vector<Particle>& p_E);
    
    void clear_ghost() { ghost.clear(); }

    // random walk for particles and boundary checking + aggregation checking
    void random_walk();
    
    std::vector<Particle>::iterator aggregation_check(std::vector<Particle>::iterator it);
    
    void help_migration_dispatch( std::vector<Particle>& orig_vec , std::vector<Particle>& p_E,  std::vector<Particle>& p_W,  std::vector<Particle>& p_N,  std::vector<Particle>& p_S);

    void help_balance_migrate_one_side(int rank_E, std::vector<Particle>& c_E, std::vector<Particle>& p_E, std::vector<Particle>& recv_cluster, std::vector<Particle>& recv_particle);


    void add(std::vector<Particle>& vec, Vec2D pos){
        vec.push_back(Particle(pos));
    }

public:
    float rmax = 0;
    float mig_time = 0;

    vector<Particle> ghost;

    
    LocalDLA(vector<Particle> cluster, vector<Particle> particle, Vec2D corner1, Vec2D corner2): 
        cluster(cluster), particle(particle), upper(0,0), lower(0,0)
        {
        lower = min(corner1, corner2);
        upper = max(corner1, corner2);  
        }

    float local_rmax(){
        return rmax;
    }

    void set_domain(Vec2D corner1, Vec2D corner2){
        lower = min(corner1, corner2);
        upper = max(corner1, corner2); 
    }
    
    // update is the step that updates the each processor's domain's diffusion and cluster formation
    // update contains walk, particle attachment and communication(ghost exchange) stages   
    void update(int num_active_core, int rank);

    // balance is called only when the domain is redecomposed
    void balance(int num_active_core, int rank);

    // migrate is the communication process in which every processor have already get the new 
    void migrate(int num_active_core, int rank);
    void balance_migrate(int num_active_core, int rank, std::vector<Particle>& recv_cluster, std::vector<Particle>& recv_particle, bool first_pass);

    // spawn new particles to play with
    // spawn_rate is the probability of spawning at a fixed location given it's a feasible spawn region
    void spawn(float spawn_rate, int rmax, int spawn_rmin, int spawn_rmax);

    inline void add_ghost_cluster(Vec2D pos) { ghost.push_back(Particle(pos)); }

    inline void add_particle(Vec2D pos) { particle.push_back(Particle(pos)); }

    inline void add_cluster(Vec2D pos){ cluster.push_back(Particle(pos)); }

    void add_particles(vector<Particle>* spawn_p_lst);


    //
    string report_domain();
    string report_particle();
    string report_cluster();
    int local_area(){
        return get_area(upper, lower);
    }


};

int* help_pickel( std::vector<Particle>& c_lst, std::vector<Particle>& p_lst);



#endif //__LOCALDLA__







