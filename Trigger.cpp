#include "Trigger.h"

trigger::trigger()
{

	TriggerChar = '0';
	State = -1;
}

trigger::trigger(vector<Point> contour, char Char) {

	AreaContour = contour;
	State = -1;
	TriggerChar = Char;
}

trigger::trigger(vector<Point> contour) {

	AreaContour = contour;
	State = -1;
}

// 设置发送的信号
void trigger::setChar() {
	cout << "what signal you want this trigger to send?" << endl;
	char temp_c;
	cin >> temp_c;
	TriggerChar = temp_c;
}

void trigger::print() const {
	cout << "Char to send: " << TriggerChar << endl;
	for (int i = 0; i < AreaContour.size(); i++) {
		cout << "(" <<AreaContour[i].x << " " << AreaContour[i].y << ")" << '\t';
	}
	cout << endl << endl;
}

void trigger::drawOnFrame(Mat& src) const{
	if (AreaContour.size() > 0) {
		for (int i = 1; i < AreaContour.size(); i++)
		{
			line(src, AreaContour[i], AreaContour[i - 1], Scalar(0, 255, 0));
		}
	}
}

// 检查手指位置touchpoints是否在触发区域AreaContour内
bool  trigger::checkTrigger(const vector<Point>& touchPoints, Mat& cur_frame) const {
	int temp = 0;
	vector<vector<Point>> TriggerContour;
	TriggerContour.push_back(AreaContour);

	if (touchPoints.size() > 0) {
		for (size_t j = 0; j < touchPoints.size(); j++) {
			double trigger = pointPolygonTest(AreaContour, touchPoints[j], false);
			if (trigger >= 0)
			{
				temp++;
			}
		}
		if (temp > 0) {
			drawContours(cur_frame, TriggerContour, -1, Scalar(0, 255, 0), CV_FILLED, 8);
			// State = 1;
			cout << TriggerChar << " has been triggered" << endl;
			return true;
		}
		else {
			return false;
		}

	}
	else {
		// State = -1;
		return false;
	}
}
