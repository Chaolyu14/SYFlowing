#include "SYFLOWING.h"

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
	vector<Mat> CameraMask;
	vector<int> fliter_coef;

	Trigger.clear();
	CameraMask.clear();

	getCameraMask(CameraMask, fliter_coef);

	if (getTriggerConfig(Trigger)) {
		bool play = true;
		while (play)
		{
			for (size_t i = 0; i < Trigger.size(); i++) {
				Trigger[i].print();
			}
			testTrigger(CameraMask[CAMERA_NUM], Trigger, sclient);
		}
	}
	else {
		cout << "file read error" << endl;
		destroyAllWindows();
		system("Pause");
	}

	closesocket(sclient);
	WSACleanup();
	destroyAllWindows();
	cout << "exiting.." << endl;
	return 0;
}