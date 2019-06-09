// header.h : include file for standard system include files,
// or project specific include files
//
#pragma once

#ifdef MAPPEDDLL_EXPORTS  
#define MAPPEDDLL_IMP_API __declspec(dllexport)   
#else  
#define MAPPEDDLL_IMP_API __declspec(dllimport)   
#endif  

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strsafe.h>
#include <aclapi.h>

#define BufferSize 100
#define BUFFERS 10
#define MAX 256
#define USRVALID 1
#define USRINVALID 2
#define LOGOUTSUCCESS 9
#define SERVERCLOSE 8
#define IPSIZE 15

#define X_STARPOINT_BRICKS 170
#define Y_STARPOINT_BRICKS 120
#define BRICK_WIDTH 34
#define BRICK_HEIGHT 12
#define NUMBER_BRIKS_COL 10
#define NUMBER_BRIKS_LINE 15
#define NUMBER_TOTAL_BRIKS 150
#define STD_BRICK 0
#define BRICK_SPEEDUP 1
#define BRICK_SLOWDOWN 2
#define BRICK_EXTRALIFE 3
#define BRICK_TRIPLE 4
#define MAX_SCREEN_WIDTH 1024
#define MAX_SCREEN_HEIGHT 768

typedef struct {
	TCHAR names[10][MAX];
	int points[10];
}TOP10;

typedef struct
{
    int id;
    int code;
	int life;
    TCHAR username[MAX];
	TCHAR ipAdress[MAX];
	TCHAR command[MAX];
    int pos;
    int score;
	TOP10 top;
} PLAYERS;

typedef struct {
	int in, out;
	PLAYERS	players[10];
}SHAREDMEM;

typedef struct
{
	int x, y;
	int trajectory;
	int accel;
	int id;
}BALL;

//	type distingue magico (!=0) de outros (=0)
//	health distingue standart dos resistentes
typedef struct {
	int x, y;
	int health;
	char type;
}BRICK;

typedef struct {
	int x, y;
	int size;
	int idPlayer;
}BAR;

typedef struct
{
	int in, out;
	int nBalls;
	BALL ball[10][5];
	BRICK bricks[10][NUMBER_TOTAL_BRIKS];
}GAMEDATA;

typedef struct {
	int nBalls;
	BALL ball[5];
}GAMEDATAPIPE;

// TODO: reference additional headers your program requires here

	MAPPEDDLL_IMP_API BOOL TesteDLL();

	MAPPEDDLL_IMP_API BOOL Login(PLAYERS *client);
	MAPPEDDLL_IMP_API BOOL Login(PLAYERS *client, TCHAR *ipAdress);


	MAPPEDDLL_IMP_API PLAYERS RecieveMessage(PLAYERS * client);
	MAPPEDDLL_IMP_API PLAYERS RecieveMessage(PLAYERS * client, TCHAR *ipAdress);

	MAPPEDDLL_IMP_API BOOL SendMessages(PLAYERS * client);
	MAPPEDDLL_IMP_API BOOL SendMessages(PLAYERS* client, TCHAR* ipAdress);

	MAPPEDDLL_IMP_API GAMEDATA RecieveBroadcast(GAMEDATA *pGame);
	MAPPEDDLL_IMP_API GAMEDATAPIPE RecieveBroadcastPipe(GAMEDATAPIPE* pGame, TCHAR* ipAdress, int command);

	MAPPEDDLL_IMP_API void CloseVars();
	MAPPEDDLL_IMP_API void DisconnectPipes();