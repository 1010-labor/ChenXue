#include<stdlib.h>
#include<stdio.h>

int main()
{
    FILE *fin;
    int i,j;
    float temp=2;
    fin=fopen("./det","r+w");
    for(i=0;i<100;i++)
      for(j=0;j<100;j++)
          fprintf(fin,"%f  ",temp);
    fclose(fin);
}
