/*
    Define Simulation Class
*/
#ifndef __GLOBALDLA__
#define __GLOBALDLA__

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "Particle.h"
#include "LocalDLA.h"
#include "mpi.h"
#include <string>
#include <algorithm>

using namespace std;



/*
    Store MPI Comm Info
*/
// class Communicator{

// public:
//     // p is total number of processors
//     int p, rank;
//     Communicator(int p, int rank) p(p), rank(rank);


// };


/*
    Store global runtime information
*/
class GlobalDLA{
public:
    // p is total number of processors
    int p, rank;

    // rmax is only meaningful for core-0
    float rmax = 0;
    float alpha = ALPHA;
    int num_active_core = 0;
    bool active = false;
    LocalDLA* localDLA = nullptr;


    // must run init after create instance
    GlobalDLA()
    {
    }
    void init(int argc, char *argv[]);

    void set_rmax(float value){
        if (localDLA != nullptr){
            if (value > max(rmax, localDLA -> rmax)){
                rmax = value;
                localDLA -> rmax = value;
            }
        }
        else
            if (value > rmax) rmax = value;
    }

    void sync_rmax(){
        if (localDLA != nullptr)
            rmax = max(rmax, localDLA -> rmax);
        float tmp_rmax;
        MPI::COMM_WORLD.Allreduce(&rmax, &tmp_rmax, 1, MPI::FLOAT, MPI::MAX);
        set_rmax(tmp_rmax);
    }

    void simulate(int timestep);

    // spawn new particles to play with
    void spawn(float spawn_rate);
    void add_seed_cluster();
    void add_seed_cluster(Vec2D pos);
    void activate_core();

    // load balance and redecomposition
    void domain_decompose();
    void report();
    void report_collective_cluster();


    void test();
    void test_migration();
    void test_balance();

    void balance();
    void finalize(){
        MPI::Finalize();
    }
};


struct Domain{
    Vec2D upper;
    Vec2D lower;
};


static inline int help_lower(int del, int size, float alpha, int lower_o);
static inline int help_size(int del, int size, float alpha);
static Domain domain_calculator(float alpha, Vec2D xy, int l, int size_o, Vec2D lower_o);




#endif //__GLOBALDLA__







