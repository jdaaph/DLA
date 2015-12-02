/*
    Define Simulation Class
*/

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "Particle.h"
#include "LocalDLA.h"
#include "mpi.h"

using namespace std;



/*
    Store MPI Comm Info
*/
class Communicator{

public:
    // p is total number of processors
    int p, rank;
    Communicator(int p, int rank) p(p), rank(rank);


};


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
    LocalDLA localDLA = NULL;

    Communicator comm;


    // must run init after create instance
    GlobalDLA();
    init(int argc, char *argv[]);



    simulate(int timestep);

    // spawn new particles to play with
    spawn(int num_par);
    activate_core();

    // load balance and redecomposition
    domain_decompose();
    report();
    test();
};













