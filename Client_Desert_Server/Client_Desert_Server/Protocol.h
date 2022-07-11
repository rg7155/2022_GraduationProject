#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

constexpr int SERVER_PORT = 4000;
constexpr int BUFSIZE = 1024;
constexpr int NAME_SIZE = 20;
constexpr int MAX_USER = 100;

constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_OBJECT = 3;
constexpr char SC_REMOVE_OBJECT = 4;
constexpr char SC_MOVE_OBJECT = 5;
constexpr char SC_STAT_CHANGE = 6;
constexpr char SC_MOVE_MONSTER = 7;


// dir
constexpr char DIR_UPRIGHT = 1;
constexpr char DIR_DOWNRIGHT = 2;
constexpr char DIR_DOWNLEFT = 3;
constexpr char DIR_UPLEFT = 4;
constexpr char DIR_RIGHT = 5;
constexpr char DIR_LEFT = 6;
constexpr char DIR_UP = 7;
constexpr char DIR_DOWN = 8;

// race
constexpr char RACE_PLAYER = 0;
constexpr char RACE_GOLEM = 1;
constexpr char RACE_CACTI = 2;
constexpr char RACE_CACTUS = 3;
constexpr char RACE_THORN = 4;

#define DISCONNECT -99.f

#include "../../Client_Desert/Enum.h"

// 프로토콜 정의
#pragma pack(push, 1) // 전체 프로그램에 영향을 미치지 않도록

struct object_anim
{
	float	fPosition;
	float	fWeight;
	bool	bEnable;
};

struct CS_LOGIN_PACKET
{
	char type;
	int size;
};

struct CS_MOVE_PACKET
{
	char			type;
	int				size;
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	unsigned int	client_time;
};

struct CS_ATTACK_PACKET {
	char	type;
	int size;
	int		skill;
};

struct SC_LOGIN_INFO_PACKET
{
	char type;
	int size;
	char id;
	float x, y, z;
};

struct SC_ADD_OBJECT_PACKET
{
	char			type;
	int	size;
	char			id;
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	short			race;		// 몬스터 판별용
	int				hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET
{
	char type;

	int size;
	short race;		// 몬스터 판별용
	short id;
};

struct SC_MOVE_OBJECT_PACKET
{
	char			type;
	int	size;
	short			id;
	short			race;		// 몬스터 판별용
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	unsigned int	client_time;
};

struct SC_STAT_CHANGE_PACKET {
	char	type;
	int size;
	short	race;		// 몬스터 판별용
	int		id;
	int		hp, hpmax;
	char	anim;
};

struct SC_MOVE_MONSTER_PACKET
{
	// 타입추가
	char type;
	int size;
	short id;
	char race;
	XMFLOAT3 xmf3Look;
	XMFLOAT3 xmf3Position;
	int eCurAnim;
	short target_id;
	short hp;
	char verse;
	//float	fElapsedTime;
};

struct SC_MOVE_BULLET_PACKET
{
	// 타입추가
	char type;
	int size;
	short id;
	char race;
	XMFLOAT3 xmf3Look;
	XMFLOAT3 xmf3Position;
	short target_id;
};
#pragma pack (pop)
