//fenhang 
// rows=cols and form keyboard input
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"mpi.h"

void getcann(float *a,int rows,int cols,int arrayid) //从文件获取输入数据
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
           fscanf(fin,"%f",&temp);//read a int from file
           a[i*cols+j]=temp;
        }
    }
    fclose(fin);
}

int main(int argc,char *argv[])
{
    float *M,*N,*P,*buffer,*ans;
    int width;
    int myid,size,arrayid;
    double startTime,endTime,totalTime;//各种测试所用的时间变量
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

    if(myid==0)
    {
        int m,k,i,j,n,q,h;
        printf("please input the width of array:\n"); //从键盘输入矩阵的宽度
        scanf("%d",&width);
        startTime=MPI_Wtime();//开始时间
        MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);//第一次通信广播

        int line=width/size; //每一块的行数
		M=(float*)malloc(sizeof(float)*width*width);//the first array   分配存储空间
        N=(float*)malloc(sizeof(float)*width*width);//the second array
        P=(float*)malloc(sizeof(float)*width*width);//the result array
        buffer=(float*)malloc(sizeof(float)*width*line);
        ans=(float*)malloc(sizeof(float)*width*line);
    
        startInputTime=MPI_Wtime();
        getcann(M,width,width,1);
            
        getcann(N,width,width,2);
        endInputTime=MPI_Wtime();
/*--------------------------------------------------从下边开始是计算时间-----------------------------------------------------------------------*/    
        startComTime=MPI_Wtime(); //开始计算时间
        MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD);//root bcast N to other，先把N广播出去
        for(m=1;m<size;m++)//root bcast M to other
        {
            MPI_Send(M+(m-1)*line*width,width*line,MPI_FLOAT,m,1,MPI_COMM_WORLD); //再把M的各个行块使用send分发出去，0号进程本身的不动
        }
        for(k=1;k<size;k++)
        {
            MPI_Recv(ans,line*width,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);//结果回收，把其它进程的结果收回来
         
            for(n=0;n<line;n++)//ans --> P
            {  
                 for(q=0;q<width;q++)
                 {
                     P[((k-1)*line+n)*width+q]=ans[n*width+q];//把回收的结果放到该放的地方去
                 }
            }
        }
       for(i=(size-1)*line;i<width;i++)//the rest part 0号进程所负责部分的计算
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
/*------------------------------------------------到这里结束计算时间--------------------------------------------------------------------------*/      
        startOutputTime=MPI_Wtime();
        printf("the result is:\n");
        for(i=0;i<width;i++)//result output结果输出
        {
             for(j=0;j<width;j++)
                 printf("%f   ",P[i*width+j]);
             printf("\n");
             fflush(stdout);
        }
        endOutputTime=MPI_Wtime();
        free(M);
        free(P);      
        endTime=MPI_Wtime();//结束计算时间
        totalTime=endTime-startTime;
        inputTime=endInputTime-startInputTime;
        comTime=endComTime-startComTime;
        outputTime=endOutputTime-startOutputTime;
        printf("the total time is:%.4fs\n",totalTime);
        printf("input time is:%.4fs\n",inputTime);
        printf("compute time is:%.4fs\n",comTime);
        printf("output time is:%.4fs\n",outputTime);
        
      }
        else//其它进程
        {
             
             int i,j,k;
             MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);//接受广播来的矩阵宽度
             int line=width/size;
             N=(float*)malloc(sizeof(float)*width*width);//the second array          
             buffer=(float*)malloc(sizeof(float)*width*line);
             ans=(float*)malloc(sizeof(float)*width*line);

             MPI_Bcast(N,width*width,MPI_FLOAT,0,MPI_COMM_WORLD); //接受广播来的矩阵N                   
             MPI_Recv(buffer,width*line,MPI_FLOAT,0,1,MPI_COMM_WORLD,&status);//接受广播来的矩阵M的一个行块，放到buffer里边去
           
             for(i=0;i<line;i++)//计算开始
             {
                  for(j=0;j<width;j++)
                  {
                     float temp=0.0;
                     for(k=0;k<width;k++)
                         temp+=buffer[i*width+k]*N[k*width+j];
                     ans[i*width+j]=temp;
                  }
             } //计算结束
             MPI_Send(ans,line*width,MPI_FLOAT,0,3,MPI_COMM_WORLD);//将结果返回给0号进程
           
          }
         free(N);
         free(buffer);
         free(ans);
         MPI_Finalize();
         return 0;
    }
