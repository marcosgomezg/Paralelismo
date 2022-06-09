
//Muestra la matriz generada por root, las submatrices y el vector
//recibidos por cada proceso, los subvectores resultado generados por
//cada proceso y el vector resultado obtenido por el root.
//Incluye medición de tiempos de comunicación y de cómputo

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 1
#define N 10

int main(int argc, char *argv[] ) {

	int i, j, f, c;

	struct timeval  tv1, tv2, tv3, tv4;
	unsigned long usec_comm = 0;
	unsigned long usec_comp = 0;
	unsigned long usec_total_comp = 0;
	unsigned long usec_total_comm = 0;
	
	float *matrix; //solo lo necesita p0
	float *vector = malloc (sizeof (float) * N); //lo necesitan todos
	float *result; //solo lo necesita p0
	
	//Variables e inicialización del entorno MPI
	int rows;
	int numprocs, rank;
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	  
	//Arrays de counts y displs para Scatterv y Gatherv
	int *countss;
	int *displss;
	int *countsg;
	int *displsg;
	
	rows = 	(N/numprocs)+((rank<N%numprocs?1:0));
  
	float *submatrix = malloc(rows * N * sizeof(float));
	float *subresult = malloc(rows * sizeof(float));
	
	if(rank==0){
	
		matrix = malloc (sizeof (float) * N * N);
		result = malloc (sizeof (float) * N);
		countss = malloc(numprocs * sizeof(int));
		displss = malloc(numprocs * sizeof(int));
		countsg = malloc(numprocs * sizeof(int));
		displsg = malloc(numprocs * sizeof(int));
		
		/* Initialize Matrix and Vector */
		for(f=0;f<N;f++) {
			vector[f] = f;
			for(c=0;c<N;c++) {
				*(matrix + (f*N) + c) = f+c;
			}
		}
	 
		if (DEBUG){
			//Mostramos la matriz generada
			printf("Soy el proceso %d y la matriz que generé es:\n",rank);
			for(f=0;f<N;f++){
				for(c=0;c<N;c++){
					printf("%f ",*(matrix +(f*N) + c));
				}
				printf("\n");
			}
			printf(".....................................................\n");
			printf("\n");
		}
		
		//inicializo los valores de los arrays de gather y scatter
	for(i=0; i<numprocs; i++){
		countss[i] = ((N/numprocs)+((i<N%numprocs?1:0)))*N;
		countsg[i] = (N/numprocs)+((i<N%numprocs?1:0));
	}
	displss[0] = 0;
	displsg[0] = 0;
	for(i=1; i<numprocs; i++){
		displss[i] = displss[i-1]+countss[i-1];
		displsg[i] = displsg[i-1]+countsg[i-1];
	}
				
		
	}
	//Antes de comenzar la difusión del vector y el reparto de la matriz
	gettimeofday(&tv1, NULL);
	//Repartimos la matriz.
	MPI_Scatterv(matrix,countss,displss,MPI_FLOAT,submatrix,(rows * N),MPI_FLOAT,0,MPI_COMM_WORLD);
	//Difundimos el vector
	MPI_Bcast(vector,N,MPI_FLOAT,0,MPI_COMM_WORLD);	
	//Después de finalizar la difusión del vector y el reparto de la matriz
	gettimeofday(&tv2, NULL);
	//Calculamos el tiempo (usec) de comunicaciones
	usec_comm = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
	
	
	
	if (DEBUG){
		//Mostramos el vector
		printf("Soy el proceso %d y el vector que recibi es:",rank);
		for(i=0;i<N;i++){
			printf("%f ",vector[i]);
		}
		printf("\n");
	} 
	
	if (DEBUG){
		//Mostramos la matriz recibida
		printf("Soy el proceso %d y la submatriz que recibi es:\n",rank);
		for(f=0;f<rows;f++){
			for(c=0;c<N;c++){
				printf("%f ",*(submatrix+(f*N)+c));
			}
			printf("\n");
		}
		printf("\n");
	}
	
	//Hacemos los cálculos que nos tocan
	//Obtenemos el tiempo antes de comenzar los cáculos
	gettimeofday(&tv3, NULL);
	for(f=0;f<rows;f++) {
		subresult[f]=0;
		for(c=0;c<N;c++) {
			subresult[f] += *(submatrix+(f*N)+c) * vector[c];			
		}
		
	}
	//Obtenemos el tiempo después de finalizar los cáculos
	gettimeofday(&tv4, NULL);
	//Calculamos el tiempo (usec) de computación
	usec_comp = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
	
	if (DEBUG){
		//Mostramos el vector subresult generado
		printf("El vector subresult generado por el proceso %d es: ", rank);
		for(i=0;i<rows;i++){
			printf("%f ", subresult[i]);
		}
		printf("\n");
	}
	
	//if (!DEBUG) printf ("El tiempo de computo para el proceso %d es %lf\n", rank, usec_comp/1E6);
	
	//Antes de comenzar la recolección de los resultados
	gettimeofday(&tv1, NULL);
	//Recolectamos los resultados
	MPI_Gatherv(subresult,rows,MPI_FLOAT,result,countsg,displsg, MPI_FLOAT,0,MPI_COMM_WORLD);

	//Después de finalizar la difusión del vector y el reparto de la matriz
	gettimeofday(&tv2, NULL);
	//Calculamos el tiempo (usec) de comunicaciones
	usec_comm += (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
	
	//Recopilamos los tiempos totales de computo y comunicaciones
	MPI_Reduce(&usec_comp,&usec_total_comp,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	MPI_Reduce(&usec_comm,&usec_total_comm,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	
	
	if(rank==0){
		/*Display result */
		if (DEBUG){
			printf("............................................\n");
			printf("El vector resultado de M x V es:\n");
			for(i=0;i<N;i++) {
				printf(" %f \t ",result[i]);
			}
			printf("\n");
		} else {
			printf ("Comp Time (seconds) = %lf\n", (double) usec_total_comp/1E6);
			printf ("Comm Time (seconds) = %lf\n", (double) usec_total_comm/1E6);
		}
		
		free (matrix);
		free (result);
		free (countss);
		free (countsg);
		free (displss);
		free (displsg);
	}
	
	free (vector);
	free (subresult);
	free (submatrix);
	MPI_Finalize();  

	return 0;
}
