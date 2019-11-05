
#ifndef KNNGRAPH_H_
#define KNNGRAPH_H_

#define SAMPLED_BRUTEFORCE 2
#define RANDOM_SAMPLED_BRUTEFORCE 3

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
} kNNGraph ;


kNNGraph * init_kNNGraph(int N, int K, int maxK) {
    kNNGraph* kNN = (kNNGraph*) safemalloc(sizeof(kNNGraph));
    kNN->list = (kNNList*) safemalloc(sizeof(kNNList)*N);
    kNN->size = N;
    kNN->k = K;

    kNNList * curlist = kNN->list;
    for (int i = 0; i < N; i++)
    {
        curlist->items = (kNNItem*) safemalloc(sizeof(kNNItem)*maxK);
        curlist->size = 0;
        curlist->max_dist = FLT_MAX;
        curlist->is_exact = false;
        curlist->id = (int) i;
        curlist++;
    }
    return kNN;
}

kNNGraph * init_kNNGraph(int N, int K ) {
    return init_kNNGraph(N, K, K);
}

void  free_kNNGraph(kNNGraph * kNN) {
    for (int i = 0; i < kNN->size; i++)
    {
        free(kNN->list[i].items);
    }
    free(kNN->list);
    free(kNN);
}


void debug_graph(kNNGraph* knng) {
    printf("knng->k: %d\n",knng->k);

    for (int i_row=0; i_row < 10; i_row++) {
        for (int j = 0; j < knng->k; ++j) {
            printf("%d ",knng->list[i_row].items[j].id);
        }
        printf("\n");
    }
}

int updatekNN(kNNGraph* kNN, int p1, int p2, float dist)
{
#ifdef SANITYCHECK
    if(p1 == p2 && kNN->format != RANDOM_SAMPLED_BRUTEFORCE) {
        printf("p1=%u=p2\n",p1);
        terminal_error("p1=p2");
    }
    if(p1 >= kNN->size) {
        printf("p1 = %u >= kNN->size\n",p1);
        terminal_error(" ");
    }
#endif

    kNNList* kl = &kNN->list[p1];
    kNNItem* ki = kl->items;

    if(kl->max_dist > dist || kl->size < kNN->k) {
        int i = 0;
        for(i=0; i <= kl->size; i++) {
            //if p2 already in list. Should not happen?
            // uninitialized if size==0
            if(kl->size > 0 && ki->id == p2) { return 0 ;} //TODO:??

            if (ki->dist > dist || i == kl->size) {
                int moveamount = kNN->k - i -1;
                if(moveamount > 0 ) { //TODO: needed?
                    // Move from ki to ki+1
                    memmove(ki+1,ki,moveamount*sizeof(kNNItem));
                }
                ki->id = p2;
                ki->dist = dist;
                ki->new_item = true; //TODO: not needed in all search types

                if(kl->size < kNN->k) { kl->size++;}
                kl->max_dist = kl->items[kl->size -1].dist; //TODO: optimize?
                break;
            }
            ki++;
        }
        return 1; // Did update
    }
    else {
        return 0; // Did not update
    }
}

int update(kNNGraph* kNN, DataSet* data, int p1, int p2) {
// To avoid concurrent memory write access
// TODO: better fix
    int update_count = 0;
#ifndef _OPENMP
    g_dist_count++;
#endif
#ifdef SANITYCHECK
    if ( p2 >= kNN->size || p1 >= kNN->size) {
        printf("%u,%u x %d\n",p1,p2,kNN->size);
        terminal_error("invalid p1,p2");
    }
#endif

    float dist;
    dist = distance(data,p1,p2);
    update_count += updatekNN(kNN,p1,p2,dist);
    update_count += updatekNN(kNN,p2,p1,dist);
    return update_count;
}


int update_one(kNNGraph* kNN, DataSet* DS, int p1, int p2) {
    g_dist_count++;
    int update_count = 0;
#ifdef SANITYCHECK
    if ( p2 >= kNN->size || p1 >= kNN->size) {
        printf("%u,%u x %d\n",p1,p2,kNN->size);
        terminal_error("invalid p1,p2");
    }
#endif
    float dist = distance(DS,p1,p2);
    update_count += updatekNN(kNN,p1,p2,dist);
    return update_count;
}





inline int get_kNN_item_id(kNNGraph * kNN, int i_list, int i_k) {
#ifdef SANITYCHECK
    if (i_list < 0 || i_list > kNN->size || i_k >= kNN->k
            || i_k >= kNN->list[i_list].size) {
        printf("%d x %d\n",i_list,kNN->size);
        std::raise(SIGINT);
        terminal_error("get_kNN_item_id: invalid i_list or i_k params");
    }
#endif
    return kNN->list[i_list].items[i_k].id;
}

inline float get_kNN_item_dist(kNNGraph * kNN, int i_list, int i_k) {
#ifdef SANITYCHECK
    if (i_list < 0 || i_list > kNN->size || i_k >= kNN->k
            || i_k >= kNN->list[i_list].size) {
        printf("%d x %d\n",i_list,kNN->size);
        std::raise(SIGINT);
        terminal_error("get_kNN_item_id: invalid i_list or i_k params");
    }
#endif
    return kNN->list[i_list].items[i_k].dist;
}




inline void set_kNN_val(kNNGraph * kNN, int i_list, int i_k, int id) {
    kNNItem* ki = &kNN->list[i_list].items[i_k];
    ki->id = id;
}

inline void set_kNN_val(kNNGraph * kNN, int i_list, int i_k, int id, float dist, BOOL new_item) {
    //TODO: &
    kNNItem ki = kNN->list[i_list].items[i_k];
    ki.id = id;
    ki.dist = dist;
    ki.new_item = new_item;
}

inline kNNList* get_kNN_list(kNNGraph * kNN, int i_list) {
#ifdef SANITYCHECK
    if (i_list < 0 || i_list > kNN->size) {
        printf("%d x %d\n",i_list,kNN->size);
        /*std::raise(SIGINT);*/
        terminal_error("invalid i_list");
    }
#endif
    return &(kNN->list[i_list]);
}

inline void set_kNN_id(kNNGraph * kNN, int i_list, int id) {
#ifdef SANITYCHECK
    if (i_list < 0 || i_list > kNN->size ) {
        printf("%d x %d\n",i_list,kNN->size);
        /*std::raise(SIGINT);*/
        terminal_error("get_kNN_item_id: invalid i_list or i_k params");
    }
#endif
    kNN->list[i_list].id = id;
}


inline kNNItem* get_kNN_item(kNNGraph * kNN, int i_list, int i_k) {
    return &kNN->list[i_list].items[i_k];
}



int write_kNN_txt(const char * fname, kNNGraph * kNN) {
    // Write kNN file in txt format

    FILE *fp;
    fp = fopen(fname,"w");
    int idx = 0;
    int K = 0;
    K = kNN->k;

    fprintf(fp,"%d\n",kNN->size);
    for(int data_i = 0; data_i < kNN->size; data_i++) {

        fprintf(fp,"%u",kNN->list[data_i].id);
        fprintf(fp," %d",K);

        // Neighbors
        for(int nn_i = 0; nn_i < kNN->k; nn_i++) {
            idx = (int) get_kNN_item_id(kNN,data_i,nn_i);
            fprintf(fp," %i",idx);
        }
        // Distances to neighbors
        for(int nn_i = 0; nn_i < kNN->k; nn_i++) {
            float dist = get_kNN_item_dist(kNN,data_i,nn_i);
            fprintf(fp," %f",dist);
        }
        fprintf(fp,"\n");
    }

    fclose(fp);
    return 1;
}

void debugStringGraph(DataSet* DS,kNNGraph* knng,int i) {
    string stmp = DS->strings->at(i);
    printf("%s: ",(DS->strings->at(i)).c_str());
    for(int j=0;j<knng->k;j++) {
        printf("%s(%.4f), ",(DS->strings->at(knng->list[i].items[j].id)).c_str(),knng->list[i].items[j].dist);
    }
    printf("\n");
}

void debugVecGraph(DataSet* DS,kNNGraph* knng,int i) {
    /*string stmp = DS->strings->at(i);*/
    printf("%d: ",i);
    for(int j=0;j<knng->k;j++) {
        printf("%d(%.4f), ",knng->list[i].items[j].id,knng->list[i].items[j].dist);
    }
    printf("\n");
}




void write_string_graph(const char * fname, kNNGraph* kNN, DataSet* DS) {
    FILE *fp;
    fp = fopen(fname,"w");

    for(int i=0;i<DS->size;i++) {
        string stmp = DS->strings->at(i);
        fprintf(fp,"%s: ",(DS->strings->at(i)).c_str());
        for(int j=0;j<kNN->k;j++) {
            fprintf(fp,"%s(%.4f), ",(DS->strings->at(kNN->list[i].items[j].id)).c_str(),kNN->list[i].items[j].dist);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}




int write_kNN_ivec(const std::string &path, kNNGraph * kNN ,int format) {
    // Write kNN file in ivec format

    // kNN->size number of rows:
    // first 4 bytes(int): length of vector (K+1)
    // second 4 bytes(int): id of data point
    // next K*4 bytes(int): ids of K nearest neighbors

    //TODO: Store distance values also

    const char * fname = path.c_str();
    FILE *fp;
    fp = fopen(fname,"w");
    int idx = 0;
    int K = 0;
    K = kNN->k+1;

    for(int data_i = 0; data_i < kNN->size; data_i++) {
        fwrite((char*)(&K),1,sizeof(int),fp);
        fwrite((char*)(&(kNN->list[data_i].id)),1,sizeof(int),fp);
        for(int nn_i = 0; nn_i < kNN->k; nn_i++) {
            idx = (int) get_kNN_item_id(kNN,data_i,nn_i);
            fwrite((char*)(&idx),1,sizeof(int),fp);
        }
    }


    fclose(fp);
    return 1;
}



void recalc_dist(kNNGraph * kNN, DataSet* data) {
    for(int data_i = 0; data_i < kNN->size; data_i++) {
        for(int j = 0; j < kNN->k; j++) {
            int idx = kNN->list[data_i].items[j].id;
            kNN->list[data_i].items[j].dist = distance(data,data_i,idx);
        }
    }
}


kNNGraph * load_kNN_ivec(const std::string &path, int format) {
    kNNGraph* kNN;
    int bf_K = 0;
    int bf_N = 0;
    unsigned int filesize = 0;
    int buf; //TODO: change to int
    const char * fname = path.c_str();
    FILE *fp;
    size_t result;
    fp = fopen(fname,"r");
    if(!fp) {terminal_error("File does not exist\n");}

    fgets((char*)(&bf_K), sizeof(int),fp);
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);


    if (filesize % ((bf_K+1)*sizeof(int)) != 0) {
        terminal_error("Data amount does not match.");
             }

    bf_N = (filesize)/((bf_K+1)*sizeof(int));

    printf("\nK=%i filesize=%u num_vectors=%f\n",bf_K,filesize,(filesize+0.0)/((bf_K+1)*sizeof(int)));


    if(format == RANDOM_SAMPLED_BRUTEFORCE ||
            format == SAMPLED_BRUTEFORCE ) {
        kNN = init_kNNGraph(bf_N,bf_K - 1);
    }
    else {
        terminal_error("Format error");
    }

    kNN->format = format; //TODO: if bf_N < N

    int i=0;
    int i_k=0;
    int i_vector=0;
    while(!feof(fp) && i*sizeof(int) < filesize) {

        // start of vector (vector header)
        if(i % (bf_K+1) == 0) {
            if(i > 0) {i_vector++;}
            i_k = 0;

            result = fread((char*)(&buf), sizeof(int),1,fp);
            if(((int) (buf)) != bf_K) {
                printf("[i:%d,i_vector:%d,fs:%d] %zu bytes bf_K: %d, buf_first:%u",i,i_vector,
                        filesize,result, bf_K,((int) (buf)));
                terminal_error("Assert error in reading vector");
            }
            if(kNN->format == RANDOM_SAMPLED_BRUTEFORCE ||
                    kNN->format == SAMPLED_BRUTEFORCE ) {
                fread((char*)(&buf), sizeof(int),1,fp);
                set_kNN_id(kNN,i_vector,((int) (buf)));
                i++;
            }
            //printf("%f ",*data);
        }
        else {
            fread((char*)(&buf), sizeof(int),1,fp);
            set_kNN_val(kNN,i_vector,i_k,((int) (buf)));
            /**cur = (int) (buf);*/
            i_k++;

        }
     i++;
    }
    //TODO: implement sanity check

    printf("Read %d vectors, %d elements, k=%d\n",i_vector+1,i,bf_K);
    return kNN;
} //END load_kNN

#endif
