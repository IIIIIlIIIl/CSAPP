/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int a[8];
    if(M==32){
        for(int i=0;i<32;i+=8){
            for(int j=0;j<32;j+=8){
                for(int x=0;x<8;x++){
                    for(int y=0;y<8;y++)a[y]=A[i+x][j+y];
                    for(int y=0;y<8;y++)B[j+y][i+x]=a[y];
                }
            }
        }   
    }else if(M==64){
        for(int i=0;i<64;i+=8){
            for(int j=0;j<64;j+=4){
                if((j/4)&1){
                    if(i==j||j==i+4){
                        for(int y=0;y<2;y++)a[y+2]=A[i+7][j+y];
                        for(int y=0;y<2;y++)a[y+6]=A[i+7][j+y+2];
                        for(int y=0;y<2;y++)B[j+y][i+6]=a[y];
                        for(int y=0;y<2;y++)B[j+y+2][i+6]=a[y+4];
                        for(int y=0;y<2;y++)B[j+y][i+7]=a[y+2];
                        for(int y=0;y<2;y++)B[j+y+2][i+7]=a[y+6];
                        for(int x=4;x>=0;x-=2){
                            for(int y=0;y<4;y++)a[y]=A[i+x][j+y];
                            for(int y=0;y<4;y++)a[y+4]=A[i+x+1][j+y];
                            for(int y=0;y<4;y++)B[j+y][i+x]=a[y];
                            for(int y=0;y<4;y++)B[j+y][i+x+1]=a[y+4];
                        }   
                    } else{
                        for(int x=6;x>=2;x-=2){
                            for(int y=0;y<4;y++)B[j+y][i+x]=A[i+x][j+y];
                            for(int y=0;y<4;y++)B[j+y][i+x+1]=A[i+x+1][j+y];
                        }
                        for(int y=0;y<4;y++)B[j+y][i]=a[y];
                        for(int y=0;y<4;y++)B[j+y][i+1]=a[y+4];
                    }
                }else{
                    if(i==j||j==i+4){
                        for(int x=0;x<6;x+=2){
                            for(int y=0;y<4;y++)a[y]=A[i+x][j+y];
                            for(int y=0;y<4;y++)a[y+4]=A[i+x+1][j+y];
                            for(int y=0;y<4;y++)B[j+y][i+x]=a[y];
                            for(int y=0;y<4;y++)B[j+y][i+x+1]=a[y+4];
                        } 
                        for(int y=0;y<4;y++)a[y]=A[i+6][j+y];
                        for(int y=0;y<4;y++)a[y+4]=A[i+7][j+y];
                        for(int y=0;y<2;y++)B[j+y][i+6]=a[y],a[y]=A[i+6][j+y+4];
                        for(int y=0;y<2;y++)B[j+y][i+7]=a[y+4],a[y+4]=A[i+6][j+y+6];
                        for(int y=2;y<4;y++)B[j+y][i+6]=a[y];
                        for(int y=2;y<4;y++)B[j+y][i+7]=a[y+4];
                    }else{
                        for(int x=0;x<8;x+=2){
                            if(x==0){
                                for(int y=0;y<4;y++)a[y]=A[i+x][j+y+4];
                                for(int y=0;y<4;y++)a[y+4]=A[i+x+1][j+y+4];
                            }
                            for(int y=0;y<4;y++)B[j+y][i+x]=A[i+x][j+y];
                            for(int y=0;y<4;y++)B[j+y][i+x+1]=A[i+x+1][j+y];
                        }
                    }
                }   
            }
        } 
    }else{
        for(int i=0;i<67;i+=8){
            for(int j=0;j<61;j+=8){
                if(i==64&&j==56){
                    for(int y=0;y<5;y++)a[y]=A[i][j+y];
                    for(int y=0;y<3;y++)a[y+5]=A[i+1][j+y];
                    for(int y=0;y<5;y++)B[j+y][i]=a[y];
                    for(int y=0;y<3;y++)a[y]=a[y+5];
                    for(int y=3;y<5;y++)a[y]=A[i+1][j+y];
                    for(int y=0;y<5;y++)B[j+y][i+1]=a[y];
                    for(int y=0;y<5;y++)a[y]=A[i+2][j+y];
                    for(int y=0;y<5;y++)B[j+y][i+2]=a[y];
                }else{
                    for(int x=0;x<MIN(8,67-i);x++){
                        for(int y=0;y<MIN(8,61-j);y++)a[y]=A[i+x][j+y];
                        for(int y=0;y<MIN(8,61-j);y++)B[j+y][i+x]=a[y];
                    }
                }
            }
        } 
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

