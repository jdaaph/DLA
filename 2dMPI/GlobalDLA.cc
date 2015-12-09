/*
    Define Simulation Class
*/

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "Particle.h"
#include "GlobalDLA.h"
#include "mpi.h"
#include <cmath>
#include <iostream>
#include <string>
#include <cstdlib> 

using namespace std;


void GlobalDLA::init(int argc, char *argv[] ){

    MPI::Status status;

    MPI::Init(argc, argv);
    this -> p = MPI::COMM_WORLD.Get_size();
    this -> rank = MPI::COMM_WORLD.Get_rank();
    this -> rmax = 0;
// Master init:
    if (rank==0){
        this -> active = true;
    }
    activate_core();
    domain_decompose();
}



// decide how many cores to use, this can reduce overhead at the beginning of the simulation
void GlobalDLA::activate_core(){

    // sync 

    if (num_active_core == p) return;

    // DEBUG FLAG
    num_active_core = MPI::COMM_WORLD.Get_size();

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



// also a helper
static Domain domain_calculator(float alpha, Vec2D xy, int l, int size_o, Vec2D lower_o){
    int del_x = xy.x - (l-1)/2;
    int del_y = xy.y - (l-1)/2;

    int lower_x = help_lower(del_x, size_o, alpha, lower_o.x);
    int lower_y = help_lower(del_y, size_o, alpha, lower_o.y);

    Vec2D lower( lower_x, lower_y);

    Vec2D upper(0,0);

    int local_size_x = help_size(del_x, size_o, alpha);
    int local_size_y = help_size(del_y, size_o, alpha);

    upper = lower + Vec2D(local_size_x, local_size_y);
    Domain domain = {upper, lower};
    return domain;
}


void GlobalDLA::domain_decompose(){


    // the offset from the center of the processor grid
    int l = floor(sqrt(num_active_core));
    Vec2D xy = rank2xy(rank, num_active_core);

    // the +3 is because we have boundary check, some may fly away
    // size_o is the central domain's size

    /////////////////// !!!!!!!!!!!!!!!!!!!!! sync the rmax between cores



    sync_rmax();

    float domain_rmax = fmax(30.0, rmax);

    int size_o;
    if (alpha != 1)
        size_o = floor((2 * BOUND_FACTOR * domain_rmax) / (1 + 2 * alpha * (1-pow(alpha, (l-1)/2.0)) / (1-alpha) ) ) + 3;
    else
        size_o = 2 * BOUND_FACTOR * domain_rmax / l;

    Vec2D lower_o (-size_o/2, -size_o/2);

    // for all node calculate their own domain and update their GlobalDLA settings and pass it on to LocalDLA
    if (active){
        Domain domain = domain_calculator(alpha, xy, l, size_o, lower_o);

        if (localDLA == nullptr){
            // create a local DLA
            vector<Particle> cluster = vector<Particle> ();
            vector<Particle> particle = vector<Particle> ();

            localDLA = new LocalDLA(cluster, particle, domain.upper, domain.lower);
        }
        else{
            localDLA -> set_domain(domain.upper, domain.lower);
            balance();
            }
    }
}


// The feasible region is a ring-like structure, this helper function determines if a domain has any feasible region

// bool help_has_feasible(int spawn_rmin, int spawn_rmax){
//     return true;
// }




// load balance and redecomposition wrapper
void GlobalDLA::balance(){
    localDLA -> balance(num_active_core, rank);
}


// void helper_write(int rank, int p, ){

// }


// wrapper function for spawning, ensure rmax is all correctly calculated
void GlobalDLA::spawn(float spawn_rate){
    // update the rmax
    sync_rmax();
    localDLA -> spawn(spawn_rate, rmax, rmax + 4, rmax + 14);
}


void GlobalDLA::report(){
    // only active core report
    if ( !active ) return; 
    cout << "Active Cores: " << num_active_core << endl;
    // string report = localDLA -> report_domain();
    // string report2 = localDLA -> report_particle();
    // cout << "Rank: " << rank << "," << report << " " << report2 << endl;
    

    string report = localDLA -> report_domain();
    string report2 = localDLA -> report_cluster();
    cout << "Rank: " << rank << "," << report << " " << report2 << endl;

    cout << "Particle: " << localDLA -> report_particle() << endl;
}


void GlobalDLA::report_collective_cluster(){
    string report = localDLA -> report_cluster();
    string report2 = localDLA -> report_particle();

    // cout << "======== cluster ======== rank: " << rank << endl;
    // cout << report << endl;
    // cout << "======== particle ======== rank: " << rank << endl;
    // cout << report2 << endl;

    cout << report ;


}



void GlobalDLA::simulate(int timestep){
    // only active cores simulate
    if (!active) return;



    for (unsigned int i = 0; i < timestep; ++i){
        localDLA -> update(num_active_core, rank);
        if (i % 300 == 0)
            spawn(0.01);
        if (i % 2000 == 0)
            domain_decompose();

        // MPI::COMM_WORLD.Barrier();
        // cout << "=========" << endl;
        // report();
    }
}



// add at the central node the first cluster seed
void GlobalDLA::add_seed_cluster(){
    int l = floor(sqrt(p));
    if (rank2xy(rank, num_active_core).x == (l-1)/2 &&  rank2xy(rank, num_active_core).y == (l-1)/2 ){
        localDLA -> add_cluster(Vec2D(0,0));
    }
}


void GlobalDLA::add_seed_cluster(Vec2D pos){
    int l = floor(sqrt(p));
    if (rank2xy(rank, num_active_core).x == (l-1)/2 &&  rank2xy(rank, num_active_core).y == (l-1)/2 ){
        localDLA -> add_cluster(pos);
    }
}




void GlobalDLA::test(){

    add_seed_cluster();
    double time_i, time_f;

    if (rank == 0)
        time_i = MPI::Wtime();

    MPI::COMM_WORLD.Barrier();

    simulate(TIME_STEP);

    MPI::COMM_WORLD.Barrier();
    if (rank == 0){
        time_f = MPI::Wtime();
        cout << "=============\n" << "time = " << time_f - time_i << endl;
    }


    report_collective_cluster();

    MPI::COMM_WORLD.Barrier();

    cout << "BIUBIUBIU" << endl;
    cout << "mig_time: " << localDLA -> mig_time << endl;

    // if (rank == 4 || rank == 0)
    //     report();

    finalize();    
}


// for debug the migration process
void GlobalDLA::test_migration(){
    add_seed_cluster();

}


// for debug the balance (domain redecompose) process
void GlobalDLA::test_balance(){
    add_seed_cluster(Vec2D(-66,-67));

    balance();

    cout << localDLA -> report_domain() << endl;

    MPI::COMM_WORLD.Barrier();

    cout << "Rank: " << rank << "," << localDLA -> report_cluster() << endl;
    // report_collective_cluster();

    MPI::COMM_WORLD.Barrier();
    finalize();    
}

















