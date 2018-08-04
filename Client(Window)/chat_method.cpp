#include"chat_method.h"

#include <cstdio>
#include <thread>
#include <winsock.h>

#include<iostream>

#include "util.h"
#include "thread_pool.h"

namespace gochat {

	void Init() {
		ConnectServer my_server;

		if ((g_user_client_sockfd = my_server.ConnectToServer(kUserServerIp, kUserServerPort)) < 0) {
			perror("�����û�����ʧ��\n");
			exit(1);
		}
		char optype_buff[kBuffSize] = {};//��������
		int optype;
		while (true) {
			puts("������������: 1.��¼ 2.ע�� 3.�رտͻ���");
			scanf("%s", optype_buff);
			if ((optype = ValidOptype(1, 3, optype_buff)) == -1) {
				continue;
			}
			if (optype == 1) {
				bool state = my_server.Login(g_user_client_sockfd);
				if (!state) {
					puts("�˺Ż����������");
				}
				else {
					puts("��¼�ɹ�");
					break;
				}
			}
			else if (optype == 2) {
				bool state = my_server.Register(g_user_client_sockfd);
				if (!state) {
					puts("����������Ӧ���ߴ��˺��ѱ�ע��");
				}
				else {
					puts("ע��ɹ�,�ѵ�¼");
					break;
				}
			}
			else if (optype == 3) {
				closesocket(g_user_client_sockfd);
				exit(0);
			}
		}
		CopyBuff(g_user_message, my_server.GetUserMessage());
		UserMessage user_message;
		user_message.ParseFromArray(g_user_message, kBuffSize);
		puts("--------------------------------------------------------------------------------------------------------------------");
		printf("%s ,��ӭ�����!������:%s\n", user_message.nickname().c_str(), GetTime().c_str());
		puts("��������ĸ�����Ϣ:");
		printf("����˺�:%s\n", user_message.user_number().c_str());
		printf("����ǳ�:%s\n��ĸ���ǩ�� : %s\n", user_message.nickname().c_str(), user_message.signture().c_str());
		puts("--------------------------------------------------------------------------------------------------------------------");

	}

	void BeginChat() {
		ConnectServer my_server;

		if ((g_message_client_sockfd = my_server.ConnectToServer(kMessageServerIp, kMessageServerPort)) < 0) {
			perror("������Ϣ����ʧ��\n");
			closesocket(g_user_client_sockfd);
			exit(1);
		}
		send(g_message_client_sockfd, g_user_message, kBuffSize, 0);
		puts("���ڳ�����ȡ������Ϣ.........");
		char offline_message_count_buf[kBuffSize] = {};
		recv(g_message_client_sockfd, offline_message_count_buf, sizeof(offline_message_count_buf), 0);
		int offline_message_count = CharbufToInt(offline_message_count_buf);
		printf("����ȡ�� %d ����Ϣ\n", offline_message_count);
		while (offline_message_count--) {
			char offline_message[kBuffSize] = {};
			recv(g_message_client_sockfd, offline_message, kBuffSize, 0);
			ChatMessage cm;
			cm.ParseFromArray(offline_message, kBuffSize);
			PrintMessage(cm);
		}
		puts("��ȡ���!");
		//�������ڽ�����Ϣ���߳�
		std::thread recv_thead(RecvThread);

		UserMessage user_message;
		user_message.ParseFromArray(g_user_message, kBuffSize);

		while (true) {
			char send_message[kBuffSize] = {} , serialize_send_message[kBuffSize] = {} , send_user_number[kBuffSize] = {};
			puts("������������: 1.˽�� 2.Ⱥ�� 3.����");
			char optype_buff[kBuffSize] = {};//��������
			int optype;
			scanf("%s", optype_buff);
			optype = ValidOptype(1, 3, optype_buff);
			if (optype == -1) continue;
			if (optype == 1) {
				puts("��������Ҫ��ϵ���˵��˺�");
				scanf("%s", send_user_number);
				char server_reply_buff[kBuffSize] = {}, user_number[kBuffSize] = {};
				SerializeUserMessage(send_user_number,UserMessage::CHECK, user_number);
				send(g_user_client_sockfd, user_number, kBuffSize, 0);
				recv(g_user_client_sockfd, server_reply_buff, kBuffSize, 0);
				UserMessage server_reply;
				server_reply.ParseFromArray(server_reply_buff,kBuffSize);
				if (server_reply.reply_result() == UserMessage::FAILED) {
					puts("���޴���!");
					continue;
				}
				puts("������Ҫ���͵���Ϣ");
				while (true) {
					gets_s(send_message);
					if (strlen(send_message) == 0) continue;
					if (strlen(send_message) != 1 || send_message[0] != '\n') break;
				}
				SerializeChatMessage(ChatMessage::PERSONAL_SEND,
					user_message.user_number().c_str(),
					user_message.nickname().c_str(), send_user_number,
					GetTime().c_str(),
					send_message,
					serialize_send_message);
				puts("˽��:");
				printf("%s\n�� : %s\n", GetTime().c_str(), send_message);
				send(g_message_client_sockfd, serialize_send_message, kBuffSize, 0);
			}
			else if (optype == 2) {
				puts("������Ҫ���͵���Ϣ");
				while (true) {
					gets_s(send_message);
					if (strlen(send_message) == 0) continue;
					if (strlen(send_message) != 1 || send_message[0] != '\n') break;
				}
				SerializeChatMessage(ChatMessage::GROUP_SEND,
					user_message.user_number().c_str(),
					user_message.nickname().c_str(),
					"", GetTime().c_str(),
					send_message,
					serialize_send_message);
				send(g_message_client_sockfd, serialize_send_message, kBuffSize, 0);
			}
			else if (optype == 3) {
				closesocket(g_user_client_sockfd);
				closesocket(g_message_client_sockfd);
				exit(0);
			}
			Sleep(15);
		}
	}

	//�����߳�
	void RecvThread() {
		while (true) {
			char recv_buff[kBuffSize] = {};
			if (recv(g_message_client_sockfd, recv_buff, sizeof(recv_buff), 0) <= 0) {
				break;
			}
			ChatMessage cm;
			cm.ParseFromArray(recv_buff, kBuffSize);
			PrintMessage(cm);
		}
	}

}//  namespace gochat

int main() {
	gochat::Init();
	gochat::BeginChat();
	closesocket(gochat::g_user_client_sockfd);
	closesocket(gochat::g_message_client_sockfd);
	return 0;
}