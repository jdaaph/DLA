#include <vector>;
#include "Vec2D.h"
#include "Constants.h"
#include "LocalDLA.h"
#include "Particle.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;


void LocalDLA::update(){

}




void LocalDLA::migrate(){
// determine boundary cluster (ghost) and free particles' their should-be domain rank

    // cluster section

    ///////// note that it may happen at the corner and two neighbors must be noticed!
    vector<Particle> c_W,c_E,c_N,c_S, ;

    int rank_W, rank_E, rank_N, rank_S;

    for (std::vector<Particle>::iterator it = cluster.begin() ; it != cluster.end(); ++it){
        if (it -> pos.x == upper.x) E.push_back(*it);
        if (it -> pos.x == lower.x) W.push_back(*it);
        if (it -> pos.y == upper.y) N.push_back(*it);
        if (it -> pos.y == lower.y) S.push_back(*it);
    }





// sort them in terms of rank

// do 6 neighbor comm, if they exists



}

void LocalDLA::spawn(float spawn_rate, int rmax, int spawn_rmin, int spawn_rmax){
    // spawn_rmax, spawn_rmin define spawn region, a ring region between those two radius
    // rmax is the actual

    // test if the domain has any feasible region

    // sanity check
    if (spawn_rmin <= rmax){
        std::cout << "Bug!!!!!!!!!!!!" << "Spawn particle may overlap with cluster" << endl;
        throw std::runtime_error("Spawn particle may overlap with cluster");
        return;
    }

    // generate that many random number
    vector<Particle>* spawn_p_lst = new vector<Particle>();

    int num_spawn = floor(spawn_rate * get_area(upper, lower));
    float tx, ty;
    int tmp_x, tmp_y;
    for (unsigned int i=0; i < num_spawn; ++i){
        // get random number in [0,1]
        tx = ((float) rand() / (RAND_MAX));
        ty = ((float) rand() / (RAND_MAX));
        tmp_x = floor ( tx * lower.x + (1 - tx) * upper.x);
        tmp_y = floor ( ty * lower.y + (1 - ty) * upper.y);

        // choose those in the feasible region and add them locally
        if ((get_r( tmp_x, tmp_y ) <= spawn_rmax) && (get_r( tmp_x, tmp_y ) >= spawn_rmin) )
            spawn_p_lst -> push_back( Vec2D(tmp_x, tmp_y) );
    }
    add_particles(spawn_p_lst);

    // The garbage collection must be done!!!
    delete spawn_p_lst;

}


void LocalDLA::add_particles(vector<Particle>* spawn_p_lst){
    particle.insert( particle.end(), spawn_p_lst -> begin(), spawn_p_lst -> end() );
}


string LocalDLA::report_domain(){
    ostringstream os;
    os << "Upper: (" << upper.x << ", " << upper.y << "), Lower: (" << lower.x << ", " << lower.y << ")" << endl;
    string s = os.str();
    return s;
}


string LocalDLA::report_particle(){
    ostringstream os;

    for (std::vector<Particle>::iterator it = particle.begin() ; it != particle.end(); ++it)
        os << " (" << (*it).pos.x << ", " << (*it).pos.y << ") ";

    os << endl;
    string s = os.str();
    return s;
}




















