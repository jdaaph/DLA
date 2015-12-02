/*
    Define Simulation Class
*/

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "mpi.h"
#include "GlobalDLA.h"

using namespace std;


int main(int argc, char *argv[]){

    GlobalDLA experiment = GlobalDLA();
    experiment.init(int argc, char *argv[]);
    experiment.test();

    cout << "Biu! Done!" << endl;
}
















































