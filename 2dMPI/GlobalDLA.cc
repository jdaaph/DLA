/*
    Define Simulation Class
*/

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "Particle.h"
#include "mpi.h"
#include <cmath>
#include <iostream>

using namespace std;


void GlobalDLA::init(int argc, char *argv[] ){

    MPI::Status status;

    MPI::Init(argc, argv)
    this -> p = MPI::COMM_WORLD.Get_size();
    this -> rank = MPI::COMM_WORLD.Get_rank();
    this -> rmas = 0;
// Master init:
    if (rank==0){
        Particle par (Vec2D(0, 0));
        this -> active = true;
    }
    activate_core();
    domain_decompose();
}


// helper function for processor grid <-> rank conversion
inline Vec2D rank2xy(int rank, int num_active_core){
    int l = floor(sqrt(num_active_core));
    // if (l != sqrt(num_active_core)){
    //     cout << "Error! not perfect square number of cores are allocated @.@?" << endl;
    //     return;
    // }
    return Vec2D( rank / l, rank % l);
}


// helper function for processor grid <-> rank conversion
inline Vec2D xy2rank(Vec2D xy, int num_active_core){
    int l = floor(sqrt(num_active_core));
    return (xy.x * l + xy.y);
}



// decide how many cores to use, this can reduce overhead at the beginning of the simulation
void GlobalDLA::activate_core(){

    // sync 

    if (num_active_core == p) return;

    // DEBUG FLAG
    num_active_core = 9;

    if (rank < num_active_core) active = true;


    //////////////////
    // sync GlobalDLA data, including `num_active_core, `rmax, will be used 
    // 
    //
    //////////////////

    // scatter





}



// helper function for helper
// lower_o is the x or y coordinate of lower corner of the central processor domain
// we will return the lower coordinate for another processor with delta 
static inline int help_lower(int del, int size, float alpha, int lower_o){
    int tmp;
    int lower;

    if (del == 0) return lower_o;
    lower = lower_o;

    if (del > 0){
        tmp = size;
        for (unsigned int i = 0; i < abs(del); ++i){
            lower += floor(tmp) + 1;
            tmp = floor(tmp * alpha);
        }
    }
    else{
        tmp = floor(size * alpha);
        for (unsigned int i = 0; i < abs(del); ++i){
            lower -= floor(tmp) + 1;
            tmp = floor(tmp * alpha);
        }
    }
    return lower;
}


// helper of helper, calculate processor domain size, (it's a square domain)
static inline int help_size(int del, int size, float alpha){
    int local_size = size;
    for (unsigned int i = 0; i < abs(del); ++i){
        local_size = floor(local_size * alpha);
    }
    return local_size;
}


// helper function for calculate the domain upper and lower corner
// l is the square root of current available processor count

struct Domain{
    Vec2D upper;
    Vec2D lower;
};

static int domain_calculator(float alpha, Vec2D xy, int l, int size_o, Vec2D lower_o){
    int del_x = xy.x - (l-1)/2;
    int del_y = xy.y - (l-1)/2;

    Vec2D lower(help_lower(del_x, size_o, alpha, lower_o), 
                help_lower(del_y, size_o, alpha, lower_o) 
               );

    Vec2D upper(0,0);

    int local_size = help_size(del, size_o, alpha);
    upper = lower + Vec2D(local_size, local_size);
    return Domain {upper, lower};
}


void GlobalDLA::domain_decompose(){
    // if there is global info stored at core-0, scatter and everyone add their right ones? or pass on one by one

    // the offset from the center of the processor grid
    int l = floor(sqrt(num_active_core));
    Vec2D xy = rank2xy(rank);

    // the +3 is because we have boundary check, some may fly away
    // size_o is the central domain's size

    if (rmax < 10000) rmax = 10000;

    int size_o;
    if (alpha == 1)
        size_o = floor((3 * rmax) / (1 + 2 * alpha * (1-pow(alpha, (p-1)/2.0)) / (1-alpha) ) ) + 3;
    else
        size_o = 6 * rmax / l;
    Vec2D lower_o (-size/2, -size/2);

    // for all node calculate their own domain and update their GlobalDLA settings and pass it on to LocalDLA
    if (active){
        Domain domain = domain_calculator(alpha, xy, l, size_o, lower_o)

        if (localDLA == NULL){
            // create a local DLA
            /////////////////////////////  DEBUG FLAG

            comm = Communicator();
            cluster = vector<Particle> ();
            particle = vector<Particle> ();

            LocalDLA = new LocalDLA(comm, cluster, particle, domain.upper, domain.lower);
        }
        else{
            localDLA -> set_domain(domain.upper, domain.lower)
            localDLA -> migrate();
        }
    }
}





void GlobalDLA::simulate(int timestep){
    for (unsigned int i = 0; i < 100; ++i){
        localDLA -> simulate();
    }
}


    // spawn new particles to play with
void GlobalDLA::spawn(int num_par){

}

    // load balance and redecomposition
void GlobalDLA::balance(){
    
}


// void helper_write(int rank, int p, ){

// }


void GlobalDLA::report(){
    cout << "Active Cores: " << num_active_core << endl;
    cout << "Rank: " << rank << "," << localDLA -> report_domain() << endl;

}


void GlobalDLA::test(){
    init();
    simulate();
    report();    
}
