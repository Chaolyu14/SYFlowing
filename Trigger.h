#ifndef TRIGGER_H
#define TRIGGER_H
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

// 用于创建修改触发点
class trigger
{
private:

	// 坐标点阵列，可生成触摸点区域蒙版
	vector<Point> AreaContour;

	// char to be sent if triggered
	char TriggerChar;

	// trigger indicator
	int State;

public:

	// 无参数构造函数
	trigger();

	//建立触发点 构造函数
	trigger(vector<Point> contour);
	trigger(vector<Point> contour, char Char);
	

	// 设定触发发送的字符
	void setChar();

	// 打印出Trigger的各项参数
	void print() const;

	// 添加areaContour到图像src上
	void drawOnFrame(Mat& src) const;

	// 检查输入的手指点位是否在触摸区域内
	bool  checkTrigger(const vector<Point>& touchPoints, Mat& cur_frame) const;

	vector<Point> getAreaContour() { return AreaContour; }

	char getTriggerChar() { return TriggerChar;	}

	int getState() { return State; }

};
#endif //TRIGGER_H