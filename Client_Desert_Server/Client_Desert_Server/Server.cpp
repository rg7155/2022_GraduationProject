#include "pch.h"


struct MyThread
{
    int iIndex = 0;
    SOCKET sock = 0;
};

HANDLE g_hClientEvent[4];
int g_iWaitClientIndex[4];
int g_iClientCount = 0; //접속한 클라 갯수

POINT g_tPosition[4];
CGameTimer m_GameTimer;

// 체력약 관련
HpPotionInfo g_tHpPotionInfo;
float fPotionCreateTime = 0.f;
LONG iHpPotionIndex;


DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI ServerMain(LPVOID arg);

// 체력약 관련
void CreateHpPotion();
bool SendRecv_HpPotionInfo(SOCKET sock);

CRITICAL_SECTION g_csHpPotion;

void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0)
    {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

//////////////


int main(int argc, char* argv[])
{
    InitializeCriticalSection(&g_csHpPotion);

    srand(unsigned int(time(NULL)));

    // ServerMain 스레드
    CreateThread(NULL, 0, ServerMain, 0, 0, NULL);

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;


    //이벤트 생성
    //클라4개 접속중일때 0이 3번, 1이 0번, 2가 1번, 3이 2번, 이벤트 기다림
    for (int i = 0; i < 4; ++i)
    {
        g_hClientEvent[i] = CreateEvent(NULL, FALSE, (i < 3 ? FALSE : TRUE), NULL);
        g_iWaitClientIndex[i] = (i == 0) ? 3 : i - 1; // 3 0 1 2
    }


    MyThread tThread;
    tThread.iIndex = 0;
    while (1)
    {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET)
        {
            err_display("accept()");
            break;
        }

        tThread.sock = client_sock;
        ++g_iClientCount;
        ++tThread.iIndex;

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient, &tThread, 0, NULL);

        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    DeleteCriticalSection(&g_csHpPotion);

    // 윈속 종료
    WSACleanup();
    return 0;
}




// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
    MyThread* pThread = (MyThread*)arg;
    SOCKET client_sock = (SOCKET)pThread->sock;
    int iCurIndex = pThread->iIndex - 1; //현재 쓰레드 인덱스, 배열 인덱스로 사용해서 -1

    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1)
    {
        if (g_iClientCount >= 2)
            WaitForSingleObject(g_hClientEvent[g_iWaitClientIndex[iCurIndex]], INFINITE);


        // 이스레드가 끝났다면 FALSE 리턴하므로
        if (!SendRecv_HpPotionInfo(client_sock))
        {
            SetEvent(g_hClientEvent[iCurIndex]);
            break;
        }

        SetEvent(g_hClientEvent[iCurIndex]);
    }


    if (--g_iClientCount >= 2)
    {
        for (int i = 0; i < 4; ++i)
        {
            //자신을 참조하던 클라를 찾음
            if (g_iWaitClientIndex[i] == iCurIndex)
            {
                g_iWaitClientIndex[i] = g_iWaitClientIndex[iCurIndex]; //자신이 참조하고있던 인덱스로 바꿔줌
                g_iWaitClientIndex[iCurIndex] = -1;
                break;
            }
        }
    }


    CloseHandle(g_hClientEvent[iCurIndex]);

    // closesocket()
    closesocket(client_sock);
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}

// 서버 프로세스 구현
DWORD WINAPI ServerMain(LPVOID arg)
{
    m_GameTimer.Reset();

    while (true)
    {
        // 1. 체력약 시간재서 보내기
        m_GameTimer.Tick(60.0f);
        CreateHpPotion();
        
    }
}

void CreateHpPotion()
{
    fPotionCreateTime += m_GameTimer.GetTimeElapsed();

    if (fPotionCreateTime >= POTION_TIME)
    {
        EnterCriticalSection(&g_csHpPotion);

        fPotionCreateTime = 0.f;
        g_tHpPotionInfo.thpPotionCreate.cnt = 0;
        g_tHpPotionInfo.thpPotionCreate.bCreateOn = true;
        g_tHpPotionInfo.thpPotionCreate.index = iHpPotionIndex++;
        g_tHpPotionInfo.thpPotionCreate.pos.x = (rand() % 1000) + 50; // 범위 재설정 필요
        g_tHpPotionInfo.thpPotionCreate.pos.y = (rand() % 500) + 50;  // 범위 재설정 필요
        //printf("포션생성\n");
        LeaveCriticalSection(&g_csHpPotion);

    }
}

bool SendRecv_HpPotionInfo(SOCKET sock)
{
    int retval;

    // 동기화 오류
    // 여기서 g_tHpPotionInfo는 공유자원
    // 서로 다른 스레드에서 동시에 접근하므로 객체가 변함
    // Main스레드도 동기화를 해야함
  
    EnterCriticalSection(&g_csHpPotion);

    // 체력약 생성 정보 보내기
    retval = send(sock, (char*)&g_tHpPotionInfo, sizeof(HpPotionInfo), 0);
    if (retval == SOCKET_ERROR)
    {
        err_display("send()");
        LeaveCriticalSection(&g_csHpPotion);

        return FALSE;
    }
    
    // [체력약생성] 현재접속된 모든 클라에 보냈으면 변수 초기화
    if (g_tHpPotionInfo.thpPotionCreate.bCreateOn)
    {
        g_tHpPotionInfo.thpPotionCreate.cnt++;

        // 접속한 클라에 개수만큼 체력약 정보 보냈으면 다시 0으로 리셋
        if (g_tHpPotionInfo.thpPotionCreate.cnt == g_iClientCount)
        {
            ZeroMemory(&g_tHpPotionInfo.thpPotionCreate, sizeof(HpPotionCreate));
        }
    }

    // [체력약삭제] 현재접속된 모든 클라에 보냈으면 변수 초기화
    if (g_tHpPotionInfo.thpPotionDelete.bDeleteOn)
    {
        g_tHpPotionInfo.thpPotionDelete.cnt++;

        // 접속한 클라에 개수만큼 체력약 정보 보냈으면 다시 0으로 리셋
        if (g_tHpPotionInfo.thpPotionDelete.cnt == g_iClientCount)
        {
            ZeroMemory(&g_tHpPotionInfo.thpPotionDelete, sizeof(HpPotionDelete));
        }
    }
    LeaveCriticalSection(&g_csHpPotion);


    // 체력약 충돌 정보 받기
    POTIONRES tHpPotionRes;

    retval = recvn(sock, (char*)&tHpPotionRes, sizeof(POTIONRES), 0);
    if (retval == SOCKET_ERROR)
    {
        err_display("recv()");
        return FALSE;
    }
    else if (retval == 0)
        return FALSE;

    // 충돌일 경우 처리 - 맵에서 삭제 및 다른 클라에 알리기
    if (tHpPotionRes.bCollision)
    {
        //printf("포션삭제\n");

        // 접속 클라 1개인 경우
        if (g_iClientCount == 1)
            return TRUE;

        g_tHpPotionInfo.thpPotionDelete.bDeleteOn = true;
        g_tHpPotionInfo.thpPotionDelete.cnt = 1;
        g_tHpPotionInfo.thpPotionDelete.index = tHpPotionRes.iIndex;


    }

    return TRUE;
}