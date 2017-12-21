// yi wei array fang shi
// the data of cannon are form file
// fen hang and feng lei
//right one 
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"mpi.h"

void getcann(float *a,int rows,int cols,int arrayid)
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
      for(i=0;i<rows;i++)
      {
           for(j=0;j<cols;j++)
           {
               fscanf(fin,"%f",&temp);
               a[i*cols+j]=temp;
           }
      }
      fclose(fin);
}

int gcd(int M,int N,int size)
{
     int i;
     double q;
     q=sqrt((double)size);
     
     for(i=M;i>0;i--)
     {
         if((M%i==0)&&(N%i==0)&&(i<=q))
              return i;
     }
}

int main(int argc,char* argv[])
{
     float *M,*N,*P,*bufferA,*bufferB,*ans;
    /* int rowM;
     int colM;
     int rowN;
     int colN;*/
     int width;
     int myid,size,arrayid,i,j,numpack,m,n,numproc;
     int R,L;
     double startTime,endTime,totalTime;
     double startInputTime,endInputTime,inputTime;
     double startComTime,endComTime,comTime;
     double startOutputTime,endOutputTime,outputTime;
     MPI_Status status;
     int namelen;
     char processor_name[MPI_MAX_PROCESSOR_NAME];
    
     MPI_Init(&argc,&argv);
     MPI_Comm_size(MPI_COMM_WORLD,&size);
     MPI_Comm_rank(MPI_COMM_WORLD,&myid);
     MPI_Get_processor_name(processor_name,&namelen);
     printf("Process %d of %d on %s\n",myid,size,processor_name);
     fflush(stdout);
     
  /*   numpack=gcd(rowM,colN,size);
     printf("the numpack is %d\n",numpack);
     numproc=numpack*numpack;*/
     if(myid==0)
     {
          float *tempN;
          printf("please input the width of array:\n");
          scanf("%d",&width);
      /*    printf("please input the cols of array M:\n");
          scanf("%d",&colM);
          printf("please input the rows of array N:\n");
          scanf("%d",&rowN);
          printf("please input the cols of array N:\n");
          scanf("%d",&colN);
          if(colM!=rowN)// error chick
          {
                printf("please input some colM and rowN!\n");
                return 0;
           }*/
          startTime=MPI_Wtime();
          numpack=gcd(width,width,size);
          printf("the numpack is %d\n",numpack);
          numproc=numpack*numpack;
          m=width/numpack;//number of row each M de kuai
          n=width/numpack;//number of col each N de kuai
                          
          MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
      /*    MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
          MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
          MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);*/
          MPI_Bcast(&numpack,1,MPI_INT,0,MPI_COMM_WORLD);
          
          M=(float*)malloc(sizeof(float)*width*width);//the first array
          N=(float*)malloc(sizeof(float)*width*width);//the second array
          P=(float*)malloc(sizeof(float)*width*width);//the result array
          bufferA=(float*)malloc(sizeof(float)*m*width);
          bufferB=(float*)malloc(sizeof(float)*width*n);
          ans=(float*)malloc(sizeof(float)*m*n);
          tempN=(float*)malloc(sizeof(float)*width*n);          

          startInputTime=MPI_Wtime();
          getcann(M,width,width,1);
         /* fprintf(stdout,"The data of M is:\n");
          fflush(stdout);
          for(i=0;i<rowM;i++)
          {
               for(j=0;j<colM;j++)
                    printf("%f   ",M[i*colM+j]);
               printf("\n");
               fflush(stdout);
          }*/

          getcann(N,width,width,2);
          endInputTime=MPI_Wtime();
         /* fprintf(stdout,"The data of N is:\n");
          for(i=0;i<rowN;i++)
          {
               for(j=0;j<colN;j++)
                   printf("%f   ",N[i*colN+j]);
               printf("\n");
               fflush(stdout);
          }*/
          startComTime=MPI_Wtime();
          for(i=0;i<m;i++)
               for(j=0;j<width;j++)
                    bufferA[i*width+j]=M[i*width+j];
          for(i=0;i<width;i++)
               for(j=0;j<n;j++)
                    bufferB[i*n+j]=N[i*width+j];
         
          for(i=1;i<numproc;i++) // send data to other
          {    
               int rn,cn;
               R=i/numpack;
               L=i%numpack;
               MPI_Send(&M[R*m*width],m*width,MPI_FLOAT,i,i,MPI_COMM_WORLD);
               for(rn=0;rn<width;rn++)
                    for(cn=0;cn<n;cn++)
                        tempN[rn*n+cn]=N[rn*width+L*n+cn];
             //  for(j=0;j<rowN;j++)
            /*  printf("the processor %d to zi jin cheng %d receive data N is:\n",myid,i);
              for(rn=0;rn<rowN;rn++)
              {
                   for(cn=0;cn<n;cn++)
                        printf("%f   ",tempN[rn*n+cn]);
                   printf("\n");
                   fflush(stdout);
              }*/

                MPI_Send(tempN,width*n,MPI_FLOAT,i,i,MPI_COMM_WORLD);
          }
          
          for(i=0;i<m;i++)
           {
                for(j=0;j<n;j++)
                {
                      int  temp=0;
                      int q;
                      for(q=0;q<width;q++)
                          temp+=bufferA[i*width+q]*bufferB[q*n+j];
                      ans[i*n+j]=temp;
                }
           }
          
          for(i=0;i<m;i++)
          {
               for(j=0;j<n;j++)
               {
                   P[i*numpack*n+j]=ans[i*n+j];
               }
          }
          int k;
          for(k=1;k<numproc;k++)
          {
              MPI_Recv(ans,m*n,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);
            /*  printf("the reslut line %d from processor %d to processor %d is:\n",k,k,myid);
              for(i=0;i<m;i++)
              {
                  for(j=0;j<n;j++)
                      printf("%f   ",ans[i*n+j]);
                  printf("\n");
                  fflush(stdout);
              }*/
              R=k/numpack;
              L=k%numpack;
              for(i=0;i<m;i++)//ans --> P
              {
                  for(j=0;j<n;j++)
                  {
                      P[(R*m+i)*numpack*n+(L*n+j)]=ans[i*n+j];
                  }
              }
          }
          endComTime=MPI_Wtime();

          startOutputTime=MPI_Wtime();
          printf("the result is:\n");
          for(i=0;i<width;i++)//result output
          {
              for(j=0;j<width;j++)
                   printf("%f   ",P[i*width+j]);
               printf("\n");
               fflush(stdout);
           }
          endOutputTime=MPI_Wtime();
          endTime=MPI_Wtime();
        
          totalTime=endTime-startTime;
        //  printf("total time is:%.4fs\n",totalTime);
          inputTime=endInputTime-startInputTime;
          comTime=endComTime-startComTime;
          outputTime=endOutputTime-startOutputTime;
          printf("the total time is:%.4fs\n",totalTime);
          printf("input time is:%.4fs\n",inputTime);
          printf("compute time is:%.4fs\n",comTime);
          printf("output time is:%.4fs\n",outputTime);

                   
          free(M);
          free(N);
          free(P);
          free(tempN);
          free(bufferA);
          free(bufferB);
          free(ans);
         /* endTime=MPI_Wtime();
          totalTime=endTime-startTime;
          printf("total time is:%.4fs\n",totalTime);*/
      }

      else //zi jin cheng
      {
           MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
          /* MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
          MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
           MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);*/
           MPI_Bcast(&numpack,1,MPI_INT,0,MPI_COMM_WORLD);
           
           numproc=numpack*numpack;
           m=width/numpack;//number of row each M de kuai
           n=width/numpack;

        if(myid<numproc)
        {
           bufferA=(float*)malloc(sizeof(float)*m*width);
           bufferB=(float*)malloc(sizeof(float)*width*n);
           ans=(float*)malloc(sizeof(float)*m*n);
           printf("I'm processor %d\n",myid);           
           MPI_Recv(bufferA,m*width,MPI_FLOAT,0,myid,MPI_COMM_WORLD,&status);
         /*  printf("the zi jin cheng %d receive data M line is\n",myid);
           for(i=0;i<m;i++)
           {
               for(j=0;j<colM;j++)
                   printf("%f   ",bufferA[i*colM+j]);
               printf("\n");
               fflush(stdout);
           }*/

           MPI_Recv(bufferB,width*n,MPI_FLOAT,0,myid,MPI_COMM_WORLD,&status);
         /*  printf("the zi jin cheng %d receive data N is:\n",myid);
           for(i=0;i<rowN;i++)
           {
                for(j=0;j<n;j++)
                     printf("%f   ",bufferB[i*n+j]);
                printf("\n");
                fflush(stdout);
           }*/

           for(i=0;i<m;i++)
           {
                for(j=0;j<n;j++)
                {
                      int temp,q;
                      temp=0;
                      for(q=0;q<width;q++)
                          temp+=bufferA[i*width+q]*bufferB[q*n+j];
                      ans[i*n+j]=temp; 
                }
           }
           MPI_Send(ans,m*n,MPI_FLOAT,0,3,MPI_COMM_WORLD);
         /*  printf("the result of processor %d is:\n",myid);
           for(i=0;i<m;i++)
           {
               for(j=0;j<n;j++)
                   printf("%f\t",ans[i*n+j]);
               printf("\n");
               fflush(stdout);
           }*/
           free(bufferA);
           free(bufferB);
           free(ans);
       }
         
     }
  
       MPI_Finalize();
    //   return 0;
}


           






     
