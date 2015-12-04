#include <vector>;
#include "Vec2D.h"
#include "Constants.h"
#include "LocalDLA.h"
#include "Particle.h"
#include "GlobalDLA.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;


void LocalDLA::update(){

}



int* help_pickel( std::vector<Particle>& c_lst, std::vector<Particle>& p_lst){
    // pickel serialize data, starts with one integer = c_lst.size(), and then cluster's x1,y1, x2,y2 .... then particles's x1,y1,x2,y2 .....
    int total_size = c_lst.size() * 2 + p_lst.size() * 2 + 1;
    



    // cout << total_size << endl;




    int* pickel = new int[total_size] ();
    pickel[0] = c_lst.size();

    unsigned int i = 1;
    for (std::vector<Particle>::iterator it = c_lst.begin() ; it != c_lst.end(); ++it){
        pickel[i]   = it -> pos.x;
        pickel[i+1] = it -> pos.y;
        i += 2;
    }

    for (std::vector<Particle>::iterator it = p_lst.begin() ; it != p_lst.end(); ++it){
        pickel[i]   = it -> pos.x;
        pickel[i+1] = it -> pos.y;
        i += 2;
    }
    return pickel;
}


// timestep level communication
void LocalDLA::migrate(int num_active_core, int rank){
// determine boundary cluster (ghost) and free particles' their should-be domain rank

    // cluster section

    ///////// note that it may happen at the corner and two neighbors must be noticed!
    vector<Particle> c_W, c_E, c_N, c_S;
    vector<Particle> p_W, p_E, p_N, p_S;

    int rank_W, rank_E, rank_N, rank_S;

    // THIS can be optimized, we don't have to check for ghost for all clusters, we can only check the newly formed clusters, but I guess this ain't the bottleneck anyway

    // cluster => ghost 
    for (std::vector<Particle>::iterator it = cluster.begin() ; it != cluster.end(); ++it){
        if (it -> pos.x == upper.x) c_E.push_back(*it);
        if (it -> pos.x == lower.x) c_W.push_back(*it);
        if (it -> pos.y == upper.y) c_N.push_back(*it);
        if (it -> pos.y == lower.y) c_S.push_back(*it);
    }

    // particle => migrate
    // remember to delete the particles that migrate

    std::vector<Particle>::iterator it = particle.begin() ;
    while ( it != particle.end()){
        if (it -> pos.x > upper.x) {
            p_E.push_back(*it);
            it = particle.erase(it);
            continue;
        }
        if (it -> pos.x < lower.x) {
            p_W.push_back(*it);
            it = particle.erase(it);
            continue;
        }
        if (it -> pos.y > upper.y){ 
            p_N.push_back(*it);
            it = particle.erase(it);
            continue;
        }
        if (it -> pos.y < lower.y) {
            p_S.push_back(*it);
            it = particle.erase(it);
            continue;
        }
        // if any branch if triggered, this particle would be moved and iterator would be updated
        it ++;
    }

    // Now check for neighbor existence and get rank
    Vec2D xy_current = rank2xy(rank, num_active_core);
    rank_E = xy2rank( xy_current + Vec2D(+1, 0), num_active_core);
    rank_W = xy2rank( xy_current + Vec2D(-1, 0), num_active_core);
    rank_N = xy2rank( xy_current + Vec2D(0, +1), num_active_core);
    rank_S = xy2rank( xy_current + Vec2D(0, -1), num_active_core);

    // cout << rank_E << " " << rank_W << " " << rank_N << " " << rank_S << endl;
    // if (rank == 1){
    //     cout << p_E .front(). pos.x << ", " << p_E .front().pos.y << endl;
    // }

    help_migrate_one_side (rank_E, c_E, p_E);
    help_migrate_one_side (rank_W, c_W, p_W);
    help_migrate_one_side (rank_N, c_N, p_N);
    help_migrate_one_side (rank_S, c_S, p_S);

}


// helper function that sends and receives and decode the information from one neighboring (I'm writing in terms of East side, 'E')
void LocalDLA::help_migrate_one_side(int rank_E, vector<Particle>& c_E, vector<Particle>& p_E){

    // if neighbor exist, prepare / probe the messages and send / recv
    MPI::Status status;
    if (rank_E != -1) {
        // prepare
        int* msg_2E = help_pickel(c_E, p_E);
        
        ////////// DEBUG FLAG
        // cout << msg_2E[1] << endl;



        // This is wrong 
        int size_2E = c_E.size() * 2 + p_E.size() * 2 + 1;
        MPI::COMM_WORLD.Send(msg_2E, size_2E, MPI::INT, rank_E, 0);


/////////////////// DEBIG FLAG !        
        // cout << size_2E << endl;





        MPI::COMM_WORLD.Probe(rank_E, 0, status);

        int size_fE = status.Get_count(MPI::INT);


/////////////////// DEBIG FLAG !
        // cout << size_fE << endl;

        /////// if it's an empty message don't do anything
        if (size_fE){
            // cout << "nonzero size!" << endl;

            int* buff_E = new int[size_fE] ();
            MPI::COMM_WORLD.Recv(buff_E, size_fE, MPI::INT, rank_E, 0);

            // decode the buffer
            int num_c_E = buff_E[0];
            for (unsigned int i=1; i < num_c_E * 2; i += 2){
                add_ghost_cluster( Vec2D(buff_E[i], buff_E[i + 1]));
                cout << "!!!! Recv biu !!!!" << endl;

            }
            for (unsigned int i= 1 + num_c_E * 2; i < size_fE - 1; i += 2){
                add_particle( Vec2D(buff_E[i], buff_E[i + 1]) );
                cout << "!!!! Recv biu !!!!" << endl;
            }
                
            
            delete buff_E;
        }

    ////////// This might not be safe?
    delete[] msg_2E;
    }
}



void LocalDLA::balance(){

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




















