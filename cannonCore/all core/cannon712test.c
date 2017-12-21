//test for one processor input time 

#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

void getcann(float *a,int width,int arrayid)
{
      int i,j;
      float temp;
      FILE *fin;
      fprintf(stdout,"read data from file:\n");
      fflush(stdout);
      if(arrayid==1)
           fin=fopen("./a02","r+b");
      if(arrayid==2)
           fin=fopen("./d02","r+b");
      for(i=0;i<width;i++)
      {
           for(j=0;j<width;j++)
           {
               fscanf(fin,"%f",&temp);
               a[i*width+j]=temp;
           }
      }
      fclose(fin);
}

int main(int argc,char* argv[])
{
     float *M,*N,*P,*bufferA,*bufferB,*ans;
     int width;
     int myid,size,arrayid,i,j,numpack,m,n,numproc;
     int imin,imax,jmin,jmax;
     int R,L;
     double startTime,endTime,totalTime;
     double startInputTime,endInputTime,inputTime;
     double startComTime,endComTime,comTime;
     double startOutputTime,endOutputTime,outputTime;
     double startBarTime,endBarTime,barTime;
     MPI_Status status;
     int namelen;
     char processor_name[MPI_MAX_PROCESSOR_NAME];

     MPI_Init(&argc,&argv);
     MPI_Comm_size(MPI_COMM_WORLD,&size);
     MPI_Comm_rank(MPI_COMM_WORLD,&myid);
     MPI_Get_processor_name(processor_name,&namelen);
     printf("Process %d of %d on %s\n",myid,size,processor_name);
     fflush(stdout);

     if(myid==0)
     {
         printf("please input the width of array M:\n");
         scanf("%d",&width);  
         M=(float*)malloc(sizeof(float)*width*width);//the first array
         N=(float*)malloc(sizeof(float)*width*width);//the second array

         startInputTime=MPI_Wtime();
         getcann(M,width,1);
         getcann(N,width,2);
         endInputTime=MPI_Wtime();
   
         inputTime=endInputTime-startInputTime;
         printf("input time is:%.4fs\n",inputTime);

         startOutputTime=MPI_Wtime();
         printf("the result is:\n");
         for(i=0;i<width;i++)//result output
         {
             for(j=0;j<width;j++)
                  printf("%f   ",M[i*width+j]);
              printf("\n");
              fflush(stdout);
         }
         endOutputTime=MPI_Wtime();

         inputTime=endInputTime-startInputTime;
         printf("input time is:%.4fs\n",inputTime);
         outputTime=endOutputTime-startOutputTime;
         printf("output time is:%.4fs\n",outputTime);
         
         free(M);
         free(N);
     }

     MPI_Finalize();
     return 0;
}
