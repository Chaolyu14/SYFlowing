#include "SYFLOWING.h"
using namespace std;

int main()
{

	/**********************************************************/

	WORD sockVision = MAKEWORD(2, 2);
	WSADATA wsadata;
	SOCKET sclient;
	struct sockaddr_in serAddr;
	int ret = -1;
	if (WSAStartup(sockVision, &wsadata) != 0)
	{
		printf("WSA初始化失败\n");
	}

	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("socket客户端创建失败\n");
	}

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVER_PORT_DEFAULT);
	serAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP_LOCALHOST);
	if (connect(sclient, (SOCKADDR *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("socket客户端连接失败\n");
	}

	/**********************************************************/

	vector<trigger> Trigger;
	char cmd;

	static int CameraNum;
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
			testTrigger(CameraMask[CAMERA_NUM], Trigger);
			}
			else {
				cout << "Read config failed" << endl;
			}
			break;

		case 'd':	// exit
			destroyAllWindows();
			closesocket(sclient);
			WSACleanup();
			system("Pause");
			return 0;
		}
	}

	return 0;
}