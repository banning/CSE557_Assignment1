#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

// Split myList into leftList and rightList based on pivot
void Split (int *myList, int pivot, int *leftList, int *rightList, int myLength ,int &leftLength, int &rightLength)
{
	int index = 0;
	//find where pivot is
	if (myList[int(myLength/2)] > pivot)
	{
	   for (int i = int(myLength/2); i>=0; i--)
	   {

	      if (myList[i]<=pivot)
	      {
	         index=i;
	         break;
	      }
	   }
	}
	else
	{
	   for (int i = int(myLength/2); i <myLength; i++)
	   {
	      if (myList[i]>pivot)
	      {
	         index=i-1;
	         break;  
	      }
	   }
	}
	leftLength=index+1;
	rightLength=myLength-index-1;
	
	for (int i = 0; i < leftLength; i++)
	{
	   leftList[i]=myList[i];
	}
	for (int i = leftLength, j = 0; i < myLength; i++, j++)
	{
	   rightList[j]=myList[i];
	}
}

void Merge (int *myList, int *list1, int *list2, int length1, int length2)
{
	int i = 0;
	int j = 0;
	int k = 0;

	myList = new int[length1+length2];
   
	while (i != length1 && j != length2)
	{
      if (list1[i] > list2[j])
      {
         myList[k]=list2[j];
         k++;
         j++;
      }
      else
      {
         myList[k]=list1[i];
         k++;
         i++;
      }
	}
	//List one is done
	if (i==length1)
	{
	   for (;j <length2; j++, k++)
	   {
	      myList[k]=list2[j];
	   }
	}
	//List 2 is done
	else
	{
	   for (;i <length1; i++, k++)
	   {
	      myList[k]=list1[i];
	   }	
	}
}

void pqsort2 (int startprocessor, int endprocessor, int* myList, int listsize)
{
	int rank, size, pgroupsize, pdistance, midprocessor, pivot, partner;
	int buffer[1];
	int* leftList = new int[listsize];
	int* rightList = new int[listsize];
	int rightLength, leftLength;
	MPI_Status status;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	pgroupsize = endprocessor - startprocessor + 1;
	pdistance = pgroupsize/2;
	midprocessor = startprocessor + pdistance;
	
	if (pdistance >= 1)
	{
	   cout<<"Processor " << rank << " has the following list of length " <<listsize <<endl;
	   for (int i = 0; i < listsize; i++)
      {
         cout<<myList[i]<<'\t';
      }
      cout<<endl;
      
		// Calculate and broadcast pivot
		if (rank == startprocessor)
		{
			//pivot = MEDIAN of list
         	buffer[0]=myList[listsize/2];
         	
         	//Broadcast pivot to all processors in range
         	for (int j = startprocessor+1; j <= endprocessor; j++)
				MPI_Send(buffer, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
		}
		else
		{
			//Recieve listsize from start processor
			MPI_Recv(buffer, 1, MPI_INT, startprocessor, 0, MPI_COMM_WORLD, &status);
		}
		
		Split (myList, buffer[0], leftList, rightList, listsize, leftLength, rightLength);
		
		if (rank >= midprocessor)
		{
			partner = rank - pdistance;
			if (partner < 0)
				partner = endprocessor;
			
			//int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
             
            // Send length of leftList 
        	 buffer[0]=leftLength;
			MPI_Send(buffer, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
			// Send leftList
			MPI_Send(leftList, leftLength, MPI_INT, partner, 0, MPI_COMM_WORLD);
			
			//int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
			
			// Receive length of rightList

			MPI_Recv(buffer, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
 			rightLength=buffer[0];        
			// Receive rightList
			MPI_Recv(rightList, rightLength, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
			
			Merge (myList, leftList, rightList, leftLength, rightLength);
			pqsort2(midprocessor, endprocessor, myList, leftLength+rightLength);
		}
		else
		{
			partner = rank + pdistance;
			if (partner > size)
				partner = startprocessor;
			
			//int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
             
            // Send length of rightList 
         	buffer[0]=rightLength;
			MPI_Send(buffer, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
			// Send leftList
			MPI_Send(rightList, rightLength, MPI_INT, partner, 0, MPI_COMM_WORLD);
			
			//int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
			
			// Receive length of leftList
			
			MPI_Recv(buffer, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
         	leftLength=buffer[0];
			// Receive leftList
			MPI_Recv(leftList, leftLength, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
			
			Merge (myList, leftList, rightList, leftLength, rightLength);
			pqsort2(startprocessor, midprocessor-1, myList, leftLength+rightLength);
		}
	}
	else
	{
	   cout<<" In the end, Processor " << rank << "has the following list"<<endl;
	   for (int i = 0; i < listsize; i++)
      {
         cout<<myList[i]<<'\t';
      }
      cout<<endl;
	} 

}

// Implment quicksort, download from somewhere
// CITE: http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
void quick_sort (int *a, int n) {
    if (n < 2)
        return;
    int p = a[n / 2];
    int *l = a;
    int *r = a + n - 1;
    while (l <= r) {
        while (*l < p)
            l++;
        while (*r > p)
            r--;
        if (l <= r) {
            int t = *l;
            *l++ = *r;
            *r-- = t;
        }
    }
    quick_sort(a, r - a + 1);
    quick_sort(l, a + n - l);
}

// Implement HyperQuicksort
void HyperQuicksort(int listsize)
{
	int rank, size;
	MPI_Status status;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* I know the assignment says about sharing this, but I don't understand why since they can each
		start with the listsize
		
	// Check if lead processor
	if (rank == 0)
	{		
		//Broadcast listsize to all processors
		for (int i = 0; i <= size; i++)
			MPI_Send(listsize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	}
	else
	{
		//Recieve listsize from lead processor
		MPI_Recv(listsize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}
	*/
	
	// Intialize myList
	int* myList = new int[listsize/size];

	//flood myList with random numbers	
	srand ( time(NULL) );
	
	// generates the same first listsize/size number
	for (int i = 0; i < listsize/size; i++)
	{
	   myList[i]=rand()%1000 * (rank + 1);
	}
	
	int pivot = listsize/size^2;
	quick_sort(myList, pivot);
	string temp;

	pqsort2 (0, size-1, myList, int(listsize/size));
}


int main (int argc, char *argv[])
{
	int listsize;
	//get listsize from user inputline
	listsize = atoi(argv[1]);
	MPI_Init(&argc, &argv);
	HyperQuicksort(listsize);
	MPI_Finalize();
	return 0;
}