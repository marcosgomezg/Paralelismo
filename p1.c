#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	
    int i, done = 0, n, count;
    double PI25DT = 3.141592653589793238462643;
    double pi, x, y, z;
    
    
    int rank ;
    int num_procs ;
    MPI_Status status;
    int count_aux;
	
	MPI_Init (&argc,&argv); //iniciando procesos que estan sincronizados
	MPI_Comm_size (MPI_COMM_WORLD, &num_procs) ; // para saber el numero de procesos, que estara en la variable num_procs
	MPI_Comm_rank (MPI_COMM_WORLD, &rank); //asocia cada proceso a un identificador
	
    while (!done)

    {
    	if (rank == 0) { //solo pide datos el primero
        printf("Enter the number of points: (0 quits) \n");
        scanf("%d",&n);
    
        
        //Distribuir n a todos los procesos, lo hace el proceso 0
        
       		 for (int j = 1; j < num_procs ; j++) { 
       			 	MPI_Send (&n, 1, MPI_INT, j ,0,MPI_COMM_WORLD);
       		 } 
       		 
        }
        
        else { 
        	
        	MPI_Recv (&n, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
        }
        
                
		if (n == 0) break;	
		
		printf("Soy el proceso %d y he recibido n=%d\n", rank, n);	
		
        count = 0; 
        
       
        for (i = rank ; i < n ; i+=num_procs) {
            // Get the random numbers between 0 and 1
       
	    x = ((double) rand()) / ((double) RAND_MAX);
	    y = ((double) rand()) / ((double) RAND_MAX);

	    // Calculate the square root of the squares
	    z = sqrt((x*x)+(y*y));

	    // Check whether z is within the circle
	    
	    if(z <= 1.0)
                count++;
        }
        
       
		if (rank == 0) { //el 0 printea
		
			count_aux = count ;
			
			for (int j = 1; j < num_procs; j++) {
				
				MPI_Recv (&count,1,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
				
				count_aux += count;
			}
			
			pi = ((double) count_aux/(double) n)*4.0;
			
        	printf("pi is approx. %.16f, Error is %.16f\n", pi , fabs(pi - PI25DT));
        }
        
        else {
        	
        	MPI_Send (&count,1,MPI_INT,0,1,MPI_COMM_WORLD);
        }
      	
    }
    
    MPI_Finalize () ; //los procesos dejan de colaborar y libera los recursos reservados
    
    return 0;
}
