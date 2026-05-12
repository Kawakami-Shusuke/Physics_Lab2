#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool型を使用するために必要
#include <math.h>
#include <time.h>
#include <windows.h>

#define N           300     // 領域のサイズ（300mm x 300mm）
#define ITERATION   100000   // 計算回数
#define OMEGA       1.0    // SOR法の加速係数（1~2）
#define DIFF_REQ     1e-5   // 必要な精度


double phi_new[N][N];
double phi_old[N][N];
bool is_electrode[N][N];

// CSVファイルへの保存関数
void save_to_csv(const char* filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("ファイルを開けませんでした。\n");
        return;
    }

    // ImageJで正しく表示されるよう、j(y方向)を外側のループにします
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            fprintf(fp, "%f", phi_new[i][j]);
            if (i < N - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("データを %s に保存しました。\n", filename);
}




//小さい円電極
#define r1 14.5 //円電極の内径
#define r2 24.5 //円電極の外径
#define center_x  110 //中心のx座標 //110
#define center_y 144 //中心のy座標 //144

//大きい円電極
#define R1 74.5 //円電極の内径
#define R2 84.5 //円電極の外径
#define centerX  110 //中心のx座標
#define centerY 144 //中心のy座標

void small_circle(){

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            //円の中心からの距離を計算
            double radius =((double)i - center_x)*((double)i - center_x)+((double)j - center_y)*((double)j - center_y);
            radius = sqrt(radius);

            //判定
            if (r1 <= radius && radius <= r2){
                phi_new[i][j] = 6.0;
                is_electrode[i][j] = true; 
            } 
        }
    }
}

void big_circle(){

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            //円の中心からの距離を計算
            double radius =((double)i - centerX)*((double)i - centerX)+((double)j - centerY)*((double)j - centerY);
            radius = sqrt(radius);

            //判定
            if (R1 <= radius && radius <= R2){
                phi_new[i][j] = 0.0;
                is_electrode[i][j] = true; 
            } 
        }
    }
}

int main(void) {
    // 1. 全体を初期化（0Vで埋める）
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            phi_new[i][j] = 0.0;
            is_electrode[i][j] = false;
        }
    }

    // 2. 電極の配置
    big_circle();
    small_circle();

    //時間計測を開始
    clock_t start_time = clock();

    // ラプラス方程式の反復計算ループ
    double  max_diff;
    int     iter = 0;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            phi_old[i][j] = phi_new[i][j];
        }
    }

    do {
        max_diff = 0.0;

        //平均の計算
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                // 電極および空孔内部は電位計算をスキップ
                if(is_electrode[i][j]) continue;
                
                double old_val = phi_new[i][j]; //１ステップ前を記録しておく
                double sum = 0;

                // --- 隣接４点を確認 ---
                // 右（i+1）
                sum += (i + 1 < N) ? phi_old[i + 1][j] : phi_old[i][j]; 
                //導体紙外縁なら勾配0
                // 左 (i-1)
                sum += (i - 1 >= 0) ? phi_old[i - 1][j] : phi_old[i][j];
                // 下 (j+1)
                sum += (j + 1 < N) ? phi_old[i][j + 1] : phi_old[i][j];
                // 上 (j-1)
                sum += (j - 1 >= 0) ? phi_old[i][j - 1] : phi_old[i][j];
                    
                //新しい電位を計算
                phi_new[i][j] = 0.25 * sum;
                //phi[i][j] = old_val + OMEGA * (phi_new - old_val);

                // 相対誤差が最大のものを採用
                if (fabs(phi_new[i][j]) > 1e-10){ //分母0を回避
                    double diff = fabs(phi_new[i][j] - old_val)7;
                    if (diff > max_diff) max_diff = diff;
                }
            }
        }
        iter++;

        if (iter % 1000 == 0) {
            printf("Iter: %d, MaxDiff: %e\n", iter, max_diff);
            Beep(800, 100);
        }

    } while (max_diff > DIFF_REQ && iter < ITERATION);

    //時間計測の終了
    clock_t end_time = clock();

    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("試行時間: %.3f 秒\n", duration);
    printf("試行回数: %d 回\n", iter);
    printf("計算精度: %.1e\n", DIFF_REQ);

    // 3. ファイルに出力
    save_to_csv("asymmetric_circle.csv");
    Beep(1000, 400);

    return 0;
}
