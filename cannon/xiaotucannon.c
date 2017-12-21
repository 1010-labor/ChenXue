#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

int **A, **B, **C; //C=A*B
int *a,*b,*c; //各个进程的缓冲区
int n; //矩阵的行列数
int np; //每个进程控制的小矩阵的行列数
MPI_Status status;
int p,rank; //进程个个数、当前进程的编号,笛卡尔进程编号
int *tempa, *tempb;

void ProduceABC(); //在根处理器中随机生成矩阵AB，数值在0-10之间;初始化矩阵C
void PrintABC();//输出结果
void ScatterAB();// 分发矩阵AB中的元素到各个进程中
void MainProcess(); //cannon算法的主过程
void collectC(); //收集结果矩阵C
void Mutiply(); //矩阵相乘
void Printab();
void Printc();

int main(int argc, char *argv[])
{ 
int i;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &p);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
// printf( "Hello world from process %d of %d\n", rank, p );

if(rank == 0)
{
   printf("请输入矩阵的行列数n = "); fflush(stdout);
   scanf("%d", &n);
   printf("\n");
}
MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

// n = atoi(argv[1]);
np = n/(int)sqrt(p);

a = (int*)malloc(np*np*sizeof(int));
b = (int*)malloc(np*np*sizeof(int));
c = (int*)malloc(np*np*sizeof(int));
memset(c, 0, np*np*sizeof(int));
//Printc();
tempa = (int*)malloc(np*np*sizeof(int));
tempb = (int*)malloc(np*np*sizeof(int)); 

if(rank == 0)
{  
   //在根处理器中为矩阵ABC分配空间
   A = (int**)malloc(n*sizeof(int*));
   B = (int**)malloc(n*sizeof(int*));
   C = (int**)malloc(n*sizeof(int*));
  
   for(i = 0; i < n; i++)
   {
    A[i] = (int*)malloc(n*sizeof(int));
    B[i] = (int*)malloc(n*sizeof(int));
    C[i] = (int*)malloc(n*sizeof(int));
   }
   ProduceABC(); //在根处理器中随机生成矩阵AB，数值在0-10之间;初始化矩阵C
   ScatterAB();// 分发矩阵AB中的元素到各个进程中  
}
else
{
   MPI_Recv(a, np*np, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
   MPI_Recv(b, np*np, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
}

MainProcess(); //cannon算法的主过程

if(rank == 0)
{
   collectC(); //收集结果矩阵C
   PrintABC(); //输出结果

   for(i = 0; i < n; i++)
   {
    free(A[i]);
    free(B[i]);
    free(C[i]);
   }
   free(A);
   free(B);
   free(C);
}
else
{
   MPI_Send(c, np*np, MPI_INT, 0, 1, MPI_COMM_WORLD);
}

free(a);
free(b);
free(c);
free(tempa);
free(tempb);

//MPI_Barrier(MPI_COMM_WORLD);
MPI_Finalize();

return 0;
}


void ProduceABC()//在根处理器中随机生成矩阵AB
{ 
int i,j;
     srand((unsigned int)time(NULL));      /*设随机数种子*/

/*随机生成A,B,并初始化C*/
     for(i=0; i<n; i++)
{
   for(j=0; j<n; j++)
   {
    A[i][j] = (int)(rand()/(double)RAND_MAX * 10);
    B[i][j] = (int)(rand()/(double)RAND_MAX * 10);
    C[i][j] = 0;
   }
}
}

void PrintABC()//输出结果
{
int i,j;
printf("矩阵A如下:\n");
for(i=0;i<n;i++)
{
   for(j=0;j<n;j++)
    printf("%6d",A[i][j]);
   printf("\n");
}
printf("\n");

printf("矩阵B如下:\n");
for(i=0;i<n;i++)
{
   for(j=0;j<n;j++)
    printf("%6d",B[i][j]);
   printf("\n");
}
printf("\n");

printf("矩阵C如下:\n");
for(i=0;i<n;i++)
{
   for(j=0;j<n;j++)
    printf("%6d",C[i][j]);
   printf("\n");
}
printf("\n");     
}

void ScatterAB()// 分发矩阵AB中的元素到各个进程中
{
int imin,imax,jmin,jmax;
int sp;
int i,j,k,m;

for(k=0; k<p; k++)
{
   /*计算相应处理器所分得的矩阵块在总矩阵中的坐标范围*/
   sp = (int)sqrt(p);
   imin = (k / sp) * np;
   imax = imin + np - 1;  
   jmin = (k % sp) * np;
   jmax = jmin + np -1;
   
   /*rank=0的处理器将A,B中的相应块拷至tempa,tempb，准备向其他处理器发送*/
   m = 0; 
   for(i=imin; i<=imax; i++)
   {
    for(j=jmin; j<=jmax; j++)
    {
     tempa[m] = A[i][j];
     tempb[m] = B[i][j];
     m++;
    }
   }
  
   /*根处理器将自己对应的矩阵块从tempa,tempb拷至a,b*/
   if(k==0)
   {
    memcpy(a, tempa, np*np*sizeof(int));
    memcpy(b, tempb, np*np*sizeof(int));
   } 
   else    /*根处理器向其他处理器发送tempa,tempb中相关的矩阵块*/
   {
    MPI_Send(tempa, np*np, MPI_INT, k, 1, MPI_COMM_WORLD);
    MPI_Send(tempb, np*np, MPI_INT, k, 2, MPI_COMM_WORLD);
   }
}
}

void MainProcess() //cannon算法的主过程
{
MPI_Comm comm; //笛卡尔结构通讯器
int crank;
int dims[2],periods[2], coords[2];
int source, dest, up, down, right, left;
int i;

dims[0] = dims[1] = (int)sqrt(p);
periods[0] = periods[1] = 1;
MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm);

MPI_Comm_rank(comm, &crank);
MPI_Cart_coords(comm, crank, 2, coords);

MPI_Cart_shift(comm, 1, -1, &right, &left);
// printf("crank = %d right = %d  left = %d\n", crank, right, left);
MPI_Cart_shift(comm, 0, -1, &down, &up);
// printf("crank = %d up = %d  down = %d\n", crank, up, down);

MPI_Cart_shift(comm, 1, -coords[0], &source, &dest);
// printf("crank = %d source = %d  dest = %d\n", crank, source, dest);
MPI_Sendrecv_replace(a, np*np, MPI_INT, dest, 1, source, 1, comm, &status);

MPI_Cart_shift(comm, 0, -coords[1], &source, &dest);
MPI_Sendrecv_replace(b, np*np, MPI_INT, dest, 1, source, 1, comm, &status);

Mutiply(); //矩阵相乘
// printf("rank = %d\n", rank);Printab();Printc();

for(i = 1; i < dims[0]; i++)
{
   MPI_Sendrecv_replace(a, np*np, MPI_INT, left, 1, right, 1, comm, &status);
   MPI_Sendrecv_replace(b, np*np, MPI_INT, up, 1, down, 1, comm, &status);

   Mutiply(); //矩阵相乘
//   printf("i = %d  rank = %d\n",i, rank);Printab();Printc();
}

MPI_Comm_free(&comm);
}
void collectC() //收集结果矩阵C
{
int i,j,k,s,m;
int imin,imax,jmin,jmax;
int sp= (int)sqrt(p);

/* 根处理器中的c赋给总矩阵C */
for (i=0;i<np;i++)
{
   for(j=0;j<np;j++)
    C[i][j]=c[i*np+j];
}
for (k=1;k<p;k++)
{
   /*根处理器从其他处理器接收相应的分块c*/
   MPI_Recv(c, np*np, MPI_INT, k, 1, MPI_COMM_WORLD, &status);
//   printf("rank = %d\n", k);Printc();

   imin = (k / sp) * np;
   imax = imin + np - 1;  
   jmin = (k % sp) * np;
   jmax = jmin + np -1;
    
   /*将接收到的c拷至C中的相应位置,从而构造出C*/
   for(i=imin,m=0; i<=imax; i++,m++)
   {
    for(j=jmin,s=0; j<=jmax; j++,s++)
     C[i][j]=c[m*np+s];
   }
} 
}

void Mutiply() //矩阵相乘
{
int i,j,k;
for(i=0; i<np; i++)
        for(j=0; j<np; j++)
          for(k=0; k<np; k++)
            c[i*np+j] += a[i*np+k]*b[k*np+j];
}

void Printab()//输出结果
{
int i;
printf("矩阵a如下:\n");
for(i=0;i<np*np;i++)
   printf("%6d",a[i]);
printf("\n");

printf("\n");

printf("矩阵b如下:\n");
for(i=0;i<np*np;i++)
   printf("%6d",b[i]);
printf("\n");
printf("\n");
}

void Printc()
{
int i;
printf("矩阵c如下:\n");
for(i=0;i<np*np;i++)
   printf("%6d",c[i]);
printf("\n");
printf("\n");
}