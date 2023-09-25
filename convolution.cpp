#include <mpi.h>
#include <iostream>
#include <vector>
#include <time.h>

using namespace std;

// definition for the convolution function
vector<vector<double>> convolution(vector<vector<double>> image, vector<vector<double>> kernel) {
    int image_height = image.size();
    int image_width = image[0].size();
    int kernel_height = kernel.size();
    int kernel_width = kernel[0].size();
    int output_height = image_height - kernel_height + 1;
    int output_width = image_width - kernel_width + 1;

    vector<vector<double>> output(output_height, vector<double>(output_width, 0.0));

    for(int i = 0; i < output_height; i++) {
        for(int j = 0; j < output_width; j ++) {
            for(int k = 0; k < kernel_height; k ++) {
                for(int l = 0; l < kernel_width; l ++) {
                    output[i][j] += image[i+k][j+l] * kernel[k][l];
                }
            }
        }
    }

    return output;
}



int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    double start, stop;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // definition fot the image and convolution kernel
    vector<vector<double>> image = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16}};
    vector<vector<double>> kernel = {{1,0},{0,1}};

    //calculate the rows for each process
    int rows_per_process = image.size()/size;
    int remainder = image.size() % size;
    int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process;

    if (rank == size - 1) {
        end_row += remainder;
    }

    // each process handle one block of the image
    vector<vector<double>> local_image(end_row - start_row, vector<double>(image[0].size(), 0.0));
    for (int i = start_row;i < end_row; i++) {
        for( int j = 0; j < image[0].size(); j++) {
            local_image[i - start_row][j] = image[i][j];
        }
    }

    //each process calculate own kernel result
    vector<vector<double>> local_output = convolution(local_image, kernel);

    //send each process kernel result to main process

    if (rank == 0) {
        printf("tast %d start\n", rank);
        start = MPI_Wtime();
        vector<vector<double>> output(image.size() - kernel.size() + 1, vector<double>(image[0].size() - kernel[0].size() +1, 0.0));

        for (int i = 0; i < local_output.size(); i ++) {
            for(int j = 0; j < local_output[0].size(); j++) {
                output[i][j] = local_output[i][j];
            }
        }

        for (int i = 1; i < size; i++) {
            int start_row = i * rows_per_process;
            int end_row = start_row + rows_per_process;

            if (i == size - 1) {
                end_row += remainder;
            }

            vector<vector<double>> temp_output(end_row - start_row, vector<double>(image[0].size() - kernel[0].size() + 1, 0.0));
            MPI_Recv(&temp_output[0][0], (end_row -start_row) * (image[0].size() - kernel[0].size() + 1), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int j = start_row; j < end_row; j ++) {
                for (int k = 0; k < image[0].size() - kernel[0].size();k++) {
                    output[j - kernel.size() + 1][k] = temp_output[j - start_row][k];

                }
            }
        }
        // output convolution result
        // for(int i = 0; i < output.size(); i ++) {
        //     for (int j = 0; j < output[0].size(); j++) {
        //         cout << output[i][j] << "";
        //     }
        //     cout << endl;
        // }

        stop = MPI_Wtime();
        printf("tast %d end\n", rank);
        printf("rank %d time:%lfs\n", rank, stop - start);
        
    }
    else {
        printf("tast %d start\n", rank);
        MPI_Send(&local_output[0][0], (end_row - start_row)*(image[0].size() - kernel[0].size() + 1), MPI_DOUBLE, 0,0, MPI_COMM_WORLD);
        printf("tast %d end\n", rank);
    }

    MPI_Finalize();

    return 0;
}