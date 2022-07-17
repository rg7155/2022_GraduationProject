#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <array>
#include <unordered_map>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <thread>
#include <mutex>
#include <list>
#include <fstream>

#pragma comment(lib, "WS2_32.lib")

using namespace std;
using namespace DirectX;

#include "../../Client_Desert/Define.h"
#include "../../Client_Desert/Enum.h"
#include "../../Client_Desert/DirectX_option.h"
#include "Protocol.h"
#include "Timer.h"

constexpr int STAGE1_VERSE = 1;
constexpr int STAGE2_VERSE = 2;
constexpr int STAGE3_VERSE = 3;
constexpr int STAGE4_VERSE = 4;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
bool Check_Collision(BoundingOrientedBox& xmOOBB, BoundingOrientedBox& xmTargetOOBB);

enum OBJECT { OBJECT_MONSTER, OBJECT_BULLET, OBJECT_FOOTHOLD, OBJECT_END };


class CSession;
class CGameObject;
extern unordered_map<int, CSession>	clients; // players + monsters  [0][1]->Player
extern list<CGameObject*>							objects[OBJECT_END]; // monsters & objects
extern unordered_map<string, BoundingOrientedBox>	oobbs;
extern unordered_map<string, vector<float>>			animTimes;

extern unordered_map<WSAOVERLAPPED*, int>	over_to_session;
extern mutex		timer_lock;


