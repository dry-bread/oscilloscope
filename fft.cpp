#include "fft.h"

// ------------------------------------------------------
//快速傅里叶变换

fft::fft(QObject *parent) : QObject(parent)
{

}

bool fft::fft1(QVector<Complex> inVec, const int len, QVector<Complex> &outVec)
{
    if ((len <= 0) || (inVec.isEmpty()) || ( outVec.isEmpty()))
        return false;
    if (!is_power_of_two(len))
        return false;
    //    qDebug()<<"fft come in"<<endl;
    //    qDebug()<<"fft len"<<len << "in" << inVec.size() <<endl;
    // create the weight array
    Complex *pVec = new Complex[len];
    Complex *Weights = new Complex[len];
    Complex *X = new Complex[len];
    int                   *pnInvBits = new int[len];
    //    qDebug()<<"fft 权值加载完毕"<<endl;

    for(int i = 0; i < len;i++)//输入时域数据
    {
        pVec[i].im = inVec.at(i).im;
        pVec[i].rl = inVec.at(i).rl;
    }

    //    qDebug()<<"fft 时域数据加载完毕"<<endl;
    // 计算权重序列
    double fixed_factor = (-2 * PI) / len;
    for (int i = 0; i < len / 2; i++) {
        double angle = i * fixed_factor;
        Weights[i].rl = cos(angle);
        Weights[i].im = sin(angle);
    }
    for (int i = len / 2; i < len; i++) {
        Weights[i].rl = -(Weights[i - len / 2].rl);
        Weights[i].im = -(Weights[i - len / 2].im);
    }

    int r = get_computation_layers(len);
    //    qDebug()<<"fft 权重序列加载完毕"<<endl;
    // 计算倒序位码
    int index = 0;
    for (int i = 0; i < len; i++) {
        index = 0;
        for (int m = r - 1; m >= 0; m--) {
            index += (1 && (i & (1 << m))) << (r - m - 1);
        }
        pnInvBits[i] = index;
        X[i].rl = pVec[pnInvBits[i]].rl;
        X[i].im = pVec[pnInvBits[i]].im;
    }
    //    qDebug()<<"fft 倒序位码加载完毕"<<endl;
    // 计算快速傅里叶变换
    for (int L = 1; L <= r; L++) {
        int distance = 1 << (L - 1);
        int W = 1 << (r - L);

        int B = len >> L;
        int N = len / B;

        for (int b = 0; b < B; b++) {
            int mid = b*N;
            for (int n = 0; n < N / 2; n++) {
                int index = n + mid;
                int dist = index + distance;
                pVec[index].rl = X[index].rl + (Weights[n*W].rl * X[dist].rl - Weights[n*W].im * X[dist].im);                      // Fe + W*Fo
                pVec[index].im = X[index].im + Weights[n*W].im * X[dist].rl + Weights[n*W].rl * X[dist].im;
            }
            for (int n = N / 2; n < N; n++) {
                int index = n + mid;
                pVec[index].rl = X[index - distance].rl + Weights[n*W].rl * X[index].rl - Weights[n*W].im * X[index].im;        // Fe - W*Fo
                pVec[index].im = X[index - distance].im + Weights[n*W].im * X[index].rl + Weights[n*W].rl * X[index].im;
            }
        }

        for(int i = 0; i< len;i++)
        {
            X[i].im = pVec[i].im;
            X[i].rl = pVec[i].rl;
        }
    }
    //    qDebug()<<"fft 计算完毕"<<endl;

    for(int i = 0; i < len;i++)//输出频域
    {
        outVec[i].im = pVec[i].im;
        outVec[i].rl = pVec[i].rl;
        //        qDebug()<<"频域复数"<<pVec[i].im<<pVec[i].rl<<endl;
    }



    if (Weights)      delete[] Weights;
    if (X)                 delete[] X;
    if (pnInvBits)    delete[] pnInvBits;
    if (pVec)           delete[] pVec;
    return true;
}

//逆转换
bool fft::ifft(const Complex inVec[], const int len, Complex outVec[])
{
    char msg[256] = "11111 ";

    if ((len <= 0) || (!inVec))
        return false;
    if (false == is_power_of_two(len)) {//数值长度是否可用
        return false;
    }

    double         *W_rl = new double[len];//蝶形矩阵
    double         *W_im = new double[len];
    double         *X_rl = new double[len];
    double         *X_im = new double[len];
    double         *X2_rl = new double[len];
    double         *X2_im = new double[len];

    double fixed_factor = (-2 * PI) / len;//固定常数
    for (int i = 0; i < len / 2; i++) {//求矩阵
        double angle = i * fixed_factor;
        W_rl[i] = cos(angle);
        W_im[i] = sin(angle);
    }
    for (int i = len / 2; i < len; i++) {//求矩阵
        W_rl[i] = -(W_rl[i - len / 2]);
        W_im[i] = -(W_im[i - len / 2]);
    }

    // 时域数据写入X1
    for (int i = 0; i < len; i++) {
        X_rl[i] = inVec[i].rl;//时域实部
        X_im[i] = inVec[i].im;//时域虚部
    }
    memset(X2_rl, 0, sizeof(double)*len);
    memset(X2_im, 0, sizeof(double)*len);

    int r = get_computation_layers(len);
    if (-1 == r)
        return false;
    for (int L = r; L >= 1; L--) {
        int distance = 1 << (L - 1);
        int W = 1 << (r - L);

        int B = len >> L;
        int N = len / B;

        for (int b = 0; b < B; b++) {
            for (int n = 0; n < N / 2; n++) {
                int index = n + b*N;
                X2_rl[index] = (X_rl[index] + X_rl[index + distance]) / 2;
                X2_im[index] = (X_im[index] + X_im[index + distance]) / 2;
            }
            for (int n = N / 2; n < N; n++) {
                int index = n + b*N;
                X2_rl[index] = (X_rl[index] - X_rl[index - distance]) / 2;           // 需要再除以W_rl[n*W]
                X2_im[index] = (X_im[index] - X_im[index - distance]) / 2;
                double square = W_rl[n*W] * W_rl[n*W] + W_im[n*W] * W_im[n*W];          // c^2+d^2
                double part1 = X2_rl[index] * W_rl[n*W] + X2_im[index] * W_im[n*W];         // a*c+b*d
                double part2 = X2_im[index] * W_rl[n*W] - X2_rl[index] * W_im[n*W];          // b*c-a*d
                if (square > 0) {
                    X2_rl[index] = part1 / square;
                    X2_im[index] = part2 / square;
                }
            }
        }
        memcpy(X_rl, X2_rl, sizeof(double)*len);
        memcpy(X_im, X2_im, sizeof(double)*len);
    }

    // 位码倒序
    int index = 0;
    for (int i = 0; i < len; i++) {
        index = 0;
        for (int m = r - 1; m >= 0; m--) {
            index += (1 && (i & (1 << m))) << (r - m - 1);
        }
        outVec[i].rl = X_rl[index];
        outVec[i].im = X_im[index];
    }

    if (W_rl)      delete[] W_rl;
    if (W_im)    delete[] W_im;
    if (X_rl)      delete[] X_rl;
    if (X_im)     delete[] X_im;
    if (X2_rl)     delete[] X2_rl;
    if (X2_im)    delete[] X2_im;

    return true;

}

int fft::get_computation_layers(int num)
{

    int nLayers = 0;
    int len = num;
    if (len == 2)
        return 1;
    while (true) {
        len = len / 2;
        nLayers++;
        if (len == 2)
            return nLayers + 1;
        if (len < 1)
            return -1;
    }

}

bool fft::is_power_of_two(int num)
{

    int temp = num;
    int mod = 0;
    int result = 0;

    if (num < 2)
        return false;
    if (num == 2)
        return true;

    while (temp > 1)
    {
        result = temp / 2;
        mod = temp % 2;
        if (mod)
            return false;
        if (2 == result)
            return true;
        temp = result;
    }
    return false;

}
// ------------------------------------------------------
