// yi wei array fangshi paotong,fang zhen
// data from keyboard input

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"mpi.h"

void getcann(float *a,int width)
{
    int i,j;
    fprintf(stdout,"please input your data:\n");
    for(i=0;i<width;i++)
    {
        for(j=0;j<width;j++)
        {
            scanf("%f",&a[i*width+j]);
        }
    }
}

int main(int argc,char *argv[])
{
    float *M,*N,*P,*buffer,*ans;
    int width=3;//shu zu de wei du
    int myid,size;
    MPI_Status status;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];    
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    printf("Process %d of %d on %s\n",myid,size,processor_name);    

    int line=width/size;
    M=(float*)malloc(sizeof(float)*width*width);//the first array
    N=(float*)malloc(sizeof(float)*width*width);//the second array
    P=(float*)malloc(sizeof(float)*width*width);//the result array
    buffer=(float*)malloc(sizeof(float)*width*line);
    ans=(float*)malloc(sizeof(float)*width*line);//the zi jin cheng jresult array
   
    if(myid==0)
    {
        int m,k,i,j,n,q,h;
        getcann(M,width);
        fprintf(stdout,"The data of M is:\n");
        for(i=0;i<width;i++)
        {
             for(j=0;j<width;j++)
                 printf("%f   ",M[i*width+j]);
             printf("\n");
        }
            
        getcann(N,width);
        fprintf(stdout,"The data of N is:\n");
        for(i=0;i<width;i++)
        {
             for(j=0;j<width;j++)
                 printf("%f   ",N[i*width+j]);
             printf("\n");
        }

        MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD);//root bcast N to other
        for(m=1;m<size;m++)//root bcast M to other
        {
            MPI_Send(M+(m-1)*line*width,width*line,MPI_FLOAT,m,1,MPI_COMM_WORLD);
        }
        for(k=1;k<size;k++)
        {
            MPI_Recv(ans,line*width,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);
            printf("the reslut line %d from processor %d to processor %d is:\n",k,k,myid);
            for(i=0;i<line;i++)
             {    
                for(j=0;j<width;j++)
                    printf("%f   ",ans[i*width+j]);
                printf("\n");      
             }

            for(n=0;n<line;n++)//ans --> P
            {  
                 for(q=0;q<width;q++)
                 {
                     P[((k-1)*line+n)*width+q]=ans[n*width+q];
                 }
            }
        }
       for(i=(size-1)*line;i<width;i++)//the rest part 
       {
           for(h=0;h<width;h++)
           {
               float temp=0.0;
               for(k=0;k<width;k++)
                   temp+=M[i*width+k]*N[k*width+h];
               P[i*width+h]=temp;
           }
       }
                 
      
        printf("the result is:\n");
        for(i=0;i<width;i++)//result output
        {
             for(j=0;j<width;j++)
                 printf("%f   ",P[i*width+j]);
             printf("\n");
        }      
      }
        else
        {
             
             int i,j,k;
             MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD);             
            // sleep(5);
             printf("the zi jin cheng %d receive data N is:\n",myid);
             for(i=0;i<width;i++)
             {
                 for(j=0;j<width;j++)
                      printf("%f   ",N[i*width+j]);
                 printf("\n");
             }  
    
             MPI_Recv(buffer,width*line,MPI_FLOAT,0,1,MPI_COMM_WORLD,&status);
             printf("the zi jin cheng %d receive data M line is\n",myid);
             for(i=0;i<line;i++)
             {
                for(j=0;j<width;j++)
                    printf("%f   ",buffer[i*width+j]);  
                printf("\n");         
             }
             for(i=0;i<line;i++)
             {
                  for(j=0;j<width;j++)
                  {
                     float temp=0.0;
                     for(k=0;k<width;k++)
                         temp+=buffer[i*width+k]*N[k*width+j];
                     ans[i*width+j]=temp;
                  }
             }
             MPI_Send(ans,line*width,MPI_FLOAT,0,3,MPI_COMM_WORLD);
             printf("the result of processor %d is:\n",myid);
             for(i=0;i<line;i++)
             {    
                for(j=0;j<width;j++)
                    printf("%f   ",ans[i*width+j]);
                printf("\n");      
             }

          }
         MPI_Finalize();
         return 0;
    }
