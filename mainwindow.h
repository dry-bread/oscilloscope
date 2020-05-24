#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QChartView>
#include <QChart>
#include <QSplineSeries>
#include <QLineSeries>
#include <QHBoxLayout>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QTimer>
#include <QString>
#include <QList>
#include <fft.h>






namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void init_time_chart();
    void init_menu();
    ~MainWindow();

private slots:
    void on_btn_sample_start_clicked();
    void txt_to_data();

    void save_time_pic();

    void save_freq_pic();

    void save_voltages_txt();

    void save_freq_txt();

    void save_keyvalue_txt();

    void RealtimeDataSlot();

    void on_btn_sample_pause_clicked();

    void on_btn_sample_stop_clicked();

    void on_btn_sample_goon_clicked();

private:
    Ui::MainWindow *ui;
    QtCharts::QChart *time_chart;//时域波形图
    QtCharts::QChart *freq_chart;//频域波形图
    QtCharts::QChartView *time_chartview;
    QtCharts::QChartView *freq_chartview;
    QtCharts::QLineSeries *time_series;
    QtCharts::QLineSeries *freq_series;
    QHBoxLayout *time_layout;
    QHBoxLayout *freq_layout;
//    QtCharts::QDateTimeAxis *time_axisX;
//    QtCharts::QValueAxis *time_axisY;
    QtCharts::QValueAxis *time_axis;
    QtCharts::QValueAxis *freq_axis;
    int  timeCount;//记录秒数
    int signal_pointer;//当前信号指针
    int signal_pointer_temp;//用于暂停时记住状态
    QTimer *myTimer;//定时器
    qreal m_x;//数据集当前添加的x
    qreal m_y;//数据集当前添加的y
    QStringList fileNames;//传入的文件
    QString fileName;//传入的文件路径
    qreal sampling_time;//采样时间
    QList<qreal> voltages;//创建QList对象来存储电压数据
//    QVector<double> m_x_freq,m_y_freq;//用来存储用于fft的时域数据
    qreal voltage_max;//最大幅值
    qreal voltage_min;//最下幅值
    qreal voltage_mean;//平均值
    qreal axis_MoveStep;//坐标轴移动长度
    QStringList past_voltages;//存储过去的电压

    QVector<Complex> in_time;//fft 输入的时域数据
    //QVector<Complex> out_freq;//fft 输出的频域数据
    qreal out_freq_am;//频率幅度值
    qreal out_freq_kHz;//频率点
//    QList<qreal> peaks;//峰值集合
//    QList<qreal> peaks_kHz;//峰值集合
    QStringList past_freq_am;
    QStringList past_freq_Hz;
    qreal peak_left;
    qreal peak_temp;
    qreal peak_temp_kHz;
    fft m_fft;
    qreal signal_period;//信号周期
    qreal max_am;
    qreal max_kHz;
    qreal number_error;

    void work_enable(bool en);
    void readTxt(QString path);
    int maxCommonDivisor1(int x,int y);//最大公约数
    int minCommonMultiple(qreal x,qreal y);//最小公倍数

    void save_pic(QObject *widget);
    void save_txt(QString str);
};

#endif // MAINWINDOW_H
