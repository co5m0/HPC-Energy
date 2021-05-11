#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define N 1024


int main(int argc, char **argv) {
srand(1);
int i,j,k, n, x;

if(argc>1){
    n = atoi(argv[1]);
}else{
    n=N;
}

double **a, **b, **c;

a = (double**)malloc(n*sizeof(double*));
b = (double**)malloc(n*sizeof(double*));
c = (double**)malloc(n*sizeof(double*));

for( x=0;x<n;x++){
    a[x]=malloc(n*sizeof(double));
    b[x]=malloc(n*sizeof(double));
    c[x]=malloc(n*sizeof(double));
}


//initialization
for (i=0;i<n;i++){
    for (j=0;j<n;j++) {
         a[i][j] = ((double) rand()*(5)/(double)RAND_MAX-2);
            b[i][j] = ((double) rand()*(5)/(double)RAND_MAX-2);
    }
}

//calculate prod
clock_t begin = clock();
 for (i=0;i<n;i++){ 
        for(j=0;j<n;j++) {
            c[i][j]=0;
            for(k=0;k<n;k++) {   
                c[i][j]=c[i][j]+a[i][k]*b[k][j];
            }
    }
 }
clock_t end = clock();
 /*
 printf("MATRIX - A\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++){ 
        printf("%lf ", a[i][j]);                
    }
        printf("\n");
 }
printf("\n"); 


  printf("MATRIX - B\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%lf ", b[i][j]);                
    }
     printf("\n");
 } 

 printf("\n");
printf("MATRIX - C (RESULT) \n");
for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%lf ", c[i][j]);                
    }    
        printf("\n");
} 
*/

double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;
printf("Time exec: %f sec, Matrix size: %d\n", time_spent,n);

}           