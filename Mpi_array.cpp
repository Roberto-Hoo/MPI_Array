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

#define  ARRAYSIZE    12
#define  MASTER       0

double data[ARRAYSIZE];
int world_size; // Numero de processos
int world_rank; // Identificacao do processo
int rc,
        dest,
        endereco,
        tag1,
        tag2,
        source,  // Processo origem
tamanho_do_Pedaco;

bool debug = false; // Imprime os vetores antes de recv, depois de Recv e depois de update
bool debug2 = true; // Imprime o vetor final
double mysum, sum;
MPI_Status status;

/*
 *  Sub rotina que atualiza cada subpedaco da vetor
 *  data[i] = data[i] + i * 1.0;
 *  E imprime a soma deste subvetor e o numero do processo
 *  que gerencia esta parte do vetor
 */
double update(int meuEndereco, int tamPedaco, int world_rank) {
    double mySum = 0.0;

    /* Perform addition to each of my array elements and keep my sum */
    for (int i = meuEndereco; i < meuEndereco + tamPedaco; i++) {
        data[i] = data[i] + i * 1.0;
        mySum = mySum + data[i];
    }
    printf("Processo %d minha soma = %6.1f\n", world_rank, mySum);
    return (mySum);
}

int minimo(int a, int b) {
    if (a > b)
        return b;
    else
        return a;
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
    tag2 = 2;
    tag1 = 1;

    //* **** Master task only ***** */
    if (world_rank == MASTER) {

        /* Initialize the array */
        sum = 0.0;
        printf("\nVetor data(%d) = ( ", world_rank);
        for (int i = 0; i < ARRAYSIZE; i++) {
            data[i] = (i + 1) * 1.0;
            printf(" %4.0f", data[i]);
            sum = sum + data[i];
        }
        printf(" )\nInitialized array sum = %4.1f\n", sum);

        /* Send each task its portion of the array - master keeps 1st part */
        endereco = tamanho_do_Pedaco;
        for (dest = 1; dest < world_size; dest++) {
            // Envia o endereco para o precesso destino
            MPI_Send(&endereco, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
            MPI_Send(&data[endereco], tamanho_do_Pedaco, MPI_DOUBLE, dest, tag2, MPI_COMM_WORLD);
            printf("Enviados %d elementos para processo %d endereco = %d\n", tamanho_do_Pedaco, dest, endereco);
            endereco = endereco + tamanho_do_Pedaco; // Atualiza o endereço para o próximo processo
        }

        /* Master does its part of the work */
        endereco = 0;
        // Atualiza e a sub rotina update imprime a soma do processo master
        mysum = update(endereco, tamanho_do_Pedaco, world_rank);

        /* Wait to receive results from each task */
        for (int i = 1; i < world_size; i++) {
            source = i;
            MPI_Recv(&endereco, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
            MPI_Recv(&data[endereco], tamanho_do_Pedaco, MPI_DOUBLE, source, tag2, MPI_COMM_WORLD, &status);
        }

        /* Get final sum and print sample results */
        //MPI_Reduce(&mysum, &sum, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
        printf("  Alguns resultados: \n");
        endereco = 0;
        for (int i = 0; i < world_size; i++) {
            printf("Vetor(%d) = ", i);
            for (int j = 0; j < minimo(5, tamanho_do_Pedaco); j++)
                printf("  %5.1f", data[endereco + j]);
            printf("\n");
            endereco = endereco + tamanho_do_Pedaco;
        }
        //printf("  Soma final = %6.1f \n", sum);

        if (debug2) {
            printf("\nVetor final data(%d) = ( ", world_rank);
            for (int i = 0; i < ARRAYSIZE; i++) {
                printf(" %4.0f", data[i]);
            }
            printf(" )\n");
        }

    }  /* end of master section */

    //***** Non-master tasks only *****/
    if (world_rank > MASTER) {

        if (debug) {
            printf("\nVetor antes de recv data(%d) = ( ", world_rank);
            for (int i = 0; i < ARRAYSIZE; i++) {
                printf(" %5.1f", data[i]);
            }
            printf(" )\n");
        }


        /* Receive my portion of array from the master task */
        source = MASTER;
        MPI_Recv(&endereco, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[endereco], tamanho_do_Pedaco, MPI_DOUBLE, source, tag2, MPI_COMM_WORLD, &status);

        if (debug) {
            printf("\nVetor depois de recv data(%d) = ( ", world_rank);
            for (int i = 0; i < ARRAYSIZE; i++) {
                printf(" %5.1f", data[i]);
            }
            printf(" )\n");
        }

        // Atualiza o pedaco do vetor e a sub rotina update imprime a soma deste pedaco do vetor
        mysum = update(endereco, tamanho_do_Pedaco, world_rank);

        if (debug) {
            printf("\nVetor depois de atualizar data(%d) = ( ", world_rank);
            for (int i = 0; i < ARRAYSIZE; i++) {
                printf(" %5.1f", data[i]);
            }
            printf(" )\n");
        }

        /* Send my results back to the master task */
        dest = MASTER;
        MPI_Send(&endereco, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
        MPI_Send(&data[endereco], tamanho_do_Pedaco, MPI_DOUBLE, MASTER, tag2, MPI_COMM_WORLD);
        //MPI_Reduce(&mysum, &sum, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);

    } /* end of non-master */

    MPI_Reduce(&mysum, &sum, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    if (world_rank == MASTER)
        printf("  Soma final = %6.1f \n", sum);


    MPI_Finalize();

    return 0;
}   /* end of main */


