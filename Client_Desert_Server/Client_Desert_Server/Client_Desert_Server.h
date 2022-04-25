#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <array>
#include <unordered_map>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "WS2_32.lib")

using namespace std;
using namespace DirectX;

#include "../../Client_Desert/Define.h"
#include "../../Client_Desert/Enum.h"

#include "../../Client_Desert/DirectX_option.h"

void error_display(const char* msg, int err_no);


