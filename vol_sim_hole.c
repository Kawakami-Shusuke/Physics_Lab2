#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool型を使用するために必要
#include <math.h>
#include <time.h>
#include <windows.h>

#define N           300     // 領域のサイズ（300mm x 300mm）
#define ITERATION   100000   // ラプラス反復計算の回数
#define OMEGA       1.95    // SOR法の加速係数(1.0~2.0)
#define DIFF_REQ    1e-5    // 必要な精度

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

                //空孔内の電位はないが、見やすさのため0Vとする。
                phi[i][j] = 0.0; 
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

    //時間計測開始
    clock_t start_time = clock();

    // ラプラス方程式の反復計算ループ
    double  max_diff;
    int     iter = 0;

    do {
        max_diff = 0.0;

        // 反復回数が偶数か奇数かでスキャン方法を変える
        //bool reverse = (iter % 2 == 1);

        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){

                //方向制御。悪影響が大きくボツにしました。
                //int i = reverse ? (N - 1 - i_raw) : i_raw;
                //int j = reverse ? (N - 1 - j_raw) : j_raw;
                
                // 電極および空孔内部は電位計算をスキップ
                if(is_electrode[i][j] || is_hole[i][j]) continue;
                
                double old_val = phi[i][j]; //1ステップ前を記録しておく
                double sum = 0;

                // --- 隣接４点を確認 ---
                // 右（i+1）
                //導体紙外縁および空孔なら勾配0
                sum += (i + 1 < N && !is_hole[i + 1][j]) ? phi[i + 1][j] : phi[i][j];
                // 左 (i-1)
                sum += (i - 1 >= 0 && !is_hole[i - 1][j]) ? phi[i - 1][j] : phi[i][j];
                // 下 (j+1)
                sum += (j + 1 < N && !is_hole[i][j + 1]) ? phi[i][j + 1] : phi[i][j];
                // 上 (j-1)
                sum += (j - 1 >= 0 && !is_hole[i][j - 1]) ? phi[i][j - 1] : phi[i][j];
                    
                //新しい電位を計算
                double phi_new = 0.25 * sum;
                phi[i][j] = old_val + OMEGA * (phi_new - old_val);
                
                // 相対誤差が最大のものを採用
                if (fabs(phi[i][j]) > 1e-10){
                    double diff = fabs(phi_new - old_val) / fabs(old_val);
                    if (diff > max_diff) max_diff = diff;
                }
            } //end of for x_axis
        } //end of for y_axis
        iter++;

        // 1000回ごとに状況を表示
        if (iter % 1000 == 0) {
            printf("Iter: %d, MaxDiff: %e\n", iter, max_diff);
            Beep(800, 100);
        }
            

    } while (max_diff > DIFF_REQ && iter < ITERATION);

    //時間計測終了
    clock_t end_time = clock();

    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("OMEGA: %.3f\n", OMEGA);
    printf("計算時間: %.3f 秒\n", duration);
    printf("計算回数: %d 回\n", iter);
    printf("計算精度: %.1e\n", DIFF_REQ);

    // 3. ファイルに出力
    save_to_csv("vol_sim_hole.csv");
    Beep(1000, 400);

    return 0;
}
