#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv)
{
    int cpu_rank;
	int p;
	int n;
	
	int M, N;					//MxN network topology dimensions
	int dimensions[2];			//array with main dimensions as elements
	int periods[2]={0,0};		//array with values of 1 and 0 about periods in the network
	MPI_Comm cartcomm;			//network communicator
	MPI_Comm colcomm;
	MPI_Comm rowcomm;	
	
	int i, j;
	
	int *a;						//array with the elements
	int *local_a;				//array with the local elements
	int num_of_elements;		//number of elements each task will handle
	
	int local_sum=0;			//the sum of the elements each task handles
	int column_sum=0;			//the sum of the whole column stored in the top task of the column
	int total_sum=0;			//the total sum of the whole network
	
	int subgrid_dimensions[2]; 	//array with the dimensions of the subgrids

	MPI_Init(&argc, &argv);
		MPI_Comm_rank(MPI_COMM_WORLD, &cpu_rank);
		MPI_Comm_size(MPI_COMM_WORLD, &p);
	
		if(cpu_rank==0)
		{
			printf("Give me the number of rows for the network: ");
			scanf("%d",&M);
			
			printf("Give me the number of columns for the network: ");
			scanf("%d",&N);
		}
	
		MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);		//send the dimensions to all tasks
		MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
		dimensions[0]=M;	//initializing the dimensions array
		dimensions[1]=N;
		
		MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 0, &cartcomm);	//creation of a MxN network topology without any periods and tasks ordered by their ranks
		
		MPI_Comm_rank(cartcomm, &cpu_rank);					//setting the ranks to the network's communicator
		
		if(cpu_rank==0)
		{
			printf("Give me the number of the a[] elements: ");
			scanf("%d", &n);
		}
		
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);		//send the total number of elements to all tasks
		
		a=(int *)malloc(n*sizeof(int));
	
		if(cpu_rank==0)
		{
			for(i=0;i<n;i++)
			{
				printf("a[%d]= ", i);
				scanf("%d", &a[i]);
			}
		}
		
		num_of_elements=n/p;	
		
		local_a=(int *)malloc(num_of_elements*sizeof(int));	
		
		MPI_Scatter(a, num_of_elements, MPI_INT, local_a, num_of_elements, MPI_INT, 0, MPI_COMM_WORLD);	//scatter the a[] array elements evenly to the tasks
		
		for(i=0;i<num_of_elements;i++)
			local_sum+=local_a[i];
		
		/////////////////////////////////////
		//      column sum calculation	   //
		/////////////////////////////////////	
		subgrid_dimensions[0]=1;				//dimensions of column subgrids
		subgrid_dimensions[1]=0;
		
		MPI_Cart_sub(cartcomm, subgrid_dimensions, &colcomm);					//break the network into subgrids and connect it to colcomm communicator
		
		MPI_Comm_rank(colcomm, &cpu_rank);										//get cpu_rank as the rank for the colcom communicator
		
		MPI_Reduce(&local_sum, &column_sum, 1, MPI_INT, MPI_SUM, 0, colcomm);	//calculate the column sum and store it in the task at the top row
		/////////////////////////////////////
		/////////////////////////////////////
		/////////////////////////////////////	
			
		/////////////////////////////////////
		// top row (total) sum calculation //
		/////////////////////////////////////
		subgrid_dimensions[0]=0;				//dimensions of column subgrids
		subgrid_dimensions[1]=1;
		
		MPI_Cart_sub(cartcomm, subgrid_dimensions, &rowcomm);					//break the network into subgrids and connect it to rowcomm communicator
		
		MPI_Comm_rank(rowcomm, &cpu_rank);										//get cpu_rank as the rank for the rowcom communicator
			
		MPI_Reduce(&column_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, rowcomm);	//calculate the column sum and store it in the task with rank==0
		/////////////////////////////////////
		/////////////////////////////////////
		/////////////////////////////////////		
	
		MPI_Comm_rank(cartcomm, &cpu_rank);			//get cpu_rank as the rank for the cartcomm main network communicator
			
		if(cpu_rank==0)
			printf("Total Sum of the a[] array: %d\n", total_sum);

	MPI_Finalize();
	return 0;
}