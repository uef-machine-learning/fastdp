
#ifndef KNNG_OPTIONS_H
#define KNNG_OPTIONS_H

#include <stdlib.h>

struct knng_options {

    int recall_K;
    int nndes_K;

    int distance_type;
    float minkowski_p;


    int num_threads;

};

/*knng_options* init_knng_options();*/



#endif
