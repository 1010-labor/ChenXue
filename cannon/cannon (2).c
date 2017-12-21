#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/* 全局变量声明 */
float **A, **B, **C;              /* 总矩阵,C = A * B */
float *a, *b, *c, *tmp_a, *tmp_b; /* a、b、c表分块，tmp_a、tmp_b表缓冲区 */
int dg, dl, dl2,p, sp;            /* dg:总矩阵维数;dl:矩阵块维数;dl2=dl*dl;p:处理器个数;sp＝sqrt(p) */
int my_rank, my_row, my_col;      /* my_rank:处理器ID;(my_row,my_col):处理器逻辑阵列坐标 */
MPI_Status status;

/*
 *函数名: get_index
 *功能：处理器逻辑阵列坐标至rank号的转换
 *输入：坐标、逻辑阵列维数
 *输出：rank号
 */
int get_index(int row, int col, int sp)
{
   return ((row+sp)%sp)*sp + (col+sp)%sp;
}

/*
 *函数名：random_A_B
 *功能：随机生成矩阵A和B
 */
void random_A_B()
{
   int i,j;

    srand((unsigned int)time(NULL));     /*设随机数种子*/

    /*随机生成A,B,并初始化C*/
    for(i=0; i<dg ; i++)
      for(j=0; j<dg ; j++)
      {
        A[i][j] = rand();
        B[i][j] = rand();
        C[i][j] = 0.0;
      }
}

/* 函数名：scatter_A_B
 * 功能：rank为0的处理器向其他处理器发送A、B矩阵的相关块
 */
void scatter_A_B()
{
   int i,j,k,l;
   int p_imin,p_imax,p_jmin,p_jmax;

   for(k=0; k<p; k++)

   {
      /*计算相应处理器所分得的矩阵块在总矩阵中的坐标范围*/
      p_jmin = (k % sp    ) * dl;
      p_jmax = (k % sp + 1) * dl-1;
      p_imin = (k - (k % sp))/sp * dl;
      p_imax = ((k - (k % sp))/sp +1) *dl -1;
      l = 0;

      /*rank=0的处理器将A,B中的相应块拷至tmp_a,tmp_b，准备向其他处理器发送*/
      for(i=p_imin; i<=p_imax; i++)
      {
            for(j=p_jmin; j<=p_jmax; j++)
            {
              tmp_a[l] = A[i][j];
          tmp_b[l] = B[i][j];
          l++;
          }
      }

      /*rank=0的处理器直接将自己对应的矩阵块从tmp_a,tmp_b拷至a,b*/
      if(k==0)
      {
         memcpy(a, tmp_a, dl2 * sizeof(float));
     memcpy(b, tmp_b, dl2 * sizeof(float));
      } else   /*rank=0的处理器向其他处理器发送tmp_a,tmp_b中相关的矩阵块*/
      {
          MPI_Send(tmp_a, dl2, MPI_FLOAT, k, 1, MPI_COMM_WORLD);
      MPI_Send(tmp_b, dl2, MPI_FLOAT, k, 2, MPI_COMM_WORLD);
      }
   }
}

/*
 *函数名:init_alignment
 *功能:矩阵A和B初始对准
 */
void init_alignment()
{
   /*将A中坐标为(i,j)的分块A(i,j)向左循环移动i步*/
   MPI_Sendrecv(a, dl2, MPI_FLOAT, get_index(my_row,my_col-my_row,sp), 1,
            tmp_a, dl2, MPI_FLOAT, get_index(my_row,my_col+my_row,sp), 1, MPI_COMM_WORLD, &status);
   memcpy(a, tmp_a, dl2 * sizeof(float) );

   /*将B中坐标为(i,j)的分块B(i,j)向上循环移动j步*/
   MPI_Sendrecv(b, dl2, MPI_FLOAT, get_index(my_row-my_col,my_col,sp), 1,
            tmp_b, dl2, MPI_FLOAT, get_index(my_row+my_col,my_col,sp), 1, MPI_COMM_WORLD, &status);
   memcpy(b, tmp_b, dl2 * sizeof(float) );
}

/*
 *函数名：main_shift
 *功能：分块矩阵左移和上移，并计算分块c
 */
void main_shift()
{
   int i,j,k,l;

   for(l=0; l<sp; l++)
   {
     /*矩阵块相乘，c+=a*b */
     for(i=0; i<dl; i++)
       for(j=0; j<dl; j++)
         for(k=0; k<dl; k++)
           c[i*dl+j] += a[i*dl+k]*b[k*dl+j];

      /* 将分块a左移1位 */
      MPI_Send(a , dl2, MPI_FLOAT, get_index(my_row, my_col-1, sp), 1, MPI_COMM_WORLD);
      MPI_Recv(a , dl2, MPI_FLOAT, get_index(my_row, my_col+1, sp), 1, MPI_COMM_WORLD, &status);

      /* 将分块b上移1位 */
      MPI_Send(b , dl2, MPI_FLOAT, get_index(my_row-1, my_col, sp), 1, MPI_COMM_WORLD);
      MPI_Recv(b , dl2, MPI_FLOAT, get_index(my_row+1, my_col, sp), 1, MPI_COMM_WORLD, &status);
   }
}

/*
 *函数名：collect_c
 *功能：rank为0的处理器从其余处理器收集分块矩阵c
 */
void collect_C()
{
   int i,j,i2,j2,k;
   int p_imin,p_imax,p_jmin,p_jmax; /* 分块矩阵在总矩阵中顶点边界值 */

   /* 将rank为0的处理器中分块矩阵c结果赋给总矩阵C对应位置 */
   for (i=0;i<dl;i++)
     for(j=0;j<dl;j++)
       C[i][j]=c[i*dl+j];

   for (k=1;k<p;k++)
   {
       /*将rank为0的处理器从其他处理器接收相应的分块c*/
       MPI_Recv(c, dl2, MPI_FLOAT, k, 1, MPI_COMM_WORLD, &status);

       p_jmin = (k % sp    ) *dl;
       p_jmax = (k % sp + 1) *dl-1;
       p_imin =  (k - (k % sp))/sp     *dl;
       p_imax = ((k - (k % sp))/sp +1) *dl -1;

       i2=0;
       /*将接收到的c拷至C中的相应位置,从而构造出C*/
       for(i=p_imin; i<=p_imax; i++)
       {
           j2=0;
           for(j=p_jmin; j<=p_jmax; j++)
           {
               C[i][j]=c[i2*dl+j2];
               j2++;
           }
           i2++;
       }
   }
}

/*函数名：print
 *功能：打印矩阵
 *输入：指向矩阵指针的指针，字符串
 */
void print(float **m,char *str)
{
   int i,j;
   printf("%s",str);
   /*打印矩阵m*/
   for(i=0;i<dg;i++)
   {
       for(j=0;j<dg;j++)
           printf("%15.0f    ",m[i][j]);
       printf("\n");
   }
   printf("\n");
}

/*
 *函数名：main
 *功能：主过程，Cannon算法，矩阵相乘
 *输入：argc为命令行参数个数，argv为每个命令行参数组成的字符串数组
 */
int main(int argc, char *argv[])
{
   int i;

   MPI_Init(&argc, &argv);                  /* 启动MPI计算 */
   MPI_Comm_size(MPI_COMM_WORLD, &p);       /* 确定处理器个数 */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); /* 确定各自的处理器标识符 */

   sp = sqrt(p);

   /* 确保处理器个数是完全平方数，否则打印错误信息，程序退出 */
   if (sp*sp != p)
   {
      if (my_rank == 0)
      printf("Number of processors is not a quadratic number!\n");
      MPI_Finalize();
      exit(1);
   }

   if (argc != 2)
   {
      if (my_rank == 0)
          printf("usage: mpirun -np ProcNum cannon MatrixDimension!\n");
      MPI_Finalize();
      exit(1);
   }

   dg  = atoi(argv[1]);    /* 总矩阵维数 */
   dl  = dg / sp;          /* 计算分块矩阵维数 */
   dl2 = dl * dl;

   /* 计算处理器在逻辑阵列中的坐标 */
   my_col =  my_rank % sp ;
   my_row = (my_rank-my_col) / sp ;

   /* 为a、b、c分配空间 */
   a = (float *)malloc( dl2 * sizeof(float) );
   b = (float *)malloc( dl2 * sizeof(float) );
   c = (float *)malloc( dl2 * sizeof(float) );

   /* 初始化c */
   for(i=0; i<dl2 ; i++)
     c[i] = 0.0;

   /* 为tmp_a、tmp_b分配空间 */
   tmp_a = (float *)malloc( dl2 * sizeof(float) );
   tmp_b = (float *)malloc( dl2 * sizeof(float) );

   if (my_rank == 0)
   {
      /* rank为0的处理器为A、B、C分配空间 */
      A = (float **)malloc( dg * sizeof(float*) );
      B = (float **)malloc( dg * sizeof(float*) );
      C = (float **)malloc( dg * sizeof(float*) );

      for(i=0; i<dg; i++)
      {
         A[i] = (float *)malloc( dg * sizeof(float) );
         B[i] = (float *)malloc( dg * sizeof(float) );
         C[i] = (float *)malloc( dg * sizeof(float) );
      }
      random_A_B();     /* rank为0的处理器随机化生成A、B矩阵 */
      scatter_A_B();    /* rank为0的处理器向其他处理器发送A、B矩阵的相关块 */
   } else               /* rank不为0的处理器接收来自rank为0的处理器的相应矩阵分块 */
   {
       MPI_Recv(a, dl2, MPI_FLOAT, 0 , 1, MPI_COMM_WORLD, &status);
       MPI_Recv(b, dl2, MPI_FLOAT, 0 , 2, MPI_COMM_WORLD, &status);
   }

   init_alignment();    /* A、B矩阵的初始对准 */

   main_shift();        /* 分块矩阵左移、上移, cannon算法的主过程 */

   if(my_rank == 0)
   {
     collect_C();       /* rank为0的处理器从其余处理器收集分块矩阵c */
     print(A,"random matrix A : \n");  /* 打印矩阵A */
     print(B,"random matrix B : \n");  /* 打印矩阵B */
     print(C,"Matrix C = A * B : \n");     /* 打印矩阵C */

   } else
   {
      MPI_Send(c,dl2,MPI_FLOAT,0,1,MPI_COMM_WORLD); /* rank不为0的处理器向rank为0的处理器发送矩阵块c */
   }

   MPI_Barrier(MPI_COMM_WORLD);        /* 同步所有处理器 */
   MPI_Finalize();                     /* 结束MPI计算 */

   return 0;
}
