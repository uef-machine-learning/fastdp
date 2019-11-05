
/*#include "ivec.h"*/
/*#include "stat.h"*/
#include "timer.h"


double calculate_sum_edges(kNNGraph* kNN, int recall_K) {
    double sum=0.0;
    for (int i = 0; i < kNN->size; i++) {
        double sumPart=0.0;
        for (int j = 0; j < recall_K; j++) {
                sumPart += (double) kNN->list[i].items[j].dist;
        }
        sum += sumPart;
    }
    return sum;
}

float calculate_recall_sampled(kNNGraph* ground_truth, kNNGraph* kNN, int recall_K) {


    g_timer.pause();
    /*float recall = 0;*/
    float recall = 0;
    float cost = 0;
    unsigned long long recall_count = 0;
    int gt_idx;
    float time_diff;

    printf("{} k=%i recall for %i/%i points\n",kNN->k,ground_truth->size,kNN->size);
    fflush(stdout);

    if( ground_truth->k <= recall_K){
        printf("%d <= %d\n", ground_truth->k , kNN->k);
        //        terminal_error("Too small k in ground truth"); //TODO:
    }

    if(recall_K > kNN->k){
        recall_K=kNN->k; //TODO
        /*printf("%d <= %d\n", ground_truth->k , kNN->k);*/
    }

    for (int i = 0; i < ground_truth->size; ++i) {

        if(ground_truth->format == RANDOM_SAMPLED_BRUTEFORCE ||
                ground_truth->format == SAMPLED_BRUTEFORCE) {
            gt_idx = ground_truth->list[i].id;
        }
        else {
            terminal_error("Unknown format");
        }

        for (int j = 0; j < recall_K; ++j) {
            for (int m = 0; m < recall_K; ++m) {
                if(kNN->list[gt_idx].items[m].id == ground_truth->list[i].items[j].id) {
                    recall_count++;
                    break;
                }
            }
        }
    }

    double sumEdgesGt = calculate_sum_edges( ground_truth, recall_K);
    double sumEdges = calculate_sum_edges( kNN, recall_K);
    double approx = sumEdges/sumEdgesGt;

    /*printf("sumEdges:%f sumEdgesGt:%f\n",sumEdgesGt,sumEdges);*/

    cost = ((float)g_dist_count)/(((float)(kNN->size - 1))*((float)(kNN->size))/2 ); //TODO: verify if correct (-1) ?
    recall = ((float)recall_count)/(recall_K*ground_truth->size);

    g_timer.contin();
    time_diff = g_timer.tuck("0:");
    printf("RECALL: %f (SAMPLED %i/%i), TIME: %f recall_count=%llu kNN->k=%d recall_K=%d cost:%f approx: %f\n",recall,ground_truth->size,kNN->size, time_diff, recall_count, kNN->k, recall_K, cost,approx);
    fflush(stdout);
    return recall;

}

float calculate_recall(kNNGraph* ground_truth, kNNGraph* kNN, int recall_K) {
    if(ground_truth->format == SAMPLED_BRUTEFORCE ||
            ground_truth->format == RANDOM_SAMPLED_BRUTEFORCE) {
        return calculate_recall_sampled(ground_truth, kNN, recall_K);
    }
    else {
        /*return calculate_recall_full(ground_truth, kNN);*/
        //TODO
    }
    return 0.0;
}

float calculate_recall(kNNGraph* ground_truth, kNNGraph* kNN) {
    return calculate_recall(ground_truth, kNN, kNN->k);
}

void calculate_recall(kNNGraph* kNN) {
    if(g_ground_truth) { calculate_recall(g_ground_truth,kNN,g_options.recall_K);}
    /*else {*/
    /*float time_diff,cost;*/
    /*time_diff = g_timer.get_time();*/
    /*cost = ((float)g_dist_count)/(((float)(kNN->size - 1))*((float)(kNN->size))/2 );  ?*/
    /*printf("TIME: %f kNN->k=%d cost:%f\n", time_diff, kNN->k, cost);*/
    /*}*/
}



