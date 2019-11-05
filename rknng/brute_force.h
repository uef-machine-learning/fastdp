
#ifndef _BRUTE_FORCE_H
#define _BRUTE_FORCE_H

kNNGraph* brute_force_search (DataSet* data, int k) {
    int N=data->size;

    kNNGraph* kNN = init_kNNGraph(N,k);

    printf("Staring brute force search\n");

#if defined(_OPENMP)
omp_set_num_threads(g_options.num_threads);
#pragma omp parallel for default(shared)
#endif
    for(int data_i = 0; data_i < N; data_i++) {
        if(data_i % 100 == 0) { printf("BRUTe FORCE %i/%i\n",data_i,N);}
        set_kNN_id(kNN, data_i, data_i);
        for(int data_j = 0; data_j < N; data_j++) {
            if(data_i == data_j){ continue;}
            float _dist = distance(data,data_i,data_j);
            {
                updatekNN(kNN,data_i,data_j,_dist);
            }
        }
    }
    return kNN;
}



typedef struct tinfo {
    kNNGraph* kNN;
    DataSet* data;
    int id;
    int num_threads;
} tinfo;

/*void* brute_force_search_pthread_seg (DataSet* data, kNNGraph* kNN, void* _arg) {*/
void* brute_force_search_pthread_seg (void* _arg) {

    /*pthread_detach(pthread_self());*/

    tinfo* arg = (tinfo*) _arg;
    DataSet* data = arg->data;
    kNNGraph* kNN = arg->kNN;
    int N=arg->data->size;

    printf("Thread id:%d\n",arg->id);
    /*return*/

    for(int data_i = 0; data_i < N; data_i++) {
        if(data_i % arg->num_threads != arg->id) { continue;}
        /*printf("thread_id=%d data_i=%d/%d\n",arg->id,data_i,N);*/
    /*if(data_i % 100 == 0) { printf("BRUTe FORCE %i/%i\n",data_i,N);}*/
    /*set_kNN_id(kNN, data_i, data_i);*/
        for(int data_j = 0; data_j < N; data_j++) {
            if(data_i == data_j){ continue;}
            float _dist = distance(data,data_i,data_j);
            {
                updatekNN(kNN,data_i,data_j,_dist);
            }
        }
    }
    pthread_exit(NULL);
}


kNNGraph* brute_force_search_pthread (DataSet* data, int k) {
    int N=data->size;
    int i;

    /*int num_threads = 4;*/
    int num_threads = g_options.num_threads;
    kNNGraph* kNN = init_kNNGraph(N,k);
    pthread_t* t = (pthread_t*) malloc(sizeof(pthread_t)*num_threads);

    printf("Staring brute force search\n");

    /*omp_set_num_threads(g_options.num_threads);*/
    /*for(int data_i = 0; data_i < N; data_i++) {*/
    /*if(data_i % 100 == 0) { printf("BRUTe FORCE %i/%i\n",data_i,N);}*/
    /*set_kNN_id(kNN, data_i, data_i);*/
    /*for(int data_j = 0; data_j < N; data_j++) {*/
    /*if(data_i == data_j){ continue;}*/
    /*float _dist = distance(data,data_i,data_j);*/
    /*{*/
    /*updatekNN(kNN,data_i,data_j,_dist);*/
    /*}*/
    /*}*/
    /*}*/

    for (i = 0; i < num_threads; i++) {
        /*int* id = (int*) malloc(sizeof(int));*/
        tinfo* arg = (tinfo*) malloc(sizeof(tinfo));
        arg->id = i;
        arg->kNN = kNN;
        arg->data = data;
        arg->num_threads = num_threads;
        pthread_create(&(t[i]), NULL, brute_force_search_pthread_seg, (void*) arg);
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(t[i], NULL);
    }

    /*sleep(3);*/

    return kNN;
}


#endif
