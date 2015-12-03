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


string LocalDLA::report_domain(){
    ostringstream os;
    os << "Upper: (" << upper.x << ", " << upper.y << "), Lower: (" << lower.x << ", " << lower.y << ")" << endl;
    string s = os.str();
    return s;
}























