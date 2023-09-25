#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<time.h>

int main(int argc, char *argv[])
{
    double start, stop;
    int i, j, k, l;
    int *a, *b, *c, *buffer, *ans;

    int size;
    // get size from command line 
    size = atoi(argv[1]);

    int rank, numprocs, line;

    MPI_Init(&argc, &argv); //MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get current process name
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs); // get current process number

    line = size/numprocs; // divide data into several blocks. one process handle one block

    a = (int*)malloc(sizeof(int)*size*size);
    b = (int*)malloc(sizeof(int)*size*size);
    c = (int*)malloc(sizeof(int)*size*size);
    // buffer size == data size need to handle 
    buffer = (int*)malloc(sizeof(int)*size*size); // block size
    ans = (int*)malloc(sizeof(int)*size*size); // saved block calculation result

    if (rank == 0) // main process
    {
        printf("test %d start\n", rank);

        // read matrix from file
        FILE *fp;
        fp = fopen("a.txt", "r"); // open file

        start = MPI_Wtime();
        for (i=0; i < size; i++){
            for(j=0;j < size; j++) {
                a[i*size + j] = i * size + j;
            }
        }
        // send matrix to other subprocess
        for (i=1; i < numprocs; i ++){
            MPI_Send(b, size*size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (k=1; k<numprocs; k++)
        {
            MPI_Recv(ans, line*size, MPI_INT, k, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // pass value to array c
            for (i=0; i <line; i ++)
            {
                for (j=0; j < size; j++)
                {
                    c[((k-1)*line +i)*size + j] = ans[i*size +j];

                }
            }
            // calculate a array left value
            for (i = (numprocs-1) *line; i < line; i ++)
            {
                for (j =0; j < size; j ++)
                {
                    int temp = 0;
                    for (k=0; k < size; k++){
                        temp += a[i*size+k] * b[k* size + j];
                    }
                    c[i * size + j] = temp;
                }
            }

            fp = fopen("c.txt", "w");

            for(i =0; i < size; i++) {
                for(j=0; j < size; j ++){
                    fprintf(fp, "%d", c[i*size+j]);
                }
                fputc('\n', fp);
            }
            fclose(fp);

            // calculate time
            stop = MPI_Wtime();
            printf("tast %d end\n", rank);
            printf("rank: %d time: %lfs\n", rank, stop-start);

            free(a);
            free(b);
            free(c);
            free(buffer);
            free(ans);
        }

    }
    // other process accept data and calculate
    else 
    {
        printf("tast %d start\n", rank);
        //receieve broadcast data (matrix b)
        MPI_Recv(b, size*size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // calculate matrix result an dsend to main process
        for (i=0; i <line; i++)
        {
            for (j=0; j < size; j++)
            {
                int temp = 0;
                for (k=0; k <size; k++) {
                    temp += buffer[i*size + k] * b[k*size + j];
                }
                ans[i*size+j] = temp;
            }
        }

        MPI_Send(ans, line*size, MPI_INT, 0, 3, MPI_COMM_WORLD);
        printf("tast %d end\n", rank);
    }

    MPI_Finalize();

    
    return 0;
}