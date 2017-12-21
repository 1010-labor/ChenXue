// yi wei array fang shi pao tong
// the data of array are from file
// rows=cols and form keyboard input
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
   /* int rowM;//shu zu de wei du
    int colM;
    int rowN;
    int colN;*/
    int width;
    int myid,size,arrayid;
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
        printf("please input the width of array:\n");
        scanf("%d",&width);
      /*  printf("please input the cols of array M:\n");
        scanf("%d",&colM);
     //   MPI_Bcast(&rowM,1,MPI_INT,0,MPI_COMM_WORLD);
     //   MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
        printf("please input the rows of array N:\n");
        scanf("%d",&rowN);
        printf("please input the cols of array N:\n");
        scanf("%d",&colN);*/
       /* if(colM!=rowN)// error chick
        {
             printf("please input some colM and rowN!\n");
             return 0;
        }*/
        startTime=MPI_Wtime();
        MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
       /* MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);*/

        int line=width/size;
        M=(float*)malloc(sizeof(float)*width*width);//the first array
        N=(float*)malloc(sizeof(float)*width*width);//the second array
        P=(float*)malloc(sizeof(float)*width*width);//the result array
        buffer=(float*)malloc(sizeof(float)*width*line);
        ans=(float*)malloc(sizeof(float)*width*line);

       /* M=(float*)malloc(sizeof(float)*width*width);//the first array
        N=(float*)malloc(sizeof(float)*width*width);//the second array
        P=(float*)malloc(sizeof(float)*width*width);//the result array
        buffer=(float*)malloc(sizeof(float)*width*line);
        ans=(float*)malloc(sizeof(float)*width*line);*/
    
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
      /*  fprintf(stdout,"The data of N is:\n");
        for(i=0;i<rowN;i++)
        {
             for(j=0;j<colN;j++)
                 printf("%f   ",N[i*colN+j]);
             printf("\n");
             fflush(stdout);
        }*/
   
        startComTime=MPI_Wtime();
        MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD);//root bcast N to other
        for(m=1;m<size;m++)//root bcast M to other
        {
            MPI_Send(M+(m-1)*line*width,width*line,MPI_FLOAT,m,1,MPI_COMM_WORLD);
        }
        for(k=1;k<size;k++)
        {
            MPI_Recv(ans,line*width,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);
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
        free(M);
        free(P);      
        endTime=MPI_Wtime();
        totalTime=endTime-startTime;
        inputTime=endInputTime-startInputTime;
        comTime=endComTime-startComTime;
        outputTime=endOutputTime-startOutputTime;
        printf("the total time is:%.4fs\n",totalTime);
        printf("input time is:%.4fs\n",inputTime);
        printf("compute time is:%.4fs\n",comTime);
        printf("output time is:%.4fs\n",outputTime);
        
      }
        else
        {
             
             int i,j,k;
             MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
          /*   MPI_Bcast(&colM,1,MPI_INT,0,MPI_COMM_WORLD);
             MPI_Bcast(&rowN,1,MPI_INT,0,MPI_COMM_WORLD);
             MPI_Bcast(&colN,1,MPI_INT,0,MPI_COMM_WORLD);*/
              int line=width/size;
         //    M=(float*)malloc(sizeof(float)*rowM*colM);//the first array
             N=(float*)malloc(sizeof(float)*width*width);//the second array
           //  P=(float*)malloc(sizeof(float)*rowM*colN);//the result array
             buffer=(float*)malloc(sizeof(float)*width*line);
             ans=(float*)malloc(sizeof(float)*width*line);

             MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD);             
            // sleep(5);
           /*  printf("the zi jin cheng %d receive data N is:\n",myid);
             for(i=0;i<rowN;i++)
             {
                 for(j=0;j<colN;j++)
                      printf("%f   ",N[i*colN+j]);
                 printf("\n");
                 fflush(stdout);
             } */ 
    
             MPI_Recv(buffer,width*line,MPI_FLOAT,0,1,MPI_COMM_WORLD,&status);
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
                  for(j=0;j<width;j++)
                  {
                     float temp=0.0;
                     for(k=0;k<width;k++)
                         temp+=buffer[i*width+k]*N[k*width+j];
                     ans[i*width+j]=temp;
                  }
             }
             MPI_Send(ans,line*width,MPI_FLOAT,0,3,MPI_COMM_WORLD);
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
