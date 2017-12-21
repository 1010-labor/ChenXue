//cannon
// Master-slave mode

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

int gcd(int M,int size)//获取最大公约数
{
     int i;
     double q;
     q=sqrt((double)size);
     
     for(i=M;i>0;i--)
     {
         if((M%i==0)&&(i<=q))
              return i;
     }
}

float* initMoveA(float *M,int width,int numpack)//A矩阵初始移动
{
	float *tempA;
  tempA=(float*)malloc(sizeof(float)*width*width);
  int i,j,k;
	int q=width/numpack;

	for(i=0;i<numpack;i++)//gong numpack kuai
  {
        int imin=i*q;
        int imax=imin+q-1;

    	for(j=imin;j<=imax;j++)//mei yi kuai you q hang,suo yi q hang de dou yao xiang zuo nuo dong i ge wei zhi
      {
           for(k=0;k<width;k++)
           {
             tempA[j*width+k]=M[j*width+((k+i*q)%width)];
           }           
      }
        
  }

  return tempA;
}
float* initMoveB(float *N,int width,int numpack)//B矩阵初始移动
{
   float *tempB;
   tempB=(float*)malloc(sizeof(float)*width*width);
   int q=width/numpack;
   int i,j,k,m;

   for(j=0;j<numpack;j++)
    { 
        int jmin=j*q;
        int jmax=jmin+q-1;

        for(m=jmin;m<=jmax;m++)
        {
           for(k=0;k<width;k++)
           {
              tempB[k*width+m]=N[((k+j*q)%width)*width+m]; 
           }
        }
    }

    return tempB;
}

int main(int argc,char* argv[])
{
     float *M,*N,*P,*bufferA,*bufferB,*ans;
     int width;
     int myid,size,arrayid,i,j,numpack,m,n,numproc;
     int imin,imax,jmin,jmax;
     int R,L;
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

     if(myid==0)
     {
          float *tempM,*tempN;
          int each;
          printf("please input the width of array M:\n");
          scanf("%d",&width);
          startTime=MPI_Wtime();
          numpack=gcd(width,size);
          each=width/numpack;
          printf("the numpack is %d\n",numpack);
          numproc=numpack*numpack;

          MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
          MPI_Bcast(&numpack,1,MPI_INT,0,MPI_COMM_WORLD);
          
          M=(float*)malloc(sizeof(float)*width*width);//the first array
          N=(float*)malloc(sizeof(float)*width*width);//the second array
          P=(float*)malloc(sizeof(float)*width*width);//the result array
          bufferA=(float*)malloc(sizeof(float)*each*each);
          bufferB=(float*)malloc(sizeof(float)*each*each);
          ans=(float*)malloc(sizeof(float)*each*each);
          tempM=(float*)malloc(sizeof(float)*each*each);
          tempN=(float*)malloc(sizeof(float)*each*each); 

          for(i=0;i<each;i++)
          {    
              for(j=0;j<each;j++)
                 ans[i*each+j]=0;
          }
         
          getcann(M,width,1);
          getcann(N,width,2);
        
          M=initMoveA(M,width,numpack);
          N=initMoveB(N,width,numpack);
 
          for(i=0;i<each;i++)//0 hao jin cheng de fen kuai
          { for(j=0;j<each;j++)
            {
              bufferA[i*each+j]=M[i*width+j];
              bufferB[i*each+j]=N[i*width+j];
            }
          }
         
          for(i=1;i<numproc;i++) // send data to other
          {    
               int rn,cn;
               R=i/numpack;
               L=i%numpack;
               imin=R*each;
               imax=imin+each-1;
               jmin=L*each;
               jmax=jmin+each-1;
               int r=imin,l=jmin;
        
               for(rn=0;rn<each;rn++)// copy data to other
               {
                   for(cn=0;cn<each;cn++)
                   {
                      tempM[rn*each+cn]=M[r*width+l];
                      tempN[rn*each+cn]=N[r*width+l];
                      l++;
                   }
                   l=jmin;
                   r++;
               }
              
                MPI_Send(tempM,each*each,MPI_FLOAT,i,5,MPI_COMM_WORLD);
                MPI_Send(tempN,each*each,MPI_FLOAT,i,6,MPI_COMM_WORLD);
          }
           free(tempM);
           free(tempN);

    }

    else 
    {
        MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&numpack,1,MPI_INT,0,MPI_COMM_WORLD);
        numproc=numpack*numpack;

        if(myid<numproc)
        {
           int each=width/numpack;
            bufferA=(float*)malloc(sizeof(float)*each*each);
            bufferB=(float*)malloc(sizeof(float)*each*each);
            ans=(float*)malloc(sizeof(float)*each*each);
            
            for(i=0;i<each;i++)
            {    
                for(j=0;j<each;j++)
                   ans[i*each+j]=0;
            }

            MPI_Recv(bufferA,each*each,MPI_FLOAT,0,5,MPI_COMM_WORLD,&status);
            MPI_Recv(bufferB,each*each,MPI_FLOAT,0,6,MPI_COMM_WORLD,&status);
        }
    }

if(myid<numproc)
{
    int each=width/numpack;
    for(i=0;i<each;i++)//ji suan jie guo cun zai ans shu zu zhong
    {  
        for(j=0;j<each;j++)
        {
           float temp;
           int q;
           temp=0;
           for(q=0;q<each;q++)
               temp+=bufferA[i*each+q]*bufferB[q*each+j];
           ans[i*each+j]+=temp;
        }
    }
    int k;
    for(k=0;k<numpack-1;k++)
    {
        int R=myid/numpack;
        int L=myid%numpack;
        int armR,armL;
        int souR,souL;
        float temp;
        if(L==0)
            armR=myid+numpack-1;
        else
            armR=myid-1;
        if(R==0)
            armL=myid+numpack*(numpack-1);
        else
            armL=myid-numpack;
        if(L==(numpack-1))
            souR=myid-numpack+1;
        else
            souR=myid+1;
        if(R==(numpack-1))
            souL=myid-numpack*(numpack-1);
        else 
            souL=myid+numpack;//zhe bian dou shi dui de a
       
        MPI_Send(bufferA,each*each,MPI_FLOAT,armR,armR,MPI_COMM_WORLD);
        MPI_Send(bufferB,each*each,MPI_FLOAT,armL,armL,MPI_COMM_WORLD);

        MPI_Recv(bufferA,each*each,MPI_FLOAT,souR,myid,MPI_COMM_WORLD,&status);
        MPI_Recv(bufferB,each*each,MPI_FLOAT,souL,myid,MPI_COMM_WORLD,&status);

        for(i=0;i<each;i++)//ji suan jie guo cun zai ans shu zu zhong
        {  
            for(j=0;j<each;j++)
            {
                float temp;
                int q;
                temp=0;
                for(q=0;q<each;q++)
                    temp+=bufferA[i*each+q]*bufferB[q*each+j];
                ans[i*each+j]+=temp;
            }
        }
    }

    if(myid==0)
    {
        for(i=0;i<each;i++)
            for(j=0;j<each;j++)
               P[i*width+j]=ans[i*each+j];
        int numproc=numpack*numpack;
        for(k=1;k<numproc;k++)
        {
             MPI_Recv(ans,each*each,MPI_FLOAT,k,3,MPI_COMM_WORLD,&status);
             int R=k/numpack;
             int L=k%numpack;
             for(i=0;i<each;i++)
                for(j=0;j<each;j++)
                   P[(R*each+i)*width+(L*each+j)]=ans[i*each+j];
        }
          printf("the result is:\n");
          for(i=0;i<width;i++)//result output
          {
              for(j=0;j<width;j++)
                   printf("%f   ",P[i*width+j]);
               printf("\n");
               fflush(stdout);
          } 
        free(M);
        free(N);
        free(P);
      //  free(tempM);
      //  free(tempN);
        endTime=MPI_Wtime();
        totalTime=endTime-startTime;
        printf("total time is:%.4fs\n",totalTime);
    }
    else
    {
        MPI_Send(ans,each*each,MPI_FLOAT,0,3,MPI_COMM_WORLD);
    }

    free(bufferA);
    free(bufferB);
    free(ans);

}
  
    MPI_Finalize();
    return 0;
}
