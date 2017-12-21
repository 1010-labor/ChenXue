// yi wei array fang shi pao tong
// the data of array are from file
// rows!=cols and form keyboard input
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
           // scanf("%f",&a[i*width+j]);
          // fread(&temp,sizeof(int),1,fin);
           fscanf(fin,"%f",&temp);//read a int from file
           a[i*cols+j]=temp;
        }
    }
    fclose(fin);
}

int main(int argc,char *argv[])
{
    float *M,*N,*P,*buffer,*ans;
    int rowM;//shu zu de wei du
    int colM;
    int rowN;
    int colN;
    int myid,size,arrayid;
    double startTime,endTime,totalTime;
    MPI_Status status;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];    
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    printf("Process %d of %d on %s\n",myid,size,processor_name);    
    fflush(stdout);    

/*    int line=rowM/size;
    M=(float*)malloc(sizeof(float)*rowM*colM);//the first array
    N=(float*)malloc(sizeof(float)*rowN*colN);//the second array
    P=(float*)malloc(sizeof(float)*rowM*colN);//the result array
    buffer=(float*)malloc(sizeof(float)*colM*line);
    ans=(float*)malloc(sizeof(float)*rowN*line);//the zi jin cheng jresult array
    */
    if(myid==0)
    {
        int m,k,i,j,n,q,h;
        printf("please input the rows of array M:\n");
        scanf("%d",&rowM);
        printf("please input the cols of array M:\n");
        scanf("%d",&colM);
     //   MPI_Bcast(&rowM,1,MPI_INT,0,MPI_COMM_WORLD);
     //   MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
        printf("please input the rows of array N:\n");
        scanf("%d",&rowN);
        printf("please input the cols of array N:\n");
        scanf("%d",&colN);
        if(colM!=rowN)// error chick
        {
             printf("please input some colM and rowN!\n");
             return 0;
        }
        startTime=MPI_Wtime();
        MPI_Bcast(&rowM,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);

        int line=rowM/size;
        M=(float*)malloc(sizeof(float)*rowM*colM);//the first array
        N=(float*)malloc(sizeof(float)*rowN*colN);//the second array
        P=(float*)malloc(sizeof(float)*rowM*colN);//the result array
        buffer=(float*)malloc(sizeof(float)*colM*line);
        ans=(float*)malloc(sizeof(float)*colN*line);

       /* M=(float*)malloc(sizeof(float)*width*width);//the first array
        N=(float*)malloc(sizeof(float)*width*width);//the second array
        P=(float*)malloc(sizeof(float)*width*width);//the result array
        buffer=(float*)malloc(sizeof(float)*width*line);
        ans=(float*)malloc(sizeof(float)*width*line);*/

        getcann(M,rowM,colM,1);
       /* fprintf(stdout,"The data of M is:\n");
        fflush(stdout);
        for(i=0;i<rowM;i++)
        {
             for(j=0;j<colM;j++)
                 printf("%f   ",M[i*colM+j]);
             printf("\n");
             fflush(stdout);
        }*/
            
        getcann(N,rowN,colN,2);
      /*  fprintf(stdout,"The data of N is:\n");
        for(i=0;i<rowN;i++)
        {
             for(j=0;j<colN;j++)
                 printf("%f   ",N[i*colN+j]);
             printf("\n");
             fflush(stdout);
        }*/

        MPI_Bcast(N,rowN*colN,MPI_FLOAT,0,MPI_COMM_WORLD);//root bcast N to other
        for(m=1;m<size;m++)//root bcast M to other
        {
            MPI_Send(M+(m-1)*line*colM,colM*line,MPI_FLOAT,m,1,MPI_COMM_WORLD);
        }
        for(k=1;k<size;k++)
        {
            MPI_Recv(ans,line*colN,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);
           /* printf("the reslut line %d from processor %d to processor %d is:\n",k,k,myid);
            for(i=0;i<line;i++)
             {    
                for(j=0;j<colN;j++)
                    printf("%f   ",ans[i*colN+j]);
                printf("\n");      
                fflush(stdout);
             }*/

            for(n=0;n<line;n++)//ans --> P
            {  
                 for(q=0;q<colN;q++)
                 {
                     P[((k-1)*line+n)*colN+q]=ans[n*colN+q];
                 }
            }
        }
       for(i=(size-1)*line;i<rowM;i++)//the rest part 
       {
           for(h=0;h<colN;h++)
           {
               float temp=0.0;
               for(k=0;k<colM;k++)
                   temp+=M[i*colM+k]*N[k*colN+h];
               P[i*colN+h]=temp;
           }
       }
                 
      
        printf("the result is:\n");
        for(i=0;i<rowM;i++)//result output
        {
             for(j=0;j<colN;j++)
                 printf("%f   ",P[i*colN+j]);
             printf("\n");
             fflush(stdout);
        }
        free(M);
        free(P);      
        endTime=MPI_Wtime();
        totalTime=endTime-startTime;
        printf("the total time is:%.4fs\n",totalTime);
        
      }
        else
        {
             
             int i,j,k;
             MPI_Bcast(&rowM,1,MPI_INT,0,MPI_COMM_WORLD);
             MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
             MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
             MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);
              int line=rowM/size;
         //    M=(float*)malloc(sizeof(float)*rowM*colM);//the first array
             N=(float*)malloc(sizeof(float)*rowN*colN);//the second array
           //  P=(float*)malloc(sizeof(float)*rowM*colN);//the result array
             buffer=(float*)malloc(sizeof(float)*colM*line);
             ans=(float*)malloc(sizeof(float)*rowN*line);

             MPI_Bcast(N,rowN*colN,MPI_FLOAT,0,MPI_COMM_WORLD);             
            // sleep(5);
           /*  printf("the zi jin cheng %d receive data N is:\n",myid);
             for(i=0;i<rowN;i++)
             {
                 for(j=0;j<colN;j++)
                      printf("%f   ",N[i*colN+j]);
                 printf("\n");
                 fflush(stdout);
             } */ 
    
             MPI_Recv(buffer,colM*line,MPI_FLOAT,0,1,MPI_COMM_WORLD,&status);
           /*  printf("the zi jin cheng %d receive data M line is\n",myid);
             for(i=0;i<line;i++)
             {
                for(j=0;j<colM;j++)
                    printf("%f   ",buffer[i*colM+j]);  
                printf("\n");   
                fflush(stdout);      
             }*/
             for(i=0;i<line;i++)
             {
                  for(j=0;j<colN;j++)
                  {
                     float temp=0.0;
                     for(k=0;k<colM;k++)
                         temp+=buffer[i*colM+k]*N[k*colN+j];
                     ans[i*colN+j]=temp;
                  }
             }
             MPI_Send(ans,line*colN,MPI_FLOAT,0,3,MPI_COMM_WORLD);
            /* printf("the result of processor %d is:\n",myid);
             for(i=0;i<line;i++)
             {    
                for(j=0;j<colN;j++)
                    printf("%f   ",ans[i*colN+j]);
                printf("\n");      
                fflush(stdout);
             }*/

          }
         free(N);
         free(buffer);
         free(ans);
         MPI_Finalize();
         return 0;
    }
