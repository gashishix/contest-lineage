#include <stdio.h>

int main(){
    int M1, N1, M2, N2;

    int matrix_f[100][100];
    int matrix_s[100][100];
    int matrix_r[100][100];

    scanf("%d %d %d %d", &M1, &N1, &M2, &N2);

    if(N1 != M2){
        printf("-1");
    }

    else{
        for(int i = 0; i < M1; i++){
            for(int j = 0; j < N1; j++){
                scanf("%d", &matrix_f[i][j]);
            }
        }

        for(int i = 0; i < M1; i++){
            for(int j = 0; j < N1; j++){
                scanf("%d", &matrix_s[i][j]);
            }
        }

        for(int i = 0; i < M1; i++)
            for(int j = 0; j < N2; j++)
            {
                matrix_r[i][j] = 0;
                for(int k = 0; k < N1; k++)
                    matrix_r[i][j] += matrix_f[i][k] * matrix_s[k][j];
            }

        for(int i = 0; i < M1; i++)
        {

            for(int j = 0; j < N2; j++)
                printf("%d ", matrix_r[i][j]);

            printf("\n");
        }
    }
    return 0;

}
