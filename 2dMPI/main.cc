/*
    Define Simulation Class
*/

#include "Constants.h"
#include "Vec2D.h"
#include <vector>
#include "mpi.h"
#include "GlobalDLA.h"
#include <time.h> 
#include <cstdlib> 

using namespace std;


int main(int argc, char *argv[]){

    // only odd perfect square -np number would be supported
    srand(time(NULL));

    GlobalDLA experiment = GlobalDLA();
    experiment.init(argc, argv);
    // experiment.test();
    experiment.test_balance();

    cout << "Biu! Done!" << endl;
}
















































