#ifndef __CONSTANTS__
#define __CONSTANTS__

/*
    Defines all constants that will be used in the program with MACROS
*/

#define MIN_NODE_SIZE 4
#define GHOST_REGION 1


#define SPAWN_RATE 0.1

// the decay rate of block size (short side)
#define ALPHA 2
#define RANDOM_SEED_SPAWN 1608911
#define RANDOM_SEED_WALK 888122


// Bound factor: if particle has a r > b.f. * rmax then it's discarded
#define BOUND_FACTOR 2


#define TIME_STEP 10e4
#define TERMINATE_RMAX 200
#define BALANCE_INTERVAL_FACTOR 5


#endif //__CONSTANTS__





