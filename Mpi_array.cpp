/*
* *****************************************************************************
* FILE: mpi_array.c
* DESCRIPTION:
*   MPI Example - Array Assignment - C Version
*   This program demonstrates a simple data decomposition. The master task
*   first initializes an array and then distributes an equal portion that
*   array to the other tasks. After the other tasks receive their portion
*   of the array, they perform an addition operation to each array element.
*   They also maintain a sum for their portion of the array. The master task
*   does likewise with its portion of the array. As each of the non-master
*   tasks finish, they send their updated portion of the array to the master.
*   An MPI collective communication call is used to collect the sums
*   maintained by each task.  Finally, the master task displays selected
*   parts of the final array and the global sum of all array elements.
*   NOTE: the number of MPI tasks must be evenly disible by 4.
* AUTHOR: Blaise Barney
* LAST REVISED: 04/02/05
***************************************************************************
*/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define  ARRAYSIZE    8
#define  MASTER       0

double data[ARRAYSIZE];
int world_size; // Numero de processos
int world_rank; // Identificacao do processo
int        rc,
         dest,
       endereco,
         tag1,
         tag2,
       source,  // Processo origem
    tamanho_do_Pedaco;

double mysum, sum;
MPI_Status status;

double update(int meuEndereco, int chunk, int myid) {
    int i;
    double mysum;
    /* Perform addition to each of my array elements and keep my sum */
    mysum = 0;
    for (i = meuEndereco; i < meuEndereco + chunk; i++) {
        data[i] = data[i] + i * 1.0;
        mysum = mysum + data[i];
    }
    printf("Task %d mysum = %e\n", myid, mysum);
    return (mysum);
}

int main() {

    //***** Initializations *****
    MPI_Init(NULL, NULL);  // Inicializa o MPI;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size % 4 != 0) {
        printf("Quitting. Number of MPI tasks must be divisible by 4.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(0);
    }
    printf("MPI task %d has started...\n", world_rank);
    tamanho_do_Pedaco = (ARRAYSIZE / world_size);
    tag2 = 1;
    tag1 = 2;

    //* **** Master task only ***** */
    if (world_rank == MASTER) {

        /* Initialize the array */
        sum = 0.0;
        printf ("\nVetor data(%d) = ( ",world_rank);
        for (int i = 0; i < ARRAYSIZE; i++) {
            data[i] = (i+1) * 1.0;
            printf("%4.1f",data[i]);
            sum = sum + data[i];
        }
        printf(" )\nInitialized array sum = %4.1f\n", sum);

        /* Send each task its portion of the array - master keeps 1st part */
        endereco = tamanho_do_Pedaco;
        for (dest = 1; dest < world_size; dest++) {
            MPI_Send(&endereco, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
            MPI_Send(&data[endereco], tamanho_do_Pedaco, MPI_FLOAT, dest, tag2, MPI_COMM_WORLD);
            printf("Enviados %d elementos para processo %d endereco= %d\n", tamanho_do_Pedaco, dest, endereco);
            endereco = endereco + tamanho_do_Pedaco; // Atualiza o endereço para o próximo processo
        }

        /* Master does its part of the work */
        endereco = 0;
        mysum = update(endereco, tamanho_do_Pedaco, world_rank);

        /* Wait to receive results from each task */
        for (int i = 1; i < world_size; i++) {
            source = i;
            MPI_Recv(&endereco, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
            MPI_Recv(&data[endereco], tamanho_do_Pedaco, MPI_FLOAT, source, tag2,
                     MPI_COMM_WORLD, &status);
        }

       /* Get final sum and print sample results */
        MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
        printf("Sample results: \n");
        endereco = 0;
        for (int i = 0; i < world_size; i++) {
            for (int j = 0; j < 5; j++)
                printf("  %e", data[endereco + j]);
            printf("\n");
            endereco = endereco + tamanho_do_Pedaco;
        }
        printf("*** Final sum= %e ***\n", sum);

    }  /* end of master section */



/***** Non-master tasks only *****/

    if (world_rank > MASTER) {

/* Receive my portion of array from the master task */
        source = MASTER;
        MPI_Recv(&endereco, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[endereco], tamanho_do_Pedaco, MPI_FLOAT, source, tag2,
                 MPI_COMM_WORLD, &status);

        mysum = update(endereco, tamanho_do_Pedaco, world_rank);

/* Send my results back to the master task */
        dest = MASTER;
        MPI_Send(&endereco, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
        MPI_Send(&data[endereco], tamanho_do_Pedaco, MPI_FLOAT, MASTER, tag2, MPI_COMM_WORLD);

        MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);

    } /* end of non-master */


    MPI_Finalize();

    return 0;
}   /* end of main */


