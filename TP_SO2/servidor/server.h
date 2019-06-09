#pragma once

#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strsafe.h>
#include <aclapi.h>
#include "../MappedDll/header.h"


#define MAXT 256
#define MAX_PLAYERS 5
#define MOVE_BALL_UPRIGHT 0
#define MOVE_BALL_UPLEFT 1
#define MOVE_BALL_DOWNRIGHT 2
#define MOVE_BALL_DOWNLEFT 3
#define PIPE_TIMEOUT 5000
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	PLAYERS player;
	DWORD cbRead;
	PLAYERS playerReply;
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, * LPPIPEINST;


//Global Variables
int nPlayers;
int memPlayers;
int nBalls;
int *ballThreadId = NULL;
bool LIVE;
BOOL gameOn;
HANDLE hCanWrite, hCanRead, hCanReadBroad, hCanWriteBroad, hMemPlayers, hFilePlayers, hMemGame, hFileGame;
HANDLE hMutex = NULL, hMutexBroad = NULL;
HANDLE hCons = NULL, hInput = NULL;
HANDLE *hMovBola = NULL;
HANDLE hLogin = NULL;
SHAREDMEM *pBuf = NULL;
GAMEDATA *pGameDataShared = NULL;

//Function prototype
DWORD WINAPI ServerInput();
PLAYERS RecieveRequest();
BOOL HandleAction(PLAYERS pAction, HANDLE pipeConection);
PLAYERS AddPlayerToArray(PLAYERS *pAction);
BOOL RemovePlayerFromArray(PLAYERS *pPlayers);
int getPlayerId(int pid);
int getPlayerUsername(TCHAR * nome);
BOOL BuildReply(PLAYERS *pAction);
DWORD WINAPI BallMovement(LPVOID lparam);

PLAYERS SaveTopTen(PLAYERS *pAction);

BOOL SendBroadcast(BALL *ball);
DWORD WINAPI ServerConsole();
void PrintPlayers();
HANDLE * CreateHandleArray(HANDLE* handle, int* tam);
int * ballIdArray(int* threadId, int* tam);
BALL* CreateBallArray(BALL* ball, int* tam);
BOOL AddBall();
BOOL RemoveBall();
DWORD WINAPI PipeRoutine(LPVOID param);

DWORD WINAPI ServerInputPipes(LPVOID param);
PLAYERS ReceiveRequestFromPipeConnection(HANDLE Pipe);
BOOL SendAnswerToClientPipe(PLAYERS *pAction, HANDLE pipe);

DWORD WINAPI PipeBroadcast(LPVOID param);
BOOL SendBroadcastPipe(BALL *balls);
void Seguranca(SECURITY_ATTRIBUTES* sa);
void Cleanup(PSID pEveryoneSID, PSID pAdminSID, PACL pACL, PSECURITY_DESCRIPTOR pSD);
void ReadGameSettings(wchar_t* fileName);
BRICK * CreateBricks(BRICK *);
//PLAYERS ShowTop10(PLAYERS* pAction);