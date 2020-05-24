#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDate>
#include <QFileDialog>
#include <QPen>


#include <QFile>
#include <QVector>
#include <QList>
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include "fft.h"
#include <QtCore/qmath.h>


QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    simulate_enable(false);
    init_time_chart();
    init_menu();
    work_enable(false);
    ui->lineEdit_sample_time->setValidator(new QIntValidator(0,10000,this));//控制采样时间只能输入0到10000的整数
    myTimer = new QTimer(this);//定时器
    connect(myTimer , SIGNAL(timeout()), this, SLOT(RealtimeDataSlot()));//连接定时器时间到达响应的槽函数
}


void MainWindow::init_time_chart()
{
    time_series = new QLineSeries;
    freq_series = new QLineSeries;
    time_chart = new QChart;
    freq_chart = new QChart;
    time_chartview = new QChartView(time_chart);
    freq_chartview = new QChartView(freq_chart);
    time_layout = new QHBoxLayout;
    freq_layout = new QHBoxLayout;
    time_axis = new QValueAxis;
    freq_axis = new QValueAxis;

    time_chart->legend()->hide(); //隐藏图例
    freq_chart->legend()->hide(); //隐藏图例

    //数据绑定 时域
    QPen green(Qt::red);//设置画笔颜色
    green.setWidth(3);//设置画笔宽度
    time_series->setPen(green);//将画笔设置到数据集中
    time_series->setUseOpenGL(true);
    time_chart->addSeries(time_series);//将数据集绑定到时域波形的图表中
    time_chart->setAnimationOptions(QChart::SeriesAnimations);//设置曲线动画模式

    //数据绑定 频域
    QPen pen(Qt::red);//设置画笔颜色
    pen.setWidth(3);//设置画笔宽度
    freq_series->setPen(pen);//将画笔设置到数据集中
    freq_series->setUseOpenGL(true);
    freq_chart->addSeries(freq_series);//将数据集绑定到时域波形的图表中
    freq_chart->setAnimationOptions(QChart::SeriesAnimations);//设置曲线动画模式

    //建立坐标轴 时域
    time_chart->createDefaultAxes();//清空并建立坐标轴
    time_chart->setAxisX(time_axis,time_series);//设置X轴，同时拥有轴和数据
    time_axis->setTickCount(10);//设置网格数
    time_chart->axisX()->setRange(0,30);
    time_chart->axisX()->setTitleText("时间/ms");
    time_chart->axisY()->setRange(-10,10);
    time_chart->axisY()->setTitleText("电压/V");

    //建立坐标轴 频域
    freq_chart->createDefaultAxes();//清空并建立坐标轴
    freq_chart->setAxisX(freq_axis,freq_series);//设置X轴，同时拥有轴和数据
    freq_axis->setTickCount(10);//设置网格数
    freq_chart->axisX()->setRange(0,50);
    freq_chart->axisY()->setRange(0,10);
    freq_chart->axisX()->setTitleText("频率/Hz");
    freq_chart->axisY()->setTitleText("幅值");

    //绑定图层 时域
    time_layout->addWidget(time_chartview);
    time_layout->setMargin(0);
    ui->widget_time->setLayout(time_layout);

    //绑定图层 频域
    freq_layout->addWidget(freq_chartview);
    freq_layout->setMargin(0);
    ui->widget_freq->setLayout(freq_layout);


}

void MainWindow::init_menu()
{
    connect(ui->action_input_txt,SIGNAL(triggered()),this,SLOT(txt_to_data()));//导入txt文件
    connect(ui->action_time_pic, SIGNAL(triggered()), this, SLOT(save_time_pic()));//保存时域截图
    connect(ui->action_freq_pic, SIGNAL(triggered()), this, SLOT(save_freq_pic()));//保存频域截图
    connect(ui->action_output_voltages,SIGNAL(triggered()),this,SLOT(save_voltages_txt()));//保存当前数据
    connect(ui->action_output_freq,SIGNAL(triggered()),this,SLOT(save_freq_txt()));
    connect(ui->action_output_keyvalue,SIGNAL(triggered()),this,SLOT(save_keyvalue_txt()));
    connect(ui->action_simulate,SIGNAL(triggered()),this,SLOT(simulate()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_sample_start_clicked()
{

    QString tempString;
    tempString = ui->lineEdit_sample_time->text();
    if(!tempString.isEmpty()){
        sampling_time =tempString.toInt();//获得采样时间
        qDebug()<<"采样时间："<<sampling_time<<endl;
        myTimer->setInterval(sampling_time);
        time_chart->axisX()->setRange(0,sampling_time*100);
        m_x=0 -sampling_time;
        m_y=0;
        voltage_max=0;
        voltage_min=0;
        signal_pointer=0;
        voltage_mean=0;
        myTimer->start();
        ui->btn_sample_start->setEnabled(false);
        ui->lineEdit_sample_time->setEnabled(false);
        ui->btn_sample_stop->setEnabled(true);
        ui->btn_sample_goon->setEnabled(true);
        ui->btn_sample_pause->setEnabled(true);

    }
    else {
        QMessageBox::warning(this,"Error","请输入采样时间",QMessageBox::Ok);

    }

}
void MainWindow::txt_to_data()//导入txt文件并获得数据
{
    //定义文件对话框类
    QFileDialog *fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(QStringLiteral("选中文件"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("File(*.txt)"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    if (fileDialog->exec()) {
        fileNames = fileDialog->selectedFiles();
        fileName = fileNames[0];
        readTxt(fileName);//读取txt文件中的数据
        if(!voltages.isEmpty()){
            ui->btn_sample_start->setEnabled(true);
            ui->lineEdit_sample_time->setEnabled(true);
        }
        else{
            QMessageBox::warning(this,"Error","文件读取失败",QMessageBox::Ok);
        }

    }

}

void MainWindow::save_time_pic()
{
    save_pic(ui->widget_time);
}

void MainWindow::save_freq_pic()
{
    save_pic(ui->widget_freq);
}

void MainWindow::save_voltages_txt()
{
    if(past_voltages.isEmpty()){
        QMessageBox::warning(this,"警告！","没有数据可以保存",QMessageBox::Ok);
    }
    else {
        QString str=past_voltages.join(' ');
        save_txt(str);
    }

}

void MainWindow::save_freq_txt()
{
    if(past_freq_am.isEmpty() || past_freq_Hz.isEmpty() ){
        QMessageBox::warning(this,"警告！","没有数据可以保存",QMessageBox::Ok);
    }
    else {
        QStringList strl;
        for(auto i=0;i<past_freq_Hz.length();i++){
            strl.append(past_freq_Hz.at(i)+' '+past_freq_am.at(i));
        }
        QString str=strl.join('\n');
        save_txt(str);
    }
}

void MainWindow::save_keyvalue_txt()
{
    QString s1="当前电压值： "+ui->label_voltages_current->text();
    QString s2="主频率： "+ui->label_voltages_Hz->text();
    QString s3="电压最大值： "+ui->label_voltages_max->text();
    QString s4="电压平均值： "+ui->label_voltages_mean->text();
    QString s5="电压最小值： "+ui->label_voltages_min->text();
    save_txt(s1+'\n'+s2+'\n'+s3+'\n'+s4+'\n'+s5);
}


void MainWindow::RealtimeDataSlot()//实时采样、分析
{
    if(simulate_flag){//仿真
        m_x +=s_sampletime;
        switch (s_wave) {
        case 1://
            m_y=s_addv+s_voltage_am * sin(m_x*(2*PI)/s_waveperiod);
            break;
        case 2://
            if((((int)m_x) % s_waveperiod) <(s_waveperiod/2.0) ){
                m_y=s_addv+s_voltage_am;
            }
            else {
                m_y= s_addv+0-s_voltage_am;
            }
            break;
        case 3://
            if((((int)m_x) % s_waveperiod) <(s_waveperiod/2.0) ){
                m_y=s_addv+(s_voltage_am * 2 /s_waveperiod *(((int)m_x) % s_waveperiod));
            }
            else {
                m_y= s_addv+(s_voltage_am * 2 /s_waveperiod *(s_waveperiod-(((int)m_x) % s_waveperiod)));
            }
            break;
        default:
            break;
        }
    }
    else{//实际输入
        if(signal_pointer>=voltages.length()){//采样结束后，停止
            myTimer->stop();
            QMessageBox::warning(this,"完成","采样完成",QMessageBox::Ok);
            in_time.clear();//清除原有频域数据
            return;
        }
        //时域波形---------------------
        m_x += sampling_time;
        //    m_y=sin(m_x);
        m_y = voltages[signal_pointer];
        //求最大最小值
        if(m_y>voltage_max){
            voltage_max = m_y;
            qDebug()<<"max :"<<voltage_max<<endl;
            ui->label_voltages_max->setText(QString::number(voltage_max) );
        }
        if(m_y<voltage_min){
            voltage_min = m_y;
            qDebug()<<"min :"<<voltage_min<<endl;
            ui->label_voltages_min->setText(QString::number(voltage_min) );
        }
        //求平均值
        voltage_mean=(voltage_mean*signal_pointer+m_y)/(signal_pointer+1);
        ui->label_voltages_mean->setText(QString::number(voltage_mean));

    }
    //绘制图线
    time_series->append(m_x, m_y);
    //调整坐标轴
    if(time_axis->max()<m_x){
        axis_MoveStep = m_x - time_axis->max();
    }
    else{
        axis_MoveStep=0;
    }
    time_chart->scroll(axis_MoveStep,0);//滚动图标
    //当前值
    if(simulate_flag){
        ui->label_s_current->setText(QString::number(m_y));
    }
    else {
        ui->label_voltages_current->setText(QString::number(m_y));
    }

    past_voltages.append(QString::number(m_y));//存储数据

    //频域波形 ---------------------------------
    Complex temp_in;
    temp_in.rl = m_y;
    if(signal_pointer>=1024){
        in_time.removeFirst();
    }
    in_time.append(temp_in);//将时域数据装入fft计算的容器里

    if(m_fft.is_power_of_two(in_time.length())){//判断数据源长度是否符合计算要求
        qDebug()<<"时域转频域个数："<<(in_time.length())<<endl;
        past_freq_am.clear();
        past_freq_Hz.clear();
        //调用接口 生成频域的 out_ 数据
        QVector<Complex> out_freq(in_time.length());
        m_fft.fft1(in_time,in_time.length(),out_freq);

        max_am=0;//最大幅值
        freq_series->clear();//清除上一次的频域波形
        peak_temp=0;//极大值

        for(auto i = 0;i<(in_time.length())/2;i++){
            //计算频域波形点
            out_freq_kHz=i/sampling_time/(in_time.length())*1000;//x轴单位Hz
            out_freq_am=qSqrt(qPow((out_freq.at(i).im),2) +qPow((out_freq.at(i).rl),2))/(in_time.length())*2;
//            if(out_freq_kHz==0){
//                out_freq_am*=2;
//            }
            //绘制频域波形
            freq_series->append(out_freq_kHz,out_freq_am);
            past_freq_am.append(QString::number(out_freq_am) );
            past_freq_Hz.append(QString::number(out_freq_kHz));
            //            qDebug()<<"频域复数："<<out_freq.at(i).rl<<out_freq.at(i).im<<endl;
            //            qDebug()<<"频域波形："<<out_freq_kHz<<out_freq_am<<endl;
            //求最大值
            if(max_am<out_freq_am){
                max_kHz=out_freq_kHz;
                max_am=out_freq_am;
                if(simulate_flag){
                    ui->label_s_freq->setText(QString::number(max_kHz));
                    ui->label_s_am->setText(QString::number(max_am));
                }
                else {
                    ui->label_voltages_Hz->setText(QString::number(max_kHz));
                    ui->label_voltages_am->setText(QString::number(max_am));
                }

            }



        }
        if((max_kHz >= freq_axis->max()) || (max_kHz <= freq_axis->min())){
            axis_MoveStep=max_kHz - freq_axis->max();
        }else {
            axis_MoveStep=0;
        }
        if((freq_axis->min()+axis_MoveStep)<0){
            axis_MoveStep-=(axis_MoveStep+freq_axis->min());
        }
        freq_chart->scroll(axis_MoveStep,0);

    }
    signal_pointer++;

}

void MainWindow::work_enable(bool en)//控制采样按钮是否可用
{
    ui->btn_sample_start->setEnabled(en);
    ui->btn_sample_pause->setEnabled(en);
    ui->btn_sample_stop->setEnabled(en);
    ui->lineEdit_sample_time->setEnabled(en);
    ui->btn_sample_goon->setEnabled(en);

}

void MainWindow::readTxt(QString path)//读取txt文件中的数据
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly|QFile::Text)){
        //        QMessageBox::warning(this,"Error","read file yangpin.txt default:%1".arg(file.errorString()));
        qDebug()<<"Error:file can't be read "<<endl;
    } //判断读取失败与否
    voltages.clear();
    QTextStream in(&file);

    QString str;
    QStringList tempStrings;

    while(!in.atEnd()){
        //    qDebug()<<in.readLine();
        str.append(in.readLine());//读取每一行
        tempStrings=str.split(' ');//取出数据

        for(int i=0;i<tempStrings.length();i++){//转化为数字并放入数组
            voltages.append(tempStrings[i].toDouble());
        }
        str.clear();//每列数据清空
    }

    if(in.atEnd()){
        qDebug()<<"数据读取完毕"<<endl;
        qDebug()<<"共"<<voltages.length()<<"个数据"<<endl;
        file.close();
    }

}

int MainWindow::maxCommonDivisor1(int x, int y)
{
    int a,b,c,temp;
    a=x;
    b=y;
    if (a<b){           //如果a<b，则调换位置。
        temp=a;
        a=b;
        b=temp;
    }
    while(a%b!=0){      //当a除以b的余数不等于0，将较小的数取出来除以余数
        c=a%b;
        a=b;
        b=c;
    }
    return b;

}

int MainWindow::minCommonMultiple(qreal x, qreal y)
{

    return ((int)x)*((int)y)/maxCommonDivisor1((int)x,(int)y);
}

void MainWindow::save_pic(QObject *widget)
{
    QPixmap p = QPixmap::grabWidget(widget);
    QString filename = QFileDialog::getSaveFileName(this, tr("保存波形"),"",tr("*.png;; *.bmp;; *.jpg;; *.tif;; *.GIF")); //选择路径
    if(filename.isEmpty()) {
        return;
    }
    else {
        if(!(p.save(filename) ) ) //保存图像
        {
            QMessageBox::information(this, tr("Failed to save the image"), tr("Failed to save the image!"));
            return;
        }
    }
}

void MainWindow::save_txt(QString str)
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存电压数据"),"",tr("*.txt")); //选择路径
    if(filename.isEmpty()) {
        return;
    }
    else {
        QFile file(filename);

        if(!file.open(QIODevice::WriteOnly  | QIODevice::Text|QIODevice::Append))
        {
            QMessageBox::warning(this,"sdf","can't open",QMessageBox::Yes);
        }
        QTextStream in(&file);
        in<<str+'\n';
        file.close();

    }
}

void MainWindow::simulate_enable(bool en)
{
    ui->groupBox_simulate_1->setVisible(en);
    ui->groupBox_simulate_2->setVisible(en);
    simulate_flag=en;
    //    ui->groupBox_real_1->setVisible(!en);
    //    ui->groupBox_real_2->setVisible(!en);
}

void MainWindow::on_btn_sample_pause_clicked()
{
    myTimer->stop();
    signal_pointer_temp=signal_pointer;//记下暂停时的pointer
    ui->btn_sample_pause->setEnabled(false);
}



void MainWindow::on_btn_sample_stop_clicked()//结束采样
{
    myTimer->stop();
    ui->btn_sample_start->setEnabled(true);
    ui->lineEdit_sample_time->setEnabled(true);
    ui->btn_sample_pause->setEnabled(false);
    ui->btn_sample_goon->setEnabled(false);
    ui->btn_sample_stop->setEnabled(false);
    time_series->clear();//清除原有线条
    in_time.clear();//清除原有频域数据
    freq_series->clear();
    past_voltages.clear();


}

void MainWindow::on_btn_sample_goon_clicked()
{
    signal_pointer=signal_pointer_temp;
    myTimer->start();
    ui->btn_sample_pause->setEnabled(true);
}

void MainWindow::simulate()
{
    simulate_enable(true);
}

void MainWindow::on_btn_simulate_exit_clicked()
{
    simulate_enable(false);
}

void MainWindow::on_btn_simulate_start_clicked()
{
    //获得仿真波形、数据等信息
    QString sstr;
    sstr=ui->lineEdit_set_voltages_am->text();
    if(sstr.isEmpty()){
        QMessageBox::warning(this,"Error","请输入幅值",QMessageBox::Ok);
        return;
    }
    s_voltage_am=sstr.toDouble();
    sstr=ui->lineEdit_set_sampletime->text();
    if(sstr.isEmpty()){
        QMessageBox::warning(this,"Error","请输入采样时间",QMessageBox::Ok);
        return;
    }
    s_sampletime=sstr.toInt();
    sstr=ui->lineEdit_set_addv->text();
    if(sstr.isEmpty()){
        sstr="0";
    }
    s_addv=sstr.toDouble();
    sstr=ui->lineEdit_set_period->text();
    if(sstr.isEmpty()){
        QMessageBox::warning(this,"Error","请输入波形周期",QMessageBox::Ok);
        return;
    }
    s_waveperiod=sstr.toInt();

    if(ui->radio_wave_sin_6->isChecked()){
        s_wave=1;
    }
    else {
        if(ui->radio_wave_square_6->isChecked()){
            s_wave=2;
        }
        else {
            if(ui->radio_wave_triangle_6->isChecked()){
                s_wave=3;
            }
            else {
                QMessageBox::warning(this,"Error","请选择波形",QMessageBox::Ok);
                return;
            }
        }
    }

    sampling_time = s_sampletime;//获得采样时间
    qDebug()<<"采样时间："<<sampling_time<<endl;
    myTimer->setInterval(sampling_time);
    time_chart->axisX()->setRange(0,sampling_time*100);
    m_x=0 -sampling_time;
    m_y=0;
    signal_pointer=0;
    myTimer->start();

    //调整按钮
    ui->btn_simulate_pause->setEnabled(true);
    ui->btn_simulate_over->setEnabled(true);
    ui->btn_simulate_exit->setEnabled(false);
    ui->btn_simulate_start->setEnabled(false);
    s_pause_flag=true;
    ui->radio_wave_sin_6->setEnabled(false);
    ui->radio_wave_square_6->setEnabled(false);
    ui->radio_wave_triangle_6->setEnabled(false);



}

void MainWindow::on_btn_simulate_over_clicked()
{
        myTimer->stop();
        time_series->clear();//清除原有线条
        in_time.clear();//清除原有频域数据
        freq_series->clear();
        past_voltages.clear();

        //调整按钮
        ui->btn_simulate_pause->setEnabled(false);
        ui->btn_simulate_over->setEnabled(false);
        ui->btn_simulate_exit->setEnabled(true);
        ui->btn_simulate_start->setEnabled(true);

        ui->radio_wave_sin_6->setEnabled(true);
        ui->radio_wave_square_6->setEnabled(true);
        ui->radio_wave_triangle_6->setEnabled(true);

}

void MainWindow::on_btn_simulate_pause_clicked()
{
    if(s_pause_flag){
        myTimer->stop();
        signal_pointer_temp=signal_pointer;//记下暂停时的pointer
        s_pause_flag= false;
    }
    else {
        signal_pointer=signal_pointer_temp;
        myTimer->start();
        s_pause_flag= true;
    }


}
