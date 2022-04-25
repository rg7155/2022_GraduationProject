#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

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
constexpr char SC_MOVE_MONSTER = 6;

#define DISCONNECT -99.f
#include "../../Client_Desert/Enum.h"

// 프로토콜 정의
#pragma pack(push, 1) // 전체 프로그램에 영향을 미치지 않도록


struct player_anim
{

	float	fPosition;
	float	fWeight;
	bool	bEnable;

};

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
	XMFLOAT4X4	xmf4x4World;
	player_anim animInfo[PLAYER::ANIM::END];
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	char id;
	XMFLOAT4X4	xmf4x4World;
	player_anim animInfo[PLAYER::ANIM::END];
};

struct SC_ADD_PLAYER_PACKET
{
	unsigned char size;
	char type;
	char id;
	XMFLOAT4X4	xmf4x4World;
	player_anim animInfo[PLAYER::ANIM::END];
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
	XMFLOAT4X4	xmf4x4World;
	player_anim animInfo[PLAYER::ANIM::END];
};

struct SC_ADD_MONSTER_PACKET
{
	unsigned char size;
	char type;
	char id;
	XMFLOAT4X4	xmf4x4World;
	char name[NAME_SIZE];

};

struct SC_MOVE_MONSTER_PACKET
{
	// 타입추가
	unsigned char size;
	char type;
	short id;
	XMFLOAT3 xmf3Look;
	XMFLOAT3 xmf3Position;
	GOLEM::ANIM eCurAnim;
	//float	fElapsedTime;
};
#pragma pack (pop)
