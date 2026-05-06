#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool型を使用するために必要
#include <math.h>
#include <time.h>

#define N           300     // 領域のサイズ（300mm x 300mm）
#define ITERATION   10000   //計算回数
#define OMEGA       1.95    // SOR法の加速係数（1~2）


double phi[N][N];
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
            fprintf(fp, "%f", phi[i][j]);
            if (i < N - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("データを %s に保存しました。\n", filename);
}

// 傾いた長方形電極の配置
void outer_rectangle_electrode() {
    // 頂点の定義
    double Ax = 42.0, Ay = 5.0;
    double Bx = 33.0, By = 16.0;
    double Dx = 95.0, Dy = 53.0;

    // 辺のベクトル
    double ABx = Bx - Ax, ABy = By - Ay;
    double ADx = Dx - Ax, ADy = Dy - Ay;

    // 辺の長さの2乗（内積の比較用）
    double AB_dot_AB = ABx * ABx + ABy * ABy;
    double AD_dot_AD = ADx * ADx + ADy * ADy;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // 点Aから現在の格子点P(i, j)へのベクトル
            double APx = (double)i - Ax;
            double APy = (double)j - Ay;

            // 内積を計算
            double dot_AB = APx * ABx + APy * ABy;
            double dot_AD = APx * ADx + APy * ADy;

            // 判定：2つのベクトルの影が辺の長さの中に収まっているか
            if (dot_AB >= 0 && dot_AB <= AB_dot_AB &&
                dot_AD >= 0 && dot_AD <= AD_dot_AD) {
                
                phi[i][j] = 0.0;          // 長方形金属を0Vとする
                is_electrode[i][j] = true;
            } 
        }
    }
}

// 傾いた長方形電極の配置
void inner_rectangle_electrode() {
    // 頂点の定義
    double Ex = 43.0, Ey = 140.0;
    double Fx = 44.0, Fy = 155.0;
    double Hx = 87.0, Hy = 139.0;

    // 辺のベクトル
    double EFx = Fx - Ex, EFy = Fy - Ey;
    double EHx = Hx - Ex, EHy = Hy - Ey;

    // 辺の長さの2乗（内積の比較用）
    double EF_dot_EF = EFx * EFx + EFy * EFy;
    double EH_dot_EH = EHx * EHx + EHy * EHy;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // 点Eから現在の格子点P(i, j)へのベクトル
            double EPx = (double)i - Ex;
            double EPy = (double)j - Ey;

            // 内積を計算
            double dot_EF = EPx * EFx + EPy * EFy;
            double dot_EH = EPx * EHx + EPy * EHy;

            // 判定：2つのベクトルの影が辺の長さの中に収まっているか
            if (dot_EF >= 0 && dot_EF <= EF_dot_EF &&
                dot_EH >= 0 && dot_EH <= EH_dot_EH) {
                
                phi[i][j] = 0.0;          // 長方形金属を0Vとする
                is_electrode[i][j] = true;
            } 
        }
    }
}

#define R1 50 //円電極の内径
#define R2 60 //円電極の外径
#define centerX  80 //中心のx座標
#define centerY 156 //中心のy座標

void circle_electrode(){

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            //円の中心からの距離を計算
            double radius =((double)i - centerX)*((double)i - centerX)+((double)j - centerY)*((double)j - centerY);
            radius = sqrt(radius);

            //判定
            if (R1 <= radius && radius <= R2){
                phi[i][j] = 6.0;
                is_electrode[i][j] = true; 
            } 
        }
    }
}

int main(void) {
    // 1. 全体を初期化（0Vで埋める）
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            phi[i][j] = 0.0;
            is_electrode[i][j] = false;
        }
    }

    // 2. 電極の配置
    outer_rectangle_electrode();
    inner_rectangle_electrode();
    circle_electrode();

    //時間計測を開始
    clock_t start_time = clock();

    // ラプラス方程式の反復計算ループ
    for (int iter = 0; iter < ITERATION; iter++){
        //平均の計算
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                // 電極および空孔内部は電位計算をスキップ
                if(is_electrode[i][j]) continue;
                
                double sum = 0;

                // --- 隣接４点を確認 ---
                // 右（i+1）
                if (i + 1 < N) { sum += phi[i + 1][j]; }
                else {sum += phi[i][j]; } //空孔なら勾配0

                // 左 (i-1)
                if (i - 1 >= 0) { sum += phi[i - 1][j]; }
                else { sum += phi[i][j]; }

                // 下 (j+1)
                if (j + 1 < N) { sum += phi[i][j + 1]; }
                else { sum += phi[i][j]; }

                // 上 (j-1)
                if (j - 1 >= 0) { sum += phi[i][j - 1]; }
                else { sum += phi[i][j]; }
                    
                //新しい電位を計算
                double phi_new = 0.25 * sum;
                phi[i][j] = phi[i][j] + OMEGA * (phi_new - phi[i][j]);
            
            }
        }
    }

    //時間計測の終了
    clock_t end_time = clock();

    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("試行時間: %.3f 秒\n", duration);
    printf("試行回数: %d 回\n", ITERATION);

    // 3. ファイルに出力
    save_to_csv("vol_sim.csv");

    return 0;
}
