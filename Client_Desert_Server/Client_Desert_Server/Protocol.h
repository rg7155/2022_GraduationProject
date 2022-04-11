#pragma once
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")

constexpr int SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;
constexpr int NAME_SIZE = 20;
constexpr int MAX_USER = 10;

constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;

#define DISCONNECT -99.f


// 프로토콜 정의
#pragma pack(push, 1) // 전체 프로그램에 영향을 미치지 않도록

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	char direction;	// 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	char id;
	short x, y;
};

struct SC_ADD_PLAYER_PACKET
{
	unsigned char size;
	char type;
	char id;
	short x, y;
	char name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	short id;
};

struct SC_MOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	short id;
	short x, y;
};
struct player_anim
{

	float	fPosition;
	float	fWeight;
	bool	bEnable;

};
struct duoPlayer
{
	unsigned char size;
	XMFLOAT4X4	xmf4x4World;
	player_anim animInfo[ANIM::END];
};


#pragma pack (pop)
