#pragma once

#include "connect_server.h"
#include "messages.pb.h"

// ����socket��protobuf�ľ�̬��
#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libprotocd.lib")

namespace gochat {

	char g_user_message[kBuffSize];

	int g_message_client_sockfd;

	int g_user_client_sockfd;

	// ��ʼ��������Ϣ
	void Init();

	// ��������
	void BeginChat();

	void RecvThread();

}// namespace0 gochat