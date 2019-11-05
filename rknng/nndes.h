
#ifndef NNDES_H
#define NNDES_H

#include <climits>

inline void nndes_join(DataSet* data, kNNGraph* kNN,int id_A, int i_knn, vector<vector<int>> &knn_list,
        unsigned long long &update_count) {
    int id_last = INT_MAX;
    for(std::vector<int>::iterator it_B = knn_list[i_knn].begin(); it_B != knn_list[i_knn].end(); ++it_B) {
        int id_B = *it_B;
        if(id_last == id_B) { continue;}
        if(id_A < id_B) {
            update_count += update(kNN,data,id_A,id_B);
        }
        id_last = id_B;
    }
}


inline void nndes_join_old(DataSet* data, kNNGraph* kNN,int id_A, int i_knn, vector<vector<int>> &knn_list,
        unsigned long long &update_count) {
    int id_last = INT_MAX;
    for(std::vector<int>::iterator it_B = knn_list[i_knn].begin(); it_B != knn_list[i_knn].end(); ++it_B) {
        int id_B = *it_B;
        if(id_last == id_B) { continue;}
        if(id_A != id_B) { //
            update_count += update(kNN,data,id_A,id_B);
        }
        id_last = id_B;
    }
}



unsigned long long nndes_iterate_limited(DataSet* data, kNNGraph* kNN, int _k_limit) {

    vector<vector<int>> new_knn(data->size);
    vector<vector<int>> old_knn(data->size);
    vector<vector<int>> reverse_new_knn(data->size);
    vector<vector<int>> reverse_old_knn(data->size);

    int k_limit = kNN->k;
    if(_k_limit>0) { k_limit = _k_limit;}

    unsigned long long update_count = 0;
    g_timer.tuck("  Build old_knn, new_knn ");
    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {
        new_knn[i_knn].clear();
        old_knn[i_knn].clear();
        reverse_new_knn[i_knn].clear();
        reverse_old_knn[i_knn].clear();

        for(int j = 0; j < k_limit && j < kNN->list[i_knn].size;j++) {
            kNNItem* ki = get_kNN_item(kNN,i_knn,j);
            if(ki->new_item
                    /*&& (k_limit <= 0 || new_knn[i_knn].size() <= k_limit)*/
              ) {
                new_knn[i_knn].push_back(ki->id);
                ki->new_item = false;
            } else {
                old_knn[i_knn].push_back(ki->id);
            }
        }
    }

    // Construct reverse kNN
    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {
        for(std::vector<int>::iterator it = new_knn[i_knn].begin(); it != new_knn[i_knn].end(); ++it) {
            new_knn[*it].push_back(i_knn);
        }
        for(std::vector<int>::iterator it = old_knn[i_knn].begin(); it != old_knn[i_knn].end(); ++it) {
            old_knn[*it].push_back(i_knn);
        }
    }

    g_timer.tuck("  START Local join");
    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {

        std::sort( old_knn[i_knn].begin(), old_knn[i_knn].end());
        std::sort( new_knn[i_knn].begin(), new_knn[i_knn].end());
        int last_id = INT_MAX;
        for(std::vector<int>::iterator it = new_knn[i_knn].begin(); it != new_knn[i_knn].end(); ++it)
        {
            int id_A = *it;

            if(id_A == last_id) {continue;}
            nndes_join(data,kNN,id_A,i_knn,new_knn,update_count);
            nndes_join_old(data,kNN,id_A,i_knn,old_knn,update_count);

            last_id = id_A;
        }
    }

    g_timer.tuck("  END Local join");

    return update_count;
}




#endif
