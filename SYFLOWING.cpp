#include "SYFLOWING.h"
using namespace std;
using namespace cv;
/**************************				******************************/

vector<Point> Mask_pts;

void onMouse(int event, int x, int y, int flags, void* userdata)
{

	if (event == EVENT_LBUTTONUP)
	{
		Mask_pts.push_back(Point(x, y));
	}

}

void on_trackbar(int, void*)
{
}

/**********************    图像处理函数    *************************************/

// 读取触摸位置数据，计算触摸的中心 - 手指的位置
vector<Point> ContourCenter(const vector<vector<Point>>& contours)
{
	// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	//  Get the mass centers: h
	vector<Point> mc(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}
	return mc;

}

// 在frame图像上显示手指centers的位置
void displayTouchPoints(Mat& frame, const vector<Point>& centers)
{
	//转换为灰度图
	//cvtColor(frame, frame, CV_GRAY2RGB);

	RNG rng(12345);
	stringstream loc_temp;
	string location;

	// show all points
	for (size_t i = 0; i < centers.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		circle(frame, centers[i], 1, color, -1, 10, 0);
		loc_temp << centers[i].x << "," << centers[i].y;
		loc_temp >> location;
		putText(frame, location, centers[i], FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
	}
}

// 读取src,存到队列frames中，并进行平均值计算，返回src-平均值
Mat filteredImage(Mat& input, queue<Mat>& frames, Mat& sum, const int& coef)
{

	Mat temp;
	cvtColor(input, temp, CV_RGB2GRAY, CV_32F);
	frames.push(temp);

	if (frames.size() > 10)
	{
		accumulate(temp, sum);
		frames.front().convertTo(frames.front(), CV_32F);
		subtract(sum, frames.front(), sum);
		frames.pop();
		Mat average = Mat::zeros(temp.size(), CV_32F);
		average = sum / 10;

		average.convertTo(average, CV_8U);
		Mat out_frame;
		subtract(temp, average, out_frame);
		// imshow("average", average);
		medianBlur(out_frame, out_frame, 3);
		threshold(out_frame, out_frame, coef, 255, THRESH_BINARY);
		// adaptiveThreshold(out_frame, out_frame, coef, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 5, 7.0);

		return out_frame;
	}
	else
	{
		accumulate(temp, sum);
		return temp;
	}
}

void displayMask(Mat& input, const Mat& contour) {
	vector<Point> pts;
	vector<Point> hull;
	findNonZero(contour, pts);
	convexHull(pts, hull, true);
	polylines(input, hull, true, Scalar(0, 0, 255), 1);
}

/**********************     建立互动点函数    *********************************/

bool getTriggerConfig(vector<trigger>& dst)
{
	FileStorage fs("TriggerConfig.yxml", FileStorage::READ);

	int TriggerNumber;
	fs["TriggerNumber"] >> TriggerNumber;
	cout << TriggerNumber << " trigger areas" << endl;

	if (TriggerNumber == 0) {
		cout << "no trigger points set up" << endl;
		return false;
	}
	else {
		dst.resize(TriggerNumber);
		dst.clear();
	}

	for (int i = 0; i < TriggerNumber; i++) {
		stringstream TriggerName_temp;
		TriggerName_temp << "Trigger_" << i;
		string TriggerName;
		TriggerName_temp >> TriggerName;

		FileNode n = fs[TriggerName];
		vector<Point> AreaContour;
		char TriggerChar;

		TriggerChar = (int)n["TriggerChar"];
		read(n["AreaContour"], AreaContour);
		trigger temp(AreaContour, TriggerChar);
		dst.push_back(temp);

	}
	fs.release();
	return true;
}
bool getTriggerConfig(vector<vector<trigger>>& dst) {
	FileStorage fs("TriggerConfig.yxml", FileStorage::READ);

	int CameraNum;
	fs["CameraNum"] >> CameraNum;
	if (CameraNum == 0) {
		cout << "no Camera config saved" << endl;
		return false;
	}
	else {
		cout << CameraNum << " camera config detected" << endl;
		dst.resize(CameraNum);
		dst.clear();
	}

	for (size_t CamIndex = 0; CamIndex < CameraNum; CamIndex++) {
		stringstream CamName_temp;
		CamName_temp << "Cam" << CamIndex + 1 << "_TriggerNumber";
		string CamName;
		CamName_temp >> CamName;
		int TriggerNumber = (int)fs[CamName];
		cout << TriggerNumber << " Trigger areas on camera " << CamIndex + 1 << endl;

		vector<trigger> Trigger_temp;
		for (int i = 0; i < TriggerNumber; i++) {
			stringstream TriggerName_temp;
			TriggerName_temp << "Cam" << CamIndex + 1 << "_Trigger_" << i + 1;
			string TriggerName;
			TriggerName_temp >> TriggerName;
			cout << "reading " << TriggerName << endl;
			FileNode n = fs[TriggerName];

			vector<Point> AreaContour;
			char TriggerChar;
			TriggerChar = (int)n["TriggerChar"];
			read(n["AreaContour"], AreaContour);
			trigger temp(AreaContour, TriggerChar);
			Trigger_temp.push_back(temp);
		}
		dst.push_back(Trigger_temp);
	}
	cout << "trigger config loaded" << endl;
	fs.release();
	return true;
}

bool saveTriggerConfig(vector<trigger>& src) {
	FileStorage fs("TriggerConfig.yxml", FileStorage::WRITE);
	fs << "TriggerNumber" << (int)src.size();

	for (size_t i = 0; i < src.size(); i++) {
		stringstream TriggerName_temp;
		TriggerName_temp << "Trigger_" << i;
		string TriggerName;
		TriggerName_temp >> TriggerName;
		fs << TriggerName << "{";
		fs << "TriggerChar" << src[i].getTriggerChar();
		fs << "AreaContour" << src[i].getAreaContour();
		fs << "}";
	}
	fs.release();
	return true;
}
bool saveTriggerConfig(vector<vector<trigger>>& src) {
	FileStorage fs("TriggerConfig.yxml", FileStorage::WRITE);
	fs << "CameraNum" << (int)src.size();
	cout << src.size() << " camera are attached" << endl;

	for (size_t CamIndex = 0; CamIndex < src.size(); CamIndex++) {
		stringstream CamName_temp;
		CamName_temp << "Cam" << CamIndex + 1 << "_TriggerNumber";
		string CamName;
		CamName_temp >> CamName;
		fs << CamName << (int)src[CamIndex].size();
		for (size_t i = 0; i < src[CamIndex].size(); i++) {
			stringstream TriggerName_temp;
			TriggerName_temp << "Cam" << CamIndex + 1 << "_Trigger_" << i + 1;
			string TriggerName;
			TriggerName_temp >> TriggerName;
			fs << TriggerName << "{";
			fs << "TriggerChar" << src[CamIndex][i].getTriggerChar();
			fs << "AreaContour" << src[CamIndex][i].getAreaContour();
			fs << "}";
		}
	}
	fs.release();
	return true;
}

void printTriggerConfig(const vector<trigger>& src) {
	cout << src.size() << " trigger area: " << endl;
	for (size_t i = 0; i < src.size(); i++)
	{
		src[i].print();
	}
}
void printTriggerConfig(const vector<vector<trigger>>& src) {
	for (size_t i = 0; i < src.size(); i++)
	{
		cout << "Camera " << i + 1 << " config:" << endl;
		printTriggerConfig(src[i]);
	}
}

void CreatTriggers(const Mat& mask, const int& coef, vector<trigger>& Trigger) {
	cout << "Press 'Space' to start";
	cout << "\ts : Generate trigger area masks and start next";
	cout << "\tq : Save trigger info and exit" << endl;

	VideoCapture inputVideo(CAMERA_NUM);
	Mat cur_frame, out_frame;
	queue<Mat> frames;
	inputVideo >> cur_frame;
	cvtColor(cur_frame, cur_frame, CV_RGB2GRAY);
	Mat	sum = Mat::zeros(cur_frame.size(), CV_32F);
	vector<vector<Point>> contours;
	vector<Point> touchPoints;

	vector<Point> AreaContour;
	bool startRec = false;

	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video device" << endl;
		system("Pause");//清屏  
		return;
	}
	else {
		namedWindow("Camera", WINDOW_AUTOSIZE);
	}

	while (1)
	{
		inputVideo >> cur_frame;
		if (cur_frame.empty()) break;

		Mat f;
		cur_frame.copyTo(f, mask);
		out_frame = filteredImage(f, frames, sum, coef);

		contours.clear();
		touchPoints.clear();
		findContours(out_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		touchPoints = ContourCenter(contours);
		displayTouchPoints(cur_frame, touchPoints);

		if (startRec)
		{
			putText(cur_frame, "start", Point2f(10, 50), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));
			if (touchPoints.size() > 0 && touchPoints[0].x > 0 && touchPoints[0].y > 0)
			{
				AreaContour.push_back(touchPoints[0]);
				cout << "triggered" << endl;
			}
		}
		else {

			putText(cur_frame, "idle", Point2f(10, 50), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));
		}

		if (Trigger.size() > 0)
		{
			// check if touchPoints are inside contour.
			for (size_t i = 0; i < Trigger.size(); i++) {
				Trigger[i].drawOnFrame(cur_frame);
				Trigger[i].checkTrigger(touchPoints, cur_frame);
			}
		}

		// draw the trigger area
		if (AreaContour.size() > 2)
		{
			for (int i = 1; i < AreaContour.size(); i++)
			{
				line(cur_frame, AreaContour[i], AreaContour[i - 1], Scalar(0, 255, 0));
			}
		}

		char key = waitKey(1000 / FPS);

		if (key == ' ')
		{
			startRec = true;
			AreaContour.clear();
		}
		else if (key == 's')
		{
			startRec = false;
			trigger temp(AreaContour);
			temp.setChar();
			Trigger.push_back(temp);
		}
		else if (key == 'd')
		{
			if (AreaContour.size() > 0) {
				AreaContour.pop_back();
			}else if(AreaContour.size() == 0 && Trigger.size() > 0){
				Trigger.pop_back();
			}
		}
		else if (key == 'q')
		{
			destroyWindow("Camera");
			break;
			return;
		}

		imshow("Camera", cur_frame);

	}

}
void CreatTriggers(const int& CamIndex, const Mat& mask, const int& coef, vector<trigger>& Trigger) {
	cout << "Press 'Space' to start";
	cout << "\ts : Generate trigger area masks and start next";
	cout << "\tq : Save trigger info and exit" << endl;

	VideoCapture inputVideo(CamIndex);
	Mat cur_frame, out_frame, sum;
	queue<Mat> frames;
	vector<vector<Point>> contours;
	vector<Point> touchPoints, AreaContour;
	bool startRec = false;

	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video device" << endl;
	}
	else {
		inputVideo >> cur_frame;
		cvtColor(cur_frame, cur_frame, CV_RGB2GRAY);
		sum = Mat::zeros(cur_frame.size(), CV_32F);
		namedWindow("Camera", WINDOW_AUTOSIZE);
	}

	while (1)
	{
		inputVideo >> cur_frame;
		if (cur_frame.empty()) break;

		cur_frame.copyTo(out_frame, mask);
		out_frame = filteredImage(out_frame, frames, sum, coef);

		contours.clear();
		touchPoints.clear();
		findContours(out_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		touchPoints = ContourCenter(contours);
		displayTouchPoints(cur_frame, touchPoints);
		displayMask(cur_frame, mask);

		if (startRec)
		{
			putText(cur_frame, "start", Point2f(10, 50), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));
			if (touchPoints.size() > 0 && touchPoints[0].x > 0 && touchPoints[0].y > 0)
			{
				AreaContour.push_back(touchPoints[0]);
				cout << "triggered" << endl;
			}
		}
		else {
			putText(cur_frame, "idle", Point2f(10, 50), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));
		}

		if (Trigger.size() > 0)
		{
			// check if touchPoints are inside contour.
			for (size_t i = 0; i < Trigger.size(); i++) {
				Trigger[i].drawOnFrame(cur_frame);
				Trigger[i].checkTrigger(touchPoints, cur_frame);
			}
		}

		// draw the trigger area
		if (AreaContour.size() > 2)
		{
			for (int i = 1; i < AreaContour.size(); i++)
			{
				line(cur_frame, AreaContour[i], AreaContour[i - 1], Scalar(0, 255, 0));
			}
		}

		char key = waitKey(1000 / FPS);

		if (key == ' ')
		{
			startRec = true;
			AreaContour.clear();
		}
		else if (key == 's')
		{
			startRec = false;
			trigger temp(AreaContour);
			temp.setChar();
			Trigger.push_back(temp);
		}
		else if (key == 'd')
		{
			if (AreaContour.size() > 0) {
				AreaContour.pop_back();
			}
			else if (AreaContour.size() == 0 && Trigger.size() > 0) {
				Trigger.pop_back();
			}
		}
		else if (key == 'q')
		{
			destroyWindow("Camera");
			return;
		}

		imshow("Camera", cur_frame);

	}

}

/**********************    设置触发区域蒙版    ********************************/

bool getCameraMask(vector<Mat>& CameraMask, vector<int>& fliter_coef)
{
	FileStorage fs("CameraConfig.yxml", FileStorage::READ);
	int CameraNum;
	if (!fs.isOpened())
	{
		cerr << "Failed to open config file" << endl;
		return false;
	}
	else {
		fs["CameraNum"] >> CameraNum;
	}

	if (CameraNum == 0) {
		cout << "no camera config detected" << endl;
		return false;
	}

	for (size_t i = 0; i < CameraNum; i++) {
		
		stringstream MaskName_temp;
		MaskName_temp << "Camera_" << i + 1 << "_Config";
		string MaskName;
		MaskName_temp >> MaskName;
	
		FileNode n = fs[MaskName];
		Mat mask;
		int coef = (int)n["fliter_coef"];
		n["mask_contour"] >> mask;
	
		CameraMask.push_back(mask);
		//imshow(MaskName, CameraMask[i]);
		fliter_coef.push_back(coef);

	}
	cout << "camera config loaded" << endl;
	fs.release();
	return true;
	
}

bool saveCameraMask(const vector<Mat>& CameraMask, const vector<int>& fliter_coef) {
	FileStorage fs("CameraConfig.yxml", FileStorage::WRITE);

	fs << "CameraNum" << (int)CameraMask.size();

	for (size_t i = 0; i < CameraMask.size(); i++) {
		stringstream MaskName_temp;
		MaskName_temp << "Camera_" << i + 1 << "_Config";
		string MaskName;
		MaskName_temp >> MaskName;
		fs << MaskName << "{";
		fs << "fliter_coef" << fliter_coef[i];
		fs << "mask_contour" << CameraMask[i];
		fs << "}";
	}
	cout << "camera config saved" << endl;
	fs.release();
	return true;
}

/*
@parm Num: Camera Number
@parm CameraMask: setup the mask the Numth Camera
*/
void SetInteractiveArea(const int& Num, vector<Mat>& CameraMask, vector<int>& fliter_coef)
{
	cout << "Draw the area for each camera \tpress 's': Generate area mask \tq:Exit" << endl;

	VideoCapture inputVideo(Num);
	Mat cur_frame, out_frame, sum;
	queue<Mat> frames;
	vector<vector<Point>> contours;
	vector<Point> touchPoints;
	int coef = 100;		// 初始化touchbar为零

	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video device" << endl;
	}
	else {
		cout << "Camera " << Num << " open succussful" << endl;
		inputVideo >> cur_frame;
		cvtColor(cur_frame, cur_frame, CV_RGB2GRAY);
		sum = Mat::zeros(cur_frame.size(), CV_32F);
		namedWindow("Camera", WINDOW_AUTOSIZE);
		setMouseCallback("Camera", onMouse, 0);
		createTrackbar("coef", "Camera", &coef, FLITER_COEF_MAX, on_trackbar);
		Mask_pts.clear();
	}

	while (1)
	{
		inputVideo >> cur_frame;
		if (cur_frame.empty()) break;
		out_frame = filteredImage(cur_frame, frames, sum, FLITER_COEF);

		contours.clear();
		touchPoints.clear();
		findContours(out_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		touchPoints = ContourCenter(contours);
		displayTouchPoints(cur_frame, touchPoints);

		char key = waitKey(1000 / FPS);

		if (key == 's' && !cur_frame.empty())
		{
			if (Mask_pts.size() > 2)
			{
				Mat mask(cur_frame.size(), CV_8UC1);
				vector<vector<Point>> CameraMaskContour;
				CameraMaskContour.clear();
				mask = 0;
				CameraMaskContour.push_back(Mask_pts);
				drawContours(mask, CameraMaskContour, 0, Scalar(255), -1);
				Mat masked(cur_frame.size(), CV_8UC3, Scalar(100, 100, 100));
				cur_frame.copyTo(masked, mask);
				CameraMask.push_back(mask);
				fliter_coef.push_back(coef);
				imshow("mask", masked);
			}
		}
		else if (key == 'd' && Mask_pts.size() >= 1) {
			Mask_pts.pop_back();
		}
		else if (key == 'q')
		{
			
			destroyWindow("Camera");
			destroyWindow("mask");
			break;
		}

		if (Mask_pts.size() >= 2)
		{
			for (int i = 1; i < Mask_pts.size(); i++)
			{
				line(cur_frame, Mask_pts[i], Mask_pts[i - 1], Scalar(0, 255, 0));
			}
		}
		/// 结果在回调函数中显示
		on_trackbar(coef, 0);
		imshow("Camera", cur_frame);
	}
}

/**********************        测试互动点        ******************************/
// 单个摄像头
void testTrigger(const Mat& CameraMask, const int coef,  vector<trigger>& Trigger, const SOCKET& sclient) {

	VideoCapture inputVideo(CAMERA_NUM);
	Mat cur_frame, out_frame;
	queue<Mat> frames;
	inputVideo >> cur_frame;
	cvtColor(cur_frame, cur_frame, CV_RGB2GRAY);
	Mat	sum = Mat::zeros(cur_frame.size(), CV_32F);
	vector<vector<Point>> contours;
	vector<Point> touchPoints;
	vector<vector<Point>>  TriggerContour;

	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video device" << endl;
		system("Pause");//清屏  
		return;
	}
	else if (Trigger.size() == 0) {
		cout << "no trigger point" << endl;
		return;
	}
	else {
		cout << Trigger.size() << "Trigger areas" << endl;
		namedWindow("Camera", WINDOW_AUTOSIZE);
	}

	// clear and set up the trigger containers
	for (int i = 0; i < Trigger.size(); i++) {
		TriggerContour.push_back(Trigger[i].getAreaContour());
	}

	int frameNumber = 0;
	char TriggerState[255];

	while (1)
	{
		inputVideo >> cur_frame;
		if (cur_frame.empty()) break;

		for (int i = 0; i < 255; i++) {
			TriggerState[i] = '0';
		}
		int index = 0;

		if (frameNumber < 100) {
			frameNumber++;
		}
		else {

			Mat f;
			cur_frame.copyTo(f, CameraMask);
			out_frame = filteredImage(f, frames, sum, coef);

			contours.clear();
			touchPoints.clear();
			findContours(out_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			touchPoints = ContourCenter(contours);
			displayTouchPoints(cur_frame, touchPoints);

			// draw Trigger areas
			if (Trigger.size() > 0 && touchPoints.size() < 10)
			{
				// check if touchPoints are inside contour.
				for (int i = 0; i < Trigger.size(); i++) {
					Trigger[i].drawOnFrame(cur_frame);
					TriggerState[index] = Trigger[i].getTriggerChar();
					index++;
					TriggerState[index] = '=';
					index++;
					if (Trigger[i].checkTrigger(touchPoints, cur_frame)) {
						TriggerState[index] = '1';
					}
					else {
						TriggerState[index] = '0';
					}
					index++;
					TriggerState[index] = '|';
					index++;
				}
				TriggerState[index] = '\n';
				send(sclient, TriggerState, strlen(TriggerState), 0);

			}

			char key = waitKey(1000 / FPS);

			if (key == 'q')
			{
				destroyWindow("Camera");
				break;
				return;
			}

			imshow("Camera", cur_frame);

		}

	}
}
void testTrigger(const Mat& CameraMask, const int coef, vector<trigger>& Trigger) {

	VideoCapture inputVideo(CAMERA_NUM);
	Mat cur_frame, out_frame;
	queue<Mat> frames;
	inputVideo >> cur_frame;
	cvtColor(cur_frame, cur_frame, CV_RGB2GRAY);
	Mat	sum = Mat::zeros(cur_frame.size(), CV_32F);
	vector<vector<Point>> contours;
	vector<Point> touchPoints;

	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video device" << endl;
		system("Pause");//清屏  
		return;
	}
	else if (Trigger.size() == 0) {
		cout << "no trigger point" << endl;
		return;
	}
	else {
		cout << Trigger.size() << "Trigger areas" << endl;
		namedWindow("Camera", WINDOW_AUTOSIZE);
	}

	int frameNumber = 0;

	while (1)
	{
		inputVideo >> cur_frame;
		if (cur_frame.empty()) break;

		if (frameNumber < 100) {
			frameNumber++;
		}
		else {

			cur_frame.copyTo(out_frame, CameraMask);
			out_frame = filteredImage(out_frame, frames, sum, coef);

			contours.clear();
			touchPoints.clear();
			findContours(out_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			touchPoints = ContourCenter(contours);
			displayTouchPoints(cur_frame, touchPoints);

			// draw Trigger areas
			if (Trigger.size() > 0 && touchPoints.size() < 10)
			{
				// check if touchPoints are inside contour.
				for (int i = 0; i < Trigger.size(); i++) {
					Trigger[i].drawOnFrame(cur_frame);
					Trigger[i].checkTrigger(touchPoints, cur_frame);
				}
			}

			char key = waitKey(1000 / FPS);

			if (key == 'q')
			{
				destroyWindow("Camera");
				return;
			}

			imshow("Camera", cur_frame);

		}

	}
}

// 多个摄像头
void testTrigger(const vector<Mat>& CameraMask, const vector<int> coef, vector<vector<trigger>>& Trigger, const SOCKET& sclient) {
	vector<VideoCapture> inputVideo(CameraMask.size());
	vector<Mat> cur_frame(CameraMask.size()), out_frame(CameraMask.size()), sum(CameraMask.size());
	vector<queue<Mat>> frames(CameraMask.size());
	vector<vector<vector<Point>>> contours(CameraMask.size());
	vector<vector<Point>> touchPoints(CameraMask.size());
	vector<int> frameNumber(CameraMask.size());
	vector<string> WindowName(CameraMask.size());

	for (int CamIndex = 0; CamIndex < CameraMask.size(); CamIndex++) {
		if (Trigger[CamIndex].size() != 0) {
			inputVideo[CamIndex].open(CamIndex);
			inputVideo[CamIndex] >> cur_frame[CamIndex];
			cvtColor(cur_frame[CamIndex], cur_frame[CamIndex], CV_RGB2GRAY);
			sum[CamIndex] = Mat::zeros(cur_frame[CamIndex].size(), CV_32F);

			if (!inputVideo[CamIndex].isOpened())
			{
				cout << "Could not open the input video device" << endl;
				return;
			}
			else if (Trigger[CamIndex].size() == 0) {
				cout << " no trigger point on camera " << CamIndex << endl;
			}
			else {
				cout << Trigger.size() << " Trigger areas on Camera " << CamIndex << endl;
			}
			stringstream ss;
			ss << "Camera" << CamIndex;
			WindowName[CamIndex] = ss.str();
			namedWindow(WindowName[CamIndex], WINDOW_AUTOSIZE);
			frameNumber[CamIndex] = 0;
		}
	}
	char cmd;
	char TriggerState[255];
	for (int i = 0; i < 255; i++) {
		TriggerState[i] = '0';
	}
	while (1)
	{
		int index = 0;
		for (int CamIndex = 0; CamIndex < CameraMask.size(); CamIndex++) {
			if (Trigger[CamIndex].size() == 0) break;
			inputVideo[CamIndex] >> cur_frame[CamIndex];
			if (cur_frame[CamIndex].empty()) break;

			if (frameNumber[CamIndex] < 100) {
				frameNumber[CamIndex]++;
			}
			else {
				cur_frame[CamIndex].copyTo(out_frame[CamIndex], CameraMask[CamIndex]);
				out_frame[CamIndex] = filteredImage(out_frame[CamIndex], frames[CamIndex], sum[CamIndex], coef[CamIndex]);

				contours[CamIndex].clear();
				touchPoints[CamIndex].clear();
				findContours(out_frame[CamIndex], contours[CamIndex], RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
				touchPoints[CamIndex] = ContourCenter(contours[CamIndex]);
				displayMask(cur_frame[CamIndex], CameraMask[CamIndex]);
				displayTouchPoints(cur_frame[CamIndex], touchPoints[CamIndex]);

				// draw Trigger areas
				if (Trigger[CamIndex].size() > 0 && touchPoints[CamIndex].size() < 10)
				{
					// check if touchPoints are inside contour.
					for (int i = 0; i < Trigger[CamIndex].size(); i++) {
						Trigger[CamIndex][i].drawOnFrame(cur_frame[CamIndex]);
						Trigger[CamIndex][i].checkTrigger(touchPoints[CamIndex], cur_frame[CamIndex]);

						TriggerState[index] = Trigger[CamIndex][i].getTriggerChar();
						index++;
						TriggerState[index] = '=';
						index++;
						if (Trigger[CamIndex][i].checkTrigger(touchPoints[CamIndex], cur_frame[CamIndex])) {
							TriggerState[index] = '1';
						}
						else {
							TriggerState[index] = '0';
						}
						index++;
						TriggerState[index] = '|';
						index++;
					}
					imshow(WindowName[CamIndex], cur_frame[CamIndex]);
				}
			}
			cmd = waitKey(1);
			if (cmd == 'q') {
				destroyAllWindows();
				return;
			}

		}
		TriggerState[index] = '\n';
		send(sclient, TriggerState, strlen(TriggerState), 0);

	}
	
}

void testTrigger(const vector<Mat>& CameraMask, const vector<int> coef, vector<vector<trigger>>& Trigger) {
	int CameraNum = CameraMask.size();
	vector<VideoCapture> inputVideo(CameraNum);
	vector<Mat> cur_frame(CameraNum), out_frame(CameraNum), sum(CameraNum);
	vector<queue<Mat>> frames(CameraNum);
	vector<vector<vector<Point>>> contours(CameraNum);
	vector<vector<Point>> touchPoints(CameraNum);
	vector<int> frameNumber(CameraNum);
	vector<string> WindowName(CameraNum);

	for (int CamIndex = 0; CamIndex < CameraNum; CamIndex++) {
		if (Trigger[CamIndex].size() != 0) {
			inputVideo[CamIndex].open(CamIndex);
			inputVideo[CamIndex] >> cur_frame[CamIndex];
			cvtColor(cur_frame[CamIndex], cur_frame[CamIndex], CV_RGB2GRAY);
			sum[CamIndex] = Mat::zeros(cur_frame[CamIndex].size(), CV_32F);

			if (!inputVideo[CamIndex].isOpened())
			{
				cout << "Could not open the input video device" << endl;
				return;
			}
			else if (Trigger[CamIndex].size() == 0) {
				cout << " no trigger point on camera " << CamIndex + 1 << endl;
			}
			else {
				cout << Trigger.size() << " Trigger areas on Camera " << CamIndex + 1 << endl;
			}
			stringstream ss;
			ss << "Camera" << CamIndex + 1;
			WindowName[CamIndex] = ss.str();
			namedWindow(WindowName[CamIndex], WINDOW_AUTOSIZE);
			frameNumber[CamIndex] = 0;
		}
	}
	char cmd;
	while (1)
	{
		for (int CamIndex = 0; CamIndex < CameraNum; CamIndex++) {
			if (Trigger[CamIndex].size() == 0) break;
			inputVideo[CamIndex] >> cur_frame[CamIndex];
			if (cur_frame[CamIndex].empty()) break;

			if (frameNumber[CamIndex] < 100) {
				frameNumber[CamIndex]++;
			}
			else {
				cur_frame[CamIndex].copyTo(out_frame[CamIndex], CameraMask[CamIndex]);
				out_frame[CamIndex] = filteredImage(out_frame[CamIndex], frames[CamIndex], sum[CamIndex], coef[CamIndex]);

				contours[CamIndex].clear();
				touchPoints[CamIndex].clear();
				findContours(out_frame[CamIndex], contours[CamIndex], RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
				touchPoints[CamIndex] = ContourCenter(contours[CamIndex]);
				displayTouchPoints(cur_frame[CamIndex], touchPoints[CamIndex]);

				// draw Trigger areas
				if (Trigger[CamIndex].size() > 0 && touchPoints[CamIndex].size() < 10)
				{
					// check if touchPoints are inside contour.
					for (int i = 0; i < Trigger[CamIndex].size(); i++) {
						Trigger[CamIndex][i].drawOnFrame(cur_frame[CamIndex]);
						Trigger[CamIndex][i].checkTrigger(touchPoints[CamIndex], cur_frame[CamIndex]);
					}
				}

				imshow(WindowName[CamIndex], cur_frame[CamIndex]);
			}
		}

		cmd = waitKey(1);
		if (cmd == 'q') {
			destroyAllWindows();
			return;
		}
	}
}
