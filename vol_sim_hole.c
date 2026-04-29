#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool型を使用するために必要
#include <math.h>

#define N 300 // 領域のサイズ（300mm x 300mm）

double phi[N][N];
bool is_electrode[N][N];
bool is_hole[N][N];

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

//まっすぐな大きい金属電極の配置
void big_rectangle(){
    for (int i = 0; i < N; i++){
        for (int j = 19; j < 46; j++){
            phi[i][j] = 0.0;          // 0V（接地）とする
            is_electrode[i][j] = true;
        }
    }
}

// 傾いた長方形電極の配置
void rectangle_3_volt() {
    // 頂点の定義
    double Ax = 47.0, Ay = 102.0;
    double Bx = 55.0, By = 112.0;
    double Dx = 80.0, Dy = 76.0;

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
                
                phi[i][j] = 3.0;          // 長方形金属を3Vとする
                is_electrode[i][j] = true;
            }
        }
    }
}

// 傾いた長方形電極の配置
void rectangle_6_volt() {
    // 頂点の定義
    double Ex = 51.0, Ey = 211.0;
    double Fx = 51.0, Fy = 224.0;
    double Hx = 121.0, Hy = 212.0;

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
                
                phi[i][j] = 6.0;          // 長方形金属を6Vとする
                is_electrode[i][j] = true;
            } 
        }
    }
}

void hole(){ //holeにはフラグを立てておく。(あとで計算)
     // 頂点の定義
    double Ix = 139.0, Iy = 112.0;
    double Jx = 147.0, Jy = 167.0;
    double Lx = 147.0, Ly = 111.0;

    // 辺のベクトル
    double IJx = Jx - Ix, IJy = Jy - Iy;
    double ILx = Lx - Ix, ILy = Ly - Iy;

    // 辺の長さの2乗（内積の比較用）
    double IJ_dot_IJ = IJx * IJx + IJy * IJy;
    double IL_dot_IL = ILx * ILx + ILy * ILy;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // 点Iから現在の格子点P(i, j)へのベクトル
            double IPx = (double)i - Ix;
            double IPy = (double)j - Iy;

            // 内積を計算
            double dot_IJ = IPx * IJx + IPy * IJy;
            double dot_IL = IPx * ILx + IPy * ILy;

            // 判定：2つのベクトルの影が辺の長さの中に収まっているか
            if (dot_IJ >= 0 && dot_IJ <= IJ_dot_IJ &&
                dot_IL >= 0 && dot_IL <= IL_dot_IL) {
            
                is_hole[i][j] = true;
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
            is_hole[i][j] = false;
        }
    }

    //電極の配置
    big_rectangle();
    rectangle_3_volt();
    rectangle_6_volt();
    
    //ホールの配置
    hole();

    // ラプラス方程式の反復計算ループ
    int iterations = 2000;

    for (int iter = 0; iter < iterations; iter++){
        //平均の計算
        for (int i = 1; i < N - 1; i++){
            for (int j = 1; j < N - 1; j++){

                // 電極および空孔内部は電位計算をスキップ
                if(is_electrode[i][j] || is_hole[i][j]) continue;
                
                double sum = 0;
                int count = 0;

                // --- 隣接４点を確認 ---
                // 右（i+1）
                if (i + 1 < N && !is_hole[i + 1][j]) { sum += phi[i + 1][j]; count++; }
                else { sum += phi[i][j]; count++; } //空孔なら同じ値とみなし、勾配=0にする。 

                // 左 (i-1)
                if (i - 1 >= 0 && !is_hole[i - 1][j]) { sum += phi[i - 1][j]; count++; }
                else { sum += phi[i][j]; count++; }

                // 下 (j+1)
                if (j + 1 < N && !is_hole[i][j + 1]) { sum += phi[i][j + 1]; count++; }
                else { sum += phi[i][j]; count++; }

                // 上 (j-1)
                if (j - 1 >= 0 && !is_hole[i][j - 1]) { sum += phi[i][j - 1]; count++; }
                else { sum += phi[i][j]; count++; }
                    
                //新しい電位を計算
                phi[i][j] = sum / (double)count;
            }
        }
    }

    // 3. ファイルに出力
    save_to_csv("vol_sim_hole.csv");

    return 0;
}