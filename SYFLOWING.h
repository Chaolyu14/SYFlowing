#ifndef SYFLOWING_H
#define SYFLOWING_H

#include <WinSock2.h>
#pragma  comment(lib,"ws2_32.lib")

#include "Trigger.h"

using namespace std;
using namespace cv;

// 默认服务器端IP地址, loaclhost
#define SERVER_IP_LOCALHOST "127.0.0.1"
// 默认服务器端口号
#define SERVER_PORT_DEFAULT 7000

// 默认摄像头设备参数
#define CAMERA_NUM 0
#define FPS 60
#define FLITER_COEF 100
#define FLITER_COEF_MAX 255

// 读取触摸位置数据，计算触摸的中心 - 手指的位置
vector<Point> ContourCenter(const vector<vector<Point>>& contours);

// 在frame图像上显示手指centers的位置
void displayTouchPoints(Mat& frame, const vector<Point>& centers);

// 读取src,存到队列frames中，并进行平均值计算，返回src-平均值
Mat filteredImage(Mat& input, queue<Mat>& frames, Mat& sum, const int& coef);

/**********************     建立互动点函数    *********************************/

bool getTriggerConfig(vector<trigger>& dst);

bool saveTriggerConfig(vector<trigger>& src);

void CreatTriggers(const Mat& mask, const int& coef, vector<trigger>& Trigger);

/**********************    设置触发区域蒙版    ********************************/

bool getCameraMask(vector<Mat>& CameraMask, vector<int>& fliter_coef);

bool saveCameraMask(const vector<Mat>& CameraMask, const vector<int>& fliter_coef);

void SetInteractiveArea(const int& Num, vector<Mat>& CameraMask, vector<int>& fliter_coef);

/**********************        测试互动点        ******************************/

void testTrigger(const Mat& CameraMask, vector<trigger>& Trigger, const SOCKET& sclient);

void testTrigger(const Mat& CameraMask, vector<trigger>& Trigger);


#endif //SYFLOWING_H