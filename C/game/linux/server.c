//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <winsock2.h>
//#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory.h>

#define PORT 55555 // ポート番号
#define CMDSIZE 32 // 分割された文字列の大きさ
#define MAP_X 23
#define MAP_Y 12

//winsock版からの移植
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (0xffff)
#define SOCKET int

//#define BUFSIZ 1024

// グローバル変数
int id = 0;				 // プレイヤーID振り分け用(1~30がIDとなる)
int mstat[MAP_Y][MAP_X]; // マップの状態を各配列に入れる(共有リソース)
char mstatstr[BUFSIZ];	 // mstatのデータを文字列にしてプレイヤーに送信する(共有リソース)
int hit = 0;			 // どのプレイヤーに当たったか
int connectioncount = 0; // 接続数
int gamestats = 0;		 // ゲームの状態を定義(0:ゲーム中ではない,1:ゲーム中)
int crash = 0;

pthread_mutex_t mutex; // クリティカルセクション制御用(Linux)

// プロトタイプ宣言
void position_set(int pid);				 // プレイヤーの初期の位置決め
void mstat_to_string(int pid);			 // mstatのデータを文字列化しながらIDに合わせてmstatstrに入れる値を変える
int move(char *s, int pid);				 // 移動
void game_clear(int pid);				 // ゲームのクリア
void initialize();						 // ゲーム画面の初期設定
void search(int pid);					 // 相手を探す
int mine(char *s, int pid, int mine);	 // 機雷投下
void map();								 // マップを表示
int fire(char *s, int pid);				 // 発射

//unsigned __stdcall timercount(void *hp); // タイマー

// プレイヤー情報の構造体
typedef struct
{
	int pid;	 // プレイヤーのID
	int hp;		 // 戦艦のHP(0になった時点でゲームオーバー)
	int mine;	 // 機雷の残り
	int command; // 行動が可能か
} playerinfo_t;

// 通信はここでする(マルチスレッド)
int* command_process(void *so)
{
	playerinfo_t player = {id, 5, 3, 1}; // プレイヤーのID,HP(0でゲームオーバー),機雷の残り数,行動可能か
	if (player.pid == id - 1)
		player.pid++;
	SOCKET s = *((SOCKET *)so);
	int numrcv;			 // 受信した文字列の文字数
	char buffer[BUFSIZ]; // 送受信に使用する文字列
	char buffer2[BUFSIZ / 2];
	char cmd1[CMDSIZE], cmd2[CMDSIZE]; // buf[]をcmd1はコマンドに、cmd2はそれの引数に分ける
	int moveprm = 0;				   // 移動のパラメータ
	int fireresult = 0;				   // 発射の結果
	int minecount = player.mine;	   // 機雷が置けたかを判断
	int playercrash = crash;

	// プレイヤーのIDを表示
	printf("playerID is %d\n", player.pid);

	// プレイヤーの位置決め
	position_set(player.pid);

	// 初期配置
	mstat_to_string(player.pid);
	// send(s, mstatstr, BUFSIZ, 0);

	while (1)
	{
		playercrash = crash;

		// 全ての文字列を初期化
		strcpy_s(buffer, BUFSIZ, "\0");
		strcpy_s(buffer2, sizeof(buffer2), "\0");
		strcpy_s(mstatstr, BUFSIZ, "\0");
		strcpy_s(cmd1, CMDSIZE, "\0");
		strcpy_s(cmd2, CMDSIZE, "\0");

		// パケットの受信
		numrcv = recv(s, buffer, sizeof(char) * BUFSIZ, 0);

		// 切断された場合(送られてきた文字列がない)
		if (numrcv == 0 || numrcv == -1)
		{
			printf("ID:%d no response\n", player.pid);
			game_clear(player.pid);
			gamestats = 0;
			connectioncount--;
			goto error;
		}

		sscanf_s(buffer, "%[^':']:%[^'.'].", cmd1, sizeof(cmd1), cmd2, sizeof(cmd2)); //: 区切りでcmd1とcmd2に分ける(.で読み込み終了)

		// exit:eを受けた場合
		if (strcmp(cmd1, "exit") == 0 && strcmp(cmd2, "e") == 0)
		{
			printf("ID:%d was disconnected\n", player.pid);
			game_clear(player.pid);
			gamestats = 0;
			connectioncount--;
			send(s, '\0', BUFSIZ, 0);
			goto error;
		}

		if (gamestats == 0 && hit == 0)
		{
			printf("game is not ready\n");
			strcpy_s(buffer, BUFSIZ, "他のプレイヤーを待機しています");
			player.hp = 5;
			player.mine = 3;
			player.command = 1;
			goto mes_send;
		}
		if (playercrash == 0)
		{
			// コマンドがmoveなら
			if (strcmp(cmd1, "move") == 0)
			{
				moveprm = move(cmd2, player.pid);
				if (moveprm == 9)
				{
					printf("dameged\n");
					player.hp--;
					player.command = 0;
				}
				else if (moveprm == 0)
					player.command = 0;
				else if (moveprm == 10)
				{
					printf("crashed\n");
					crash = 1;
					moveprm = 9;
				}
				sprintf_s(buffer, BUFSIZ, "%d", moveprm);
			}

			// コマンドがIDなら
			else if (strcmp(cmd1, "ID") == 0)
				sprintf_s(buffer, BUFSIZ, "yourID is %d", player.pid);

			// sizeなら
			else if (strcmp(cmd1, "size") == 0 && strcmp(cmd2, "s") == 0)
				strcpy_s(buffer, BUFSIZ, "10,10");

			// mine
			else if (strcmp(cmd1, "mine") == 0)
			{
				if (player.mine == 0)
					strcpy_s(buffer, BUFSIZ, "機雷がありません");
				else
				{
					player.mine = mine(cmd2, player.pid, player.mine);
					if (player.mine == minecount)
						strcpy_s(buffer, BUFSIZ, "機雷が投下できませんでした");
				}
				minecount = player.mine;
				player.command = 0;
			}

			// fire:x,y
			else if (strcmp(cmd1, "fire") == 0)
			{
				fireresult = fire(cmd2, player.pid);
				if (fireresult == 5)
					strcpy_s(buffer, BUFSIZ, "範囲指定が正しくありません");
				if (fireresult == 4)
				{
					strcpy_s(buffer, BUFSIZ, "何にも命中しませんでした");
					player.command = 0;
				}
				if (fireresult == 3)
				{
					strcpy_s(buffer, BUFSIZ, "敵に命中しました");
					player.command = 0;
				}
				if (fireresult == 2)
				{
					strcpy_s(buffer, BUFSIZ, "機雷に命中し、破壊しました");
					player.command = 0;
				}
				if (fireresult == 1)
					strcpy_s(buffer, BUFSIZ, "自分を撃つ事はできません");
			}

			// search
			else if (strcmp(cmd1, "search") == 0)
			{
				strcpy_s(mstatstr, BUFSIZ, "\0");
				search(player.pid);
				strcpy_s(buffer, BUFSIZ, mstatstr);
				player.command = 0;
			}

			// map:m
			else if (strcmp(cmd1, "map") == 0 && strcmp(cmd2, "m") == 0)
			{
				map();
				strcpy_s(buffer, BUFSIZ, mstatstr);
			}

			else
				strcpy_s(buffer, BUFSIZ, "undefined"); // 存在しないコマンドが入力された時に返信する

			// 攻撃を受けた
			if (hit == player.pid)
			{
				player.hp--;
				sprintf_s(buffer2, sizeof(buffer2), "\n敵の弾がこちらに当たりました\nHP残り%d", player.hp);
				strcat_s(buffer, BUFSIZ, buffer2);
				hit = 0;
			}
		}
		else if (playercrash == 1)
		{
			send(s, "衝突して両者沈みました\n引き分け", BUFSIZ, 0);
			printf("ccount=%d\n", connectioncount);
			gamestats = 0;
			connectioncount--;
			game_clear(player.pid);
			if (connectioncount < 1)
				crash = 0;
			goto error;
		}

		if (player.hp <= 0)
		{
			send(s, "あなたの負けです", BUFSIZ, 0);
			game_clear(player.pid);
			gamestats = 0;
			hit = player.pid;
			connectioncount--;
			goto error;
		}

		if (gamestats == 0 && hit != player.pid && hit != 0)
		{
			send(s, "あなたの勝ちです", BUFSIZ, 0);
			hit = 0;
		}
	mes_send:
		if (SOCKET_ERROR == send(s, buffer, BUFSIZ, 0)) // 送信
			goto error;									// 送信に問題があった場合errorへ移行

		if (player.command == 0)
		{
			int time = 3;
			time = time + (5 - player.hp) * 600;
			sleep(time);
			player.command = 1;
		}
	}

error:
	if (SOCKET_ERROR == close(s))		   // sとの接続を切断する
		printf("%s\n", "Socket close failed"); // ソケット閉鎖に失敗した場合

	free(so);
	_endthreadex(0);

	return 0;
}

// ゲームのクリア
void game_clear(int pid)
{
	// 切断要求を出したプレイヤーのIDをmstat上で探し、空白にする。機雷を全て除去する
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			if (mstat[i][j] == pid)
			{
				mstat[i][j] = 32;
			}
			else if (mstat[i][j] > 1999)
			{
				mstat[i][j] = 32;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	return;
}

// 移動
int move(char *s, int pid)
{
	pthread_mutex_lock(&mutex);
	int prm;
	int x, y; // プレイヤーの位置を入れる
	// 命令を出したプレイヤーを探す
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			if (mstat[i][j] == pid)
			{
				x = j;
				y = i;
			}
		}
	}
	// main関数内のcmd2の値によって処理が変わる
	if (strcmp(s, "u") == 0)
	{ // 上に移動
		if (mstat[y - 1][x] == 32)
		{
			mstat[y][x] = 32;
			mstat[y - 1][x] = pid;
			prm = 0;
		}
		else if (mstat[y - 1][x] > 999)
		{
			mstat[y][x] = 32;
			mstat[y - 1][x] = pid;
			prm = 9;
		}
		else if (mstat[y - 1][x] < 9)
		{
			mstat[y][x] = 32;
			mstat[y - 1][x] = pid;
			prm = 10;
		}
		else
			prm = 1;
	}

	else if (strcmp(s, "d") == 0)
	{ // 下に移動
		if (mstat[y + 1][x] == 32)
		{
			mstat[y][x] = 32;
			mstat[y + 1][x] = pid;
			prm = 0;
		}
		else if (mstat[y + 1][x] > 999)
		{
			mstat[y][x] = 32;
			mstat[y + 1][x] = pid;
			prm = 9;
		}
		else if (mstat[y + 1][x] < 9)
		{
			mstat[y][x] = 32;
			mstat[y + 1][x] = pid;
			prm = 10;
		}
		else
			prm = 1;
	}

	else if (strcmp(s, "l") == 0)
	{ // 左に移動
		if (x != 2)
		{ // 左端以外
			if (mstat[y][x - 2] == 32)
			{
				mstat[y][x] = 32;
				mstat[y][x - 2] = pid;
				prm = 0;
			}
			else if (mstat[x - 2][x] > 999)
			{
				mstat[y][x] = 32;
				mstat[y][x - 2] = pid;
				prm = 9;
			}
			else if (mstat[x - 2][x] < 9)
			{
				mstat[y][x] = 32;
				mstat[y][x - 2] = pid;
				prm = 10;
			}
		}
		else
			prm = 1;
	}

	else if (strcmp(s, "r") == 0)
	{ // 右に移動
		if (x != 20)
		{ // 右端以外
			if (mstat[y][x + 2] == 32)
			{
				mstat[y][x] = 32;
				mstat[y][x + 2] = pid;
				prm = 0;
			}
			else if (mstat[x + 2][x] > 999)
			{
				mstat[y][x] = 32;
				mstat[y][x + 2] = pid;
				prm = 9;
			}
			else if (mstat[x + 2][x] < 9)
			{
				mstat[y][x] = 32;
				mstat[y][x + 2] = pid;
				prm = 10;
			}
		}
		else
			prm = 1;
	}
	else
	{
		prm = 2; // どれにも該当しなかった場合
		goto error;
	}
	printf("ID=%d:x=%d y=%d\n", pid, x, y);
error:
	pthread_mutex_unlock(&mutex);
	return prm;
}

// mstatのデータを文字列化しながらIDに合わせてmstatstrに入れる値を変える
void mstat_to_string(int pid)
{
	pthread_mutex_lock(&mutex);
	int k = 0;
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			if (mstat[i][j] == pid) // 自分ならOにする
				mstatstr[k] = 79;
			else if (mstat[i][j] - 1000 == pid) // 地雷なら*にする
				mstatstr[k] = 42;
			else if (mstat[i][j] < 9) // 敵なら空白にする
				mstatstr[k] = 32;
			else // それ以外はそのまま代入
				mstatstr[k] = mstat[i][j];
			k++;
		}
		mstatstr[k] = '\n';
		k++;
	}
	pthread_mutex_unlock(&mutex);
	return;
}

// ゲーム画面の初期設定
void initialize()
{
	int i, j;
	for (i = 0; i < MAP_Y; i++)
	{ // 全てのマスをスペースにする
		for (j = 0; j < MAP_X; j++)
			mstat[i][j] = 32;
	}
	// 一番上の行にアルファベットを表示する
	for (i = 2, j = 2; i < MAP_X - 2; i++)
	{
		if (i % 2 == 0)
		{
			mstat[0][i] = j + 63;
			j++;
		}
	}
	// 一番下の行にアルファベットを表示する
	for (i = 2, j = 2; i < MAP_X - 2; i++)
	{
		if (i % 2 == 0)
		{
			mstat[11][i] = j + 63;
			j++;
		}
	}
	// 一番左の列に番号を表示する
	for (i = 1; i < MAP_Y - 1; i++)
	{
		mstat[i][1] = i + 48;
		if (mstat[i][1] == 58)
		{
			mstat[i][0] = 49;
			mstat[i][1] = 48;
		}
	}
	// 一番右の列に番号を表示する
	for (i = 1; i < MAP_Y - 1; i++)
	{
		mstat[i][22] = i + 48;
		if (mstat[i][22] == 58)
		{
			mstat[i][21] = 49;
			mstat[i][22] = 48;
		}
	}
	return;
}

// プレイヤーの初期の位置決め
void position_set(int pid)
{
	pthread_mutex_lock(&mutex);
	srand((unsigned)time(NULL)); // 乱数を生成
	int i, j;
	while (1)
	{
		i = rand() % 10 + 1;
		j = rand() % 21 + 1;

		if (j % 2 == 0)
		{
			if (mstat[i][j] == 32)
			{ // そこが空白なら置く
				mstat[i][j] = pid;
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	return;
}

// 相手を探す
void search(int pid)
{
	pthread_mutex_lock(&mutex);
	int k = 0;
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			if (mstat[i][j] == pid) // 自分ならOにする
				mstatstr[k] = 79;
			else if (mstat[i][j] > 1000) // 地雷なら*にする
				mstatstr[k] = 42;
			else if (mstat[i][j] < 9) // 敵なら!にする
				mstatstr[k] = 33;
			else // それ以外はそのまま代入
				mstatstr[k] = mstat[i][j];
			k++;
		}
		mstatstr[k] = '\n';
		k++;
	}
	pthread_mutex_unlock(&mutex);
	return;
}

// 機雷投下
int mine(char *s, int pid, int mine)
{
	pthread_mutex_lock(&mutex);
	int x, y; // プレイヤーの位置を入れる
	// 命令を出したプレイヤーを探す
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			if (mstat[i][j] == pid)
			{
				x = j;
				y = i;
			}
		}
	}
	// main関数内のcmd2の値によって処理が変わる
	if (strcmp(s, "u") == 0)
	{ // 上に機雷設置
		if (mstat[y - 1][x] == 32)
		{
			mstat[y - 1][x] = pid + 1000;
		}
		else
			goto minecount; // 地雷の数を減らさない
	}

	else if (strcmp(s, "d") == 0)
	{ // 下に機雷設置
		if (mstat[y + 1][x] == 32)
		{
			mstat[y + 1][x] = pid + 1000;
		}
		else
			goto minecount; // 地雷の数を減らさない
	}

	else if (strcmp(s, "l") == 0)
	{ // 左に機雷設置
		if (x != 2)
		{ // 左端以外
			if (mstat[y][x - 2] == 32)
			{
				mstat[y][x - 2] = pid + 1000;
			}
		}
		else
			goto minecount; // 地雷の数を減らさない
	}

	else if (strcmp(s, "r") == 0)
	{ // 右に機雷設置
		if (x != 20)
		{ // 右端以外
			if (mstat[y][x + 2] == 32)
			{
				mstat[y][x + 2] = pid + 1000;
			}
		}
		else
			goto minecount; // 機雷の数を減らさない
	}

	else
	{
		printf("Parameters are abnormal\n"); // どれにも該当しなかった場合
		goto minecount;						 // 機雷の数を減らさない
	}
	mine--;
minecount:
	pthread_mutex_unlock(&mutex);
	return mine;
}

/*
unsigned __stdcall timercount(void *hp)
{
	// return wait;
	return (0);
}
*/

int fire(char *s, int pid)
{
	pthread_mutex_lock(&mutex);
	int result;
	char prmx[CMDSIZE];
	char prmy[CMDSIZE];
	int x = 0;
	int y = 0;
	sscanf_s(s, "%[^','],%[^'.'].", prmx, sizeof(prmx), prmy, sizeof(prmy));
	if (strcmp(prmx, "A") == 0)
		x = 2;
	if (strcmp(prmx, "B") == 0)
		x = 4;
	if (strcmp(prmx, "C") == 0)
		x = 6;
	if (strcmp(prmx, "D") == 0)
		x = 8;
	if (strcmp(prmx, "E") == 0)
		x = 10;
	if (strcmp(prmx, "F") == 0)
		x = 12;
	if (strcmp(prmx, "G") == 0)
		x = 14;
	if (strcmp(prmx, "H") == 0)
		x = 16;
	if (strcmp(prmx, "I") == 0)
		x = 18;
	if (strcmp(prmx, "J") == 0)
		x = 20;
	if (x == 0)
	{
		result = 5;
		pthread_mutex_unlock(&mutex);
		return result;
	}
	y = atoi(prmy);
	if (y == 0)
	{
		result = 5;
		pthread_mutex_unlock(&mutex);
		return result;
	}
	printf("ID=%d:x=%d y=%d\n", pid, x, y);
	if (1 < x && x < 21 && 0 < y && y < 11)
	{
		if (mstat[y][x] == pid)
			result = 1;
		else if (mstat[y][x] > 999)
		{
			result = 2;
			mstat[y][x] = 32;
		}
		else if (mstat[y][x] < 9)
		{
			result = 3;
			hit = mstat[y][x];
		}
		else
			result = 4;
	}
	else
		result = 5;
	pthread_mutex_unlock(&mutex);
	return result;
}

// mapコマンドを実行
void map()
{
	pthread_mutex_lock(&mutex);
	int mstatcpy[MAP_Y][MAP_X]; // mstatをコピー
	int i, k = 0;
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			mstatcpy[i][j] = mstat[i][j];
		}
	}
	// 一番上の行を壁にする
	for (i = 0; i < MAP_X; i++)
		mstatcpy[0][i] = 50;
	// 一番下の行を壁にする
	for (i = 0; i < MAP_X; i++)
		mstatcpy[11][i] = 50;
	// 一番左の列を壁にする
	for (i = 0; i < MAP_Y; i++)
	{
		mstatcpy[i][0] = 50;
		mstatcpy[i][1] = 50;
	}
	// 一番右の列を壁にする
	for (i = 0; i < MAP_Y; i++)
	{
		mstatcpy[i][21] = 50;
		mstatcpy[i][22] = 50;
	}
	for (int i = 0; i < MAP_Y; i++)
	{
		for (int j = 0; j < MAP_X; j++)
		{
			// そのマスが
			if (mstat[i][j] < 9) // プレイヤーならプレイヤーのIDにする
				mstatstr[k] = mstat[i][j] + 48;
			else if (mstat[i][j] > 999) // 機雷なら3にする
				mstatstr[k] = 51;
			else if (mstatcpy[i][j] == 50)
				mstatstr[k] = mstatcpy[i][j];
			else if (mstat[i][j] == 32) // 空白なら0にする
				mstatstr[k] = 48;
			k++;
			mstatstr[k] = 44;
			k++;
		}
		mstatstr[k] = '\n';
		k++;
	}
	pthread_mutex_unlock(&mutex);
	return;
}

int main()
{
	initialize();					// 画面の初期設定
	struct sockaddr_in srcAddr;		// sockaddr_in 構造体

	/*
	WSADATA wsaData;				// winsockの起動

	// winsockの初期化
	while (WSAStartup(WINSOCK_VERSION, &wsaData))
	{
		// WinSockの初期化失敗
		WSACleanup();
		printf("Initialization failed\n");
		sleep(1);
	}
	*/

	// ソケット作成
	int listen_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listen_s)
	{ // ソケットのオープン失敗
		printf("Socket open failed\n");
		return 0;
	}

	// 構造体初期化
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_port = htons(PORT);
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// バインド
	int len = sizeof(struct sockaddr);
	if (bind(listen_s, (struct sockaddr *)&srcAddr, len))
	{
		printf("Bind failed\n");
		return 0;
	}

	// ソケットを接続待ちの状態にする
	if (SOCKET_ERROR == listen(listen_s, SOMAXCONN))
	{
		printf("Listen failed\n");
		return 0;
	}

	while (1)
	{
		// 接続の受付
		if (connectioncount < 2)
			printf("connection waiting\n");
		SOCKET *s; // 相手
		while (NULL == (s = (SOCKET *)malloc(sizeof(SOCKET))))
			;
		// 接続待ちのリクエストを一つ取り出し、接続を確立する
		*s = accept(listen_s, (struct sockaddr *)&srcAddr, &len);
		printf("palyer connected\n");
		// IDを設定
		if (INVALID_SOCKET != *s && connectioncount < 2)
		{
			if (connectioncount >= 1)
				gamestats = 1;
			if (id == 9)
				id = 0;
			connectioncount++;
			id++;
			pthread_t th[2];
			for (int i = 0; i < 2; i++)
			{
				pthread_create(&th[i],NULL,command_process,s);	// スレッドを立てる
				pthread_join(th[i], NULL);
			}

			close(*s);
			free(s);

			if (th)
				while (!CloseHandle(th))
					; // 参照カウンタを一つ減らす
			else
			{

			}
		}

		else if (connectioncount >= 2)
		{
			printf("このサーバには同時に2人までしか接続できません\n");
			// send(*s, "このサーバには同時に2人までしか接続できません", BUFSIZ, 0);
			send(*s, '\0', BUFSIZ, 0);
			close(*s);
		}

		else
			free(s);
	}

	//終了設定
	close(listen_s);
	//WSACleanup();
	return (0);
}