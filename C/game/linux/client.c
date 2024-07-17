#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory.h>

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <Windows.h>

#define PORT 55555 // ポート番号
#define BUFSIZ 1024

int main()
{
	int numrcv; // 受信した文字列の数
	int i;

	// IP アドレス，ポート番号，ソケット，sockaddr_in 構造体
	char destination[32] = {'\0'};
	int dstSocket;
	struct sockaddr_in dstAddr;

	char buffer[BUFSIZ] = {'\0'}; // 入出力

	// Windowsでのtcp/ip接続
	//WSADATA data;
	//WSAStartup(MAKEWORD(2, 0), &data);

	// 相手先アドレスの入力と送る文字の入力
	printf("サーバーのIP:");
    fgets(destination, 32, stdin);
	//scanf_s("%s", destination, 32);

	// sockaddr_in 構造体のセット
	memset(&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_port = htons(PORT);
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = inet_addr(destination);

	// ソケットの生成
	dstSocket = socket(AF_INET, SOCK_STREAM, 0);

	// 接続失敗
	if (connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr)))
	{
		printf("接続に失敗しました\nプログラムを終了します\n");
		close(dstSocket);
		//WSACleanup();
		return (-1);
	}

	// 接続成功
	//printf("接続成功\n", destination);
	printf("接続成功\n");

	/*
	recv(dstSocket, buffer, sizeof(buffer), 0);
	printf("%s\n", buffer);
	*/

	// ゲームの処理
	while (1)
	{
		printf("コマンドを入力>");
		scanf("%s", buffer);
		send(dstSocket, buffer, BUFSIZ, 0);
		numrcv = recv(dstSocket, buffer, sizeof(char) * BUFSIZ, 0);

		// 切断
		if (numrcv == 0 || numrcv == -1)
		{
			printf("サーバーから切断されました\nプログラムを終了します\n");
			break;
		}
		if (strlen(buffer) == 1)
		{
			i = atoi(buffer);
			if (i == 0)
				printf("移動しました\n");
			else if (i == 1)
				printf("壁があって移動できません\n");
			else if (i == 2)
				printf("パラメータに間違いがあります\n");
			else if (i == 9)
				printf("弾に当たりました\n");
		}
		else
			printf("%s\n", buffer);
	}

	//ソケットの終了
	close(dstSocket);
	//WSACleanup();
	return (0);
}