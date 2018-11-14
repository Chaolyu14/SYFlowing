#include "SYFLOWING.h"

using namespace std;

void singleCameraMode() {
	vector<trigger> Trigger;
	char cmd;
	vector<Mat> CameraMask;
	vector<int> fliter_coef;
	while (1)
	{
		printf("type in your command\t what to do next？\n");
		printf("\n\ta: Set Interactive Area\n\tb: Create Trigger Points\n\tc: Test triggers\n\td: Exit\n");
		cin >> cmd;

		Trigger.clear();
		CameraMask.clear();

		switch (cmd)
		{
		case 'a':	// 定义互动区域
			SetInteractiveArea(CAMERA_NUM, CameraMask, fliter_coef);
			saveCameraMask(CameraMask, fliter_coef);
			break;

		case 'b':	// 创建触发区域
			getCameraMask(CameraMask, fliter_coef);
			CreatTriggers(CameraMask[CAMERA_NUM], fliter_coef[CAMERA_NUM], Trigger);
			saveTriggerConfig(Trigger);
			break;

		case 'c':	//  测试触发点区域
			getCameraMask(CameraMask, fliter_coef);
			if (getTriggerConfig(Trigger)) {
				for (size_t i = 0; i < Trigger.size(); i++) {
					Trigger[i].print();
				}
				testTrigger(CameraMask[CAMERA_NUM],fliter_coef[CAMERA_NUM], Trigger);
			}
			else {
				cout << "Read config failed" << endl;
			}
			break;

		case 'd':	// exit
			destroyAllWindows();
			return;
		}
	}
}

void multiCameraMode(const int CameraNum) {
	vector<vector<trigger>> Trigger(CameraNum);
	vector<trigger> Trigger_Temp;
	char cmd;
	vector<Mat> CameraMask;
	vector<int> fliter_coef;

	while (1)
	{
		printf("type in your command\t what to do next？\n");
		printf("\n\ta: Set Interactive Area\n\tb: Create Trigger Points\n\tc: Test triggers\n\td: Exit\n");
		cin >> cmd;

		Trigger.clear();
		CameraMask.clear();
		fliter_coef.clear();

		switch (cmd)
		{
		case 'a':	// 定义互动区域
			for (int CamIndex = 0; CamIndex < CameraNum; CamIndex++) {
				SetInteractiveArea(CamIndex, CameraMask, fliter_coef);
			}
			saveCameraMask(CameraMask, fliter_coef);
			break;

		case 'b':	// 创建触发区域
			getCameraMask(CameraMask, fliter_coef);
			for (int CamIndex = 0; CamIndex < CameraNum; CamIndex++) {
				Trigger_Temp.clear();
				CreatTriggers(CamIndex, CameraMask[CamIndex], fliter_coef[CamIndex], Trigger_Temp);
				Trigger.push_back(Trigger_Temp);
				cout << "Camera " << CamIndex + 1 << " setting up finished" << endl;
			}

			saveTriggerConfig(Trigger);
			break;

		case 'c':	//  测试触发点区域
			if (getCameraMask(CameraMask, fliter_coef) && getTriggerConfig(Trigger)) {
				printTriggerConfig(Trigger);
				testTrigger(CameraMask, fliter_coef, Trigger);
			}
			break;
		case 'd':	// exit
			destroyAllWindows();
			return;
		}
	}

}

int main()
{
	int CameraNum;
	cout << "how many cameras are attached?" << endl;
	cin >> CameraNum;

	if (CameraNum == 1) {
		singleCameraMode();
	}
	else if (CameraNum > 1) {
		multiCameraMode(CameraNum);
	}

	return 0;
}