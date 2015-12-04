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
    static const float alpha = ALPHA;
    int num_active_core = 0;
    bool active = false;
    LocalDLA* localDLA = nullptr;


    // must run init after create instance
    GlobalDLA()
    {
    }
    void init(int argc, char *argv[]);



    void simulate();

    // spawn new particles to play with
    void spawn(float spawn_rate, int spawn_rmin, int spawn_rmax);
    void activate_core();

    // load balance and redecomposition
    void domain_decompose();
    void report();
    void test();
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







