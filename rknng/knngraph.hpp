
#ifndef KNNGRAPH_H_
#define KNNGRAPH_H_

#define SAMPLED_BRUTEFORCE 2
#define RANDOM_SAMPLED_BRUTEFORCE 3


#include "dataset.hpp"
#include <float.h>




typedef int BOOL;
#define true 1
#define false 0


typedef struct kNNItem {
    int id;
    float dist;
    BOOL new_item;
    int visited;
} kNNItem;

typedef struct kNNList {
    // List of <size> number of nearest neighbors
    kNNItem * items;
    float max_dist;
    int size;
    // <id> of point which nearest neighbors this represents
    int id;
    BOOL is_exact;
} kNNList;

typedef struct kNNGraph {
    int size;
    int k;
    int format;
    kNNList * list;
    void * DS;
    unsigned long long dist_count;
} kNNGraph ;

kNNGraph* init_kNNGraph(int N, int K, int maxK);
kNNGraph * init_kNNGraph(int N, int K );
void  free_kNNGraph(kNNGraph * kNN);
void debug_graph(kNNGraph* knng);
int updatekNN(kNNGraph* kNN, int p1, int p2, float dist);
int update(kNNGraph* kNN, DataSet* data, int p1, int p2);
int update_one(kNNGraph* kNN, DataSet* DS, int p1, int p2);
inline int get_kNN_item_id(kNNGraph * kNN, int i_list, int i_k);
inline float get_kNN_item_dist(kNNGraph * kNN, int i_list, int i_k);
void set_kNN_val(kNNGraph * kNN, int i_list, int i_k, int id);
void set_kNN_val(kNNGraph * kNN, int i_list, int i_k, int id, float dist, BOOL new_item);
inline kNNList* get_kNN_list(kNNGraph * kNN, int i_list);
void set_kNN_id(kNNGraph * kNN, int i_list, int id);
kNNItem* get_kNN_item(kNNGraph * kNN, int i_list, int i_k);
int write_kNN_txt(const char * fname, kNNGraph * kNN);
void debugStringGraph(DataSet* DS,kNNGraph* knng,int i);
void debugVecGraph(DataSet* DS,kNNGraph* knng,int i);
void write_string_graph(const char * fname, kNNGraph* kNN, DataSet* DS);
int write_kNN_ivec(const std::string &path, kNNGraph * kNN ,int format);
void recalc_dist(kNNGraph * kNN, DataSet* data);
kNNGraph * load_kNN_ivec(const std::string &path, int format);

#endif
