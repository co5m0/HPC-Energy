#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define N 10


int main(int argc, char **argv) {
clock_t begin = clock();
int i,j,k, n;

if(argc>1){
    n = atoi(argv[1]);
}else{
    n=N;
}

int **a, **b, **c;

a = (int**)malloc(n*sizeof(int*));
b = (int**)malloc(n*sizeof(int*));
c = (int**)malloc(n*sizeof(int*));

for(int x=0;x<n;x++){
    a[x]=malloc(n*sizeof(int));
    b[x]=malloc(n*sizeof(int));
    c[x]=malloc(n*sizeof(int));
}


//initialization
for (i=0;i<n;i++){
    for (j=0;j<n;j++) {
        a[i][j]= (int)rand()%5-2;
        b[i][j]=(int)rand()%5-1;
    }
}

//calculate prod
 for (i=0;i<n;i++){ 
        for(j=0;j<n;j++) {
            c[i][j]=0;
            for(k=0;k<n;k++) {   
                c[i][j]=c[i][j]+a[i][k]*b[k][j];
            }
    }
 }
 /* printf("MATRIX - A\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++){ 
        printf("%d ", a[i][j]);                
    }
        printf("\n");
 }
printf("\n"); */


 /*  printf("MATRIX - B\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%d ", b[i][j]);                
    }
     printf("\n");
 } */

/* printf("\n");
printf("MATRIX - C (RESULT) \n");
for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%d ", c[i][j]);                
    }    
        printf("\n");
} */
clock_t end = clock();
double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;
printf("Time exec: %f sec\n", time_spent);

}           