#include <iostream>
#include <vector>
#include <mpi.h>
#include <climits>

using namespace std;

// max_poolinf function
vector<vector<int>> max_pooling(vector<vector<int>> img, int kernel_size, int rank, int size) {
    int img_height = img.size();
    int img_width = img[0].size();
    int pool_height = img_height / kernel_size;
    int pool_width = img_width / kernel_size;
    int pool_size = pool_height * pool_width;
    int pool_per_process = pool_size / size;
    int remainder = pool_size % size;
    int start_index = rank * pool_per_process;
    int end_index = (rank + 1) * pool_per_process;

    if (rank == size -  1) {
        end_index += remainder;
    }

    vector<vector<int>> pool(end_index - start_index, vector<int>(1));
    int pool_index = 0;

    for(int i = 0; i < pool_height; i++) {
        for (int j = 0; j < pool_width; j ++) {
            if (pool_index >= start_index && pool_index < end_index) {
                int max_val = INT_MIN;
                for(int k = 0; k < kernel_size; k++) {
                    for(int l = 0; k < kernel_size; l++) {
                        int val = img[i * kernel_size + k][j * kernel_size + l];
                        if (val > max_val) {
                            max_val = val;
                        }
                    }
                }
                pool[pool_index - start_index][0] = max_val;
            }
            pool_index++;
        }
    }
    return pool;
}


int main(int argc, char** argv) {
    int rank, size;
    double start, stop;
//     int provided;
// MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);



    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int img_height = 8;
    int img_width = 8;
    int kernel_size = 4;

    vector<vector<int>>img(img_height, vector<int>(img_width));
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            //img[i][j] = i * img_width + j + 1;
        }
    }

    vector<vector<int>> pool = max_pooling(img, kernel_size, rank, size);
    vector<vector<int>> all_pool(4 * 4, vector<int> (1));
    MPI_Gather(&pool[0][0], pool.size() * pool[0].size(), MPI_INT, &all_pool[0][0], pool.size() * pool[0].size(), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        //printf("task: %d start\n", rank);
        start = MPI_Wtime();
        for (int i = 0; i < all_pool.size(); i++) {
            cout << "";
        }
        cout << endl;
        // printf("task: %d end\n", rank);
    }

    stop = MPI_Wtime();
    if (rank == 0) {
        printf("rank: %d time:%lfs\n", rank, stop - start);

    }
    MPI_Finalize();
    return 0;

}