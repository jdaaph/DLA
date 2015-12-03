#include <vector>;
#include "Vec2D.h"
#include "Constants.h"
#include "LocalDLA.h"
#include "Particle.h"

#include <sstream>
#include <string>

using namespace std;


void LocalDLA::update(){

}


void LocalDLA::migrate(){

}


void LocalDLA::add_particle(vector<Particle>* spawn_p_lst){
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

    for (std::vector<Particle>::iterator it = myvector.begin() ; it != myvector.end(); ++it)
        cout << " (" << (*it).x << ", " << (*it).x << ") ";

    os << endl;
    string s = os.str();
    return s;
}




















