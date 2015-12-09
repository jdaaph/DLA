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


void LocalDLA::update(int num_active_core, int rank){
    random_walk();
    clear_ghost();
    migrate( num_active_core, rank);

}


// random walk include walk + aggregation checking (boundary checking is done by the migrate)
///////// THIS IS A POTENTIAL OPTIMIZATION FLAG

void LocalDLA::random_walk(){
    int direction;

    /// use the while way to write, because iter may change because boundary check
    std::vector<Particle>::iterator it = particle.begin();
    while (it != particle.end()){
        direction = rand() % 4;
        switch (direction){
            case (0):
            {
                it -> pos += Vec2D (-1, 0);
                break;
            }
            case (1):
            {
                it -> pos += Vec2D (+1, 0);
                break;
            }
            case (2):
            {
                it -> pos += Vec2D (0, -1);
                break;
            }
            case (3):
            {
                it -> pos += Vec2D (0, +1);
                break;
            }
        }
        // aggregation_check will automatically change the iterator to the next
        it = aggregation_check( it );

    }
}


std::vector<Particle>::iterator LocalDLA::aggregation_check(std::vector<Particle>::iterator it){
    // iterate over the cluster to check if the cluster is in the particle's spherical region (), if this happens, this particle is removed from particle to be added as cluster
    Vec2D p_pos(it -> pos);

    // out of bound check 
    if (get_r(p_pos) > BOUND_FACTOR * fmax(rmax, 100.0))
        return (particle.erase(it));

    // aggregation check
    for (std::vector<Particle>::iterator c_it = cluster.begin() ; c_it != cluster.end(); ++ c_it){
        // only direct neighbor can trigger it 
        if (get_distance2(c_it -> pos, p_pos) <= 1) {

            // cout << "rmax = " << rmax << ", newly_attach: " << p_pos.x << endl;
            // cout << p_pos.y << endl;
            cluster.push_back(Particle(p_pos));
            // update the local rmax if it is farthest from origin
            if ( get_r(p_pos) > rmax )  rmax = get_r(p_pos);
            return (particle.erase(it));
        }
    }


    ////////////////////// Don't forget the ghost check
    for (std::vector<Particle>::iterator g_it = ghost.begin() ; g_it != ghost.end(); ++ g_it){
        if (get_distance2(g_it -> pos, p_pos) <= 1) {
            cluster.push_back(Particle(p_pos));
            // update the local rmax if it is farthest from origin
            if ( get_r(p_pos) > rmax )  rmax = get_r(p_pos);
            return (particle.erase(it));
        }
    }

    // if all check are done!
    ++ it;
    return it;
}



int* help_pickel( std::vector<Particle>& c_lst, std::vector<Particle>& p_lst){
    // pickel serialize data, starts with one integer = c_lst.size(), and then cluster's x1,y1, x2,y2 .... then particles's x1,y1,x2,y2 .....
    int total_size = c_lst.size() * 2 + p_lst.size() * 2 + 1;
    
    // if (total_size == 1) total_size = 0;

////////// POTENTIAL OPTIMIZATION FLAG
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


    // cluster => push to neighbors to update ghost 
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

    double time_i = MPI::Wtime();
    help_migrate_one_side (rank_E, c_E, p_E);
    help_migrate_one_side (rank_W, c_W, p_W);
    help_migrate_one_side (rank_N, c_N, p_N);
    help_migrate_one_side (rank_S, c_S, p_S);
    double time_f = MPI::Wtime();
    mig_time += time_f - time_i;
}



// will find all out of domain particles/clusters from orig_vec and push it to mig_vec
void LocalDLA::help_migration_dispatch( std::vector<Particle>& orig_vec , std::vector<Particle>& p_E,  std::vector<Particle>& p_W,  std::vector<Particle>& p_N,  std::vector<Particle>& p_S){
    std::vector<Particle>::iterator it = orig_vec.begin() ;
    while ( it != orig_vec.end()){
        if (it -> pos.x > upper.x) {
            p_E.push_back(*it);
            it = orig_vec.erase(it);
            continue;
        }
        if (it -> pos.x < lower.x) {
            p_W.push_back(*it);
            it = orig_vec.erase(it);
            continue;
        }
        if (it -> pos.y > upper.y){ 
            p_N.push_back(*it);
            it = orig_vec.erase(it);
            continue;
        }
        if (it -> pos.y < lower.y) {
            p_S.push_back(*it);
            it = orig_vec.erase(it);
            continue;
        }
        // if any branch if triggered, this particle would be moved and iterator would be updated
        it ++;
    }
}





// re-decompose communication

// do this l + 1 times to ensure correctness
// the send dispatcher would also go through the lately received
// cluster and particle would be passed and after received stored in a special place

// potential change is use all scatter, but this would not be run many times so it doesn't matter



// last step!!! call migrate to get the right ghost cluster

void LocalDLA::balance_migrate(int num_active_core, int rank, std::vector<Particle>& recv_cluster, std::vector<Particle>& recv_particle, bool first_pass){
// determine boundary cluster (ghost) and free particles' their should-be domain rank

// if this is the first migration "first_pass == true", we need to transfer all 

    // cluster section
    ///////// note that it may happen at the corner and two neighbors must be noticed!
    vector<Particle> c_W, c_E, c_N, c_S;
    vector<Particle> p_W, p_E, p_N, p_S;

    int rank_W, rank_E, rank_N, rank_S;

    // THIS can be optimized, we don't have to check for ghost for all clusters, we can only check the newly formed clusters, but I guess this ain't the bottleneck anyway


    // cluster => push to neighbors to update ghost
    if (first_pass) {
        help_migration_dispatch(cluster, c_E, c_W, c_N, c_S);
        help_migration_dispatch(particle, p_E, p_W, p_N, p_S);
    }
    help_migration_dispatch(recv_cluster, c_E, c_W, c_N, c_S);
    help_migration_dispatch(recv_particle, p_E, p_W, p_N, p_S);

    // Now check for neighbor existence and get rank
    Vec2D xy_current = rank2xy(rank, num_active_core);
    rank_E = xy2rank( xy_current + Vec2D(+1, 0), num_active_core);
    rank_W = xy2rank( xy_current + Vec2D(-1, 0), num_active_core);
    rank_N = xy2rank( xy_current + Vec2D(0, +1), num_active_core);
    rank_S = xy2rank( xy_current + Vec2D(0, -1), num_active_core);

    help_balance_migrate_one_side (rank_E, c_E, p_E, recv_cluster, recv_particle);
    help_balance_migrate_one_side (rank_W, c_W, p_W, recv_cluster, recv_particle);
    help_balance_migrate_one_side (rank_N, c_N, p_N, recv_cluster, recv_particle);
    help_balance_migrate_one_side (rank_S, c_S, p_S, recv_cluster, recv_particle);
}



// helper function that sends and receives and decode the information from one neighboring (I'm writing in terms of East side, 'E')
void LocalDLA::help_balance_migrate_one_side(int rank_E, std::vector<Particle>& c_E, std::vector<Particle>& p_E, std::vector<Particle>& recv_cluster, std::vector<Particle>& recv_particle){

    // if neighbor exist, prepare / probe the messages and send / recv
    MPI::Status status;
        if (rank_E != -1) {
            // prepare
            int* msg_2E = help_pickel(c_E, p_E);

            // This is wrong 
            int size_2E = c_E.size() * 2 + p_E.size() * 2 + 1;
            MPI::COMM_WORLD.Send(msg_2E, size_2E, MPI::INT, rank_E, 0);
            MPI::COMM_WORLD.Probe(rank_E, 0, status);
            int size_fE = status.Get_count(MPI::INT);

            /////// if it's an empty message don't do anything
            if (size_fE){
                int* buff_E = new int[size_fE] ();
                MPI::COMM_WORLD.Recv(buff_E, size_fE, MPI::INT, rank_E, 0);

                // decode the buffer
                int num_c_E = buff_E[0];
                for (unsigned int i=1; i < num_c_E * 2; i += 2)
                    add(recv_cluster, Vec2D(buff_E[i], buff_E[i + 1]));

                for (unsigned int i= 1 + num_c_E * 2; i < size_fE - 1; i += 2)
                    add( recv_particle, Vec2D(buff_E[i], buff_E[i + 1]) );
                
                delete buff_E;
            }

        ////////// This might not be safe?
        delete[] msg_2E;
    }
}





// helper function that sends and receives and decode the information from one neighboring (I'm writing in terms of East side, 'E')
void LocalDLA::help_migrate_one_side(int rank_E, vector<Particle>& c_E, vector<Particle>& p_E){

    // if neighbor exist, prepare / probe the messages and send / recv
    MPI::Status status;
    if (rank_E != -1) {
        // prepare
        int* msg_2E = help_pickel(c_E, p_E);

        // This is wrong 
        int size_2E = c_E.size() * 2 + p_E.size() * 2 + 1;
        MPI::COMM_WORLD.Send(msg_2E, size_2E, MPI::INT, rank_E, 0);
        MPI::COMM_WORLD.Probe(rank_E, 0, status);
        int size_fE = status.Get_count(MPI::INT);

        /////// if it's an empty message don't do anything
        if (size_fE){
            int* buff_E = new int[size_fE] ();
            MPI::COMM_WORLD.Recv(buff_E, size_fE, MPI::INT, rank_E, 0);

            // decode the buffer
            int num_c_E = buff_E[0];
            for (unsigned int i=1; i < num_c_E * 2; i += 2){
                add_ghost_cluster( Vec2D(buff_E[i], buff_E[i + 1]));
            }
            for (unsigned int i= 1 + num_c_E * 2; i < size_fE - 1; i += 2){
                add_particle( Vec2D(buff_E[i], buff_E[i + 1]) );
            }
            
            delete buff_E;
        }

    ////////// This might not be safe?
    delete[] msg_2E;
    }
}



void LocalDLA::balance(int num_active_core, int rank){
    // receive buffer only for domain redecomposition
    vector<Particle> recv_cluster;
    vector<Particle> recv_particle;

    // clear ghost
    clear_ghost();
    int l = floor(sqrt(num_active_core));

    // do communication
    for (unsigned int i = 0; i < l; ++i)
        balance_migrate(num_active_core, rank, recv_cluster, recv_particle, (i==0) );

    // merge recv into right place
    particle.insert(particle.end(), recv_particle.begin(), recv_particle.end());
    cluster.insert(cluster.end(), recv_cluster.begin(), recv_cluster.end());

    // reconstruct ghost 

}




void LocalDLA::spawn(float spawn_rate, int rmax, int spawn_rmin, int spawn_rmax){
    // spawn_rmax, spawn_rmin define spawn region, a ring region between those two radius
    // rmax is the actual

    // test if the domain has any feasible region
    srand(RANDOM_SEED_SPAWN);

    // sanity check
    if (spawn_rmin <= rmax){
        std::cout << "Bug!!!!!!!!!!!!" << "Spawn particle may overlap with cluster" << endl;
        throw std::runtime_error("Spawn particle may overlap with cluster");
        return;
    }

    // generate that many random number
    vector<Particle>* spawn_p_lst = new vector<Particle>();

    int num_spawn = ceil(spawn_rate * get_area(upper, lower));
    float tx, ty;
    int tmp_x, tmp_y;
    int actual_num_spawn = 0;


    for (unsigned int i=0; i < num_spawn; ++i){

        // get random number in [0,1]
        tx = ((float) rand() / (RAND_MAX));
        ty = ((float) rand() / (RAND_MAX));
        tmp_x = floor ( tx * lower.x + (1 - tx) * upper.x);
        tmp_y = floor ( ty * lower.y + (1 - ty) * upper.y);

        // choose those in the feasible region and add them locally
        if ((get_r( tmp_x, tmp_y ) <= spawn_rmax) && (get_r( tmp_x, tmp_y ) >= spawn_rmin) ){
            spawn_p_lst -> push_back( Vec2D(tmp_x, tmp_y) );
            actual_num_spawn ++;
        }
    }

    cout << "rmax = " << rmax << ", num_trial: " << num_spawn << ", actual_num_spawn: " << actual_num_spawn << endl;
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


string LocalDLA::report_cluster(){
    ostringstream os;

    for (std::vector<Particle>::iterator it = cluster.begin() ; it != cluster.end(); ++it)
        os << " (" << (*it).pos.x << ", " << (*it).pos.y << ") ";

    os << endl;
    string s = os.str();
    return s;
}

















