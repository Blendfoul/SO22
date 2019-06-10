#include "Server.h"

SHAREDMEM message;
GAMEDATA gamedata;
PLAYERS* players = NULL;
BALL* balls;
BRICK *bricks;
BAR* bars;

TCHAR nome[10][MAXT] = { TEXT("User 1"), TEXT("User 2"), TEXT("User 3"), TEXT("User 4"), TEXT("User 5"), TEXT("User 6"), TEXT("User 7"), TEXT("User 8"), TEXT("User 9"), TEXT("User 10") };
static int values[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 0 };

int nPipes = 0;
int nBars = 0;

int initAccel;
int initLife;

HANDLE hPipe;
HANDLE hPipeBroad;
HANDLE hEventOver;
OVERLAPPED stOverlappedBroad;
HANDLE hBroadcast[MAX_PLAYERS];
HANDLE hEventOverBroad = INVALID_HANDLE_VALUE;
SECURITY_ATTRIBUTES sa;

LPTSTR PIPE_SERVER_NAME = (wchar_t*)TEXT("\\\\.\\pipe\\arkanoid");
LPTSTR PIPE_SERVER_NAME_DATA = (wchar_t*)TEXT("\\\\.\\pipe\\arkanoidDATA");

int _tmain(int argc, TCHAR* argv[])
{
#ifdef UNICODE
	if (_setmode(_fileno(stdin), _O_WTEXT) &&
		_setmode(_fileno(stdout), _O_WTEXT) &&
		_setmode(_fileno(stderr), _O_WTEXT))
	{
		_tprintf(TEXT("UNICODE ON!\n"));
	}
#endif

	nBalls = 1;
	nPlayers = 0;
	memPlayers = 0;
	hMovBola = NULL;
	DWORD threadID[4];
	LIVE = true;
	gamedata.in = 0;
	gamedata.out = 0;
	BOOL fReceiveConnected = FALSE;
	BOOL fWriteConnected = FALSE;
	
	if (argc == 2) {
		_tprintf(TEXT("Reading config file!\n"));
		ReadGameSettings(argv[1]);
	}

	Seguranca(&sa);

	hMutex = OpenMutex(SYNCHRONIZE, false, TEXT("Mutex_1"));

	if (hMutex == NULL)
	{
		hMutex = CreateMutex(NULL, FALSE, TEXT("Mutex_1"));
	}
	else
	{
		_tprintf(TEXT("There is an instance of the server already running. Closing server..."));
		fgetwc(stdin);
		return EXIT_FAILURE;
	}

	players = (PLAYERS*)malloc(sizeof(PLAYERS) * MAX_PLAYERS);

	if (players == NULL)
	{
		_tprintf(TEXT("Error alocating array!\n"));
		return EXIT_FAILURE;
	}

	hMutexBroad = CreateMutex(NULL, FALSE, TEXT("Mutex_2"));
	hCanWrite = CreateSemaphore(NULL, BUFFERS, BUFFERS, TEXT("Semaphore_1"));
	hCanRead = CreateSemaphore(NULL, 0, BUFFERS, TEXT("Semaphore_2"));
	hCanWriteBroad = CreateSemaphore(NULL, BUFFERS, BUFFERS, TEXT("Semaphore_3"));
	hCanReadBroad = CreateSemaphore(NULL, 0, BUFFERS, TEXT("Semaphore_4"));
	hFilePlayers = CreateFile(TEXT("Shared"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFilePlayers == NULL)
		return EXIT_FAILURE;
	hMemPlayers = CreateFileMapping(hFilePlayers, NULL, PAGE_READWRITE, 0, sizeof(SHAREDMEM), TEXT("Shared_1"));

	hFileGame = CreateFile(TEXT("Shared2"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	hMemGame = CreateFileMapping(hFileGame, NULL, PAGE_READWRITE, 0, sizeof(GAMEDATA), TEXT("Shared_2"));

	if (hCanWrite == NULL || hCanRead == NULL || hMemPlayers == NULL || hFileGame == NULL || hMemGame == NULL)
	{
		_tprintf(TEXT("Erro de criação de objectos do windows %lu\n"), GetLastError());
		//_gettchar();
		return EXIT_FAILURE;
	}

	pBuf = (SHAREDMEM*)MapViewOfFile(hMemPlayers, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHAREDMEM));

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Erro de criação da view of file %lu\n"), GetLastError());

		CloseHandle(hCanRead);
		CloseHandle(hCanWrite);
		CloseHandle(hMemPlayers);
		CloseHandle(hMemGame);
		return EXIT_FAILURE;
	}

	pGameDataShared = (GAMEDATA*)MapViewOfFile(hMemGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GAMEDATA));

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Erro de criação da view of file %lu\n"), GetLastError());

		CloseHandle(hCanRead);
		CloseHandle(hCanWrite);
		CloseHandle(hMemPlayers);
		CloseHandle(hMemGame);
		return EXIT_FAILURE;
	}

	

	//Threads

	hInput = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerConsole, NULL, 0, &threadID[0]);

	if (hInput != NULL)
		_tprintf(TEXT("Lancei uma thread com id %d\n"), threadID[0]);
	else
	{
		_tprintf(TEXT("Erro ao criar Thread\n"));
		return EXIT_FAILURE;
	}

	hPipe = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PipeRoutine, &sa, 0, &threadID[2]);

	if (hPipe != NULL)
		_tprintf(TEXT("Lancei uma thread com id %d\n"), threadID[2]);
	else
	{
		_tprintf(TEXT("Erro ao criar Thread\n"));
		return EXIT_FAILURE;
	}

	hPipeBroad = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PipeBroadcast, &sa, 0, &threadID[3]);

	if (hPipeBroad != NULL)
		_tprintf(TEXT("Lancei uma thread com id %d\n"), threadID[3]);
	else
	{
		_tprintf(TEXT("Erro ao criar Thread\n"));
		return EXIT_FAILURE;
	}

	hCons = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerInput, NULL, 0, &threadID[1]);

	if (hCons != NULL)
		_tprintf(TEXT("Lancei uma thread com id %d\n"), threadID[1]);
	else
	{
		_tprintf(TEXT("Erro ao criar Thread\n"));
		return EXIT_FAILURE;
	}

	hMovBola = CreateHandleArray(hMovBola, &nBalls);
	ballThreadId = ballIdArray(ballThreadId, &nBalls);
	balls = CreateBallArray(balls, &nBalls);
	bricks = CreateBricks(bricks);

	hMovBola[nBalls - 1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BallMovement, (LPVOID)& ballThreadId[nBalls - 1], CREATE_SUSPENDED, (LPDWORD)& ballThreadId[nBalls - 1]);

	if (hMovBola != NULL)
		_tprintf(TEXT("Lancei uma thread com id %d\n"), ballThreadId[nBalls - 1]);
	else
	{
		_tprintf(TEXT("Erro ao criar Thread BallMovement"));
		return EXIT_FAILURE;
	}

	WaitForSingleObject(hPipeBroad, INFINITE);
	WaitForSingleObject(hPipe, INFINITE);
	WaitForSingleObject(hInput, INFINITE);
	WaitForSingleObject(hCons, INFINITE);
	WaitForMultipleObjects(nBalls, hMovBola, FALSE, INFINITE);

	UnmapViewOfFile(pBuf);
	
	DisconnectNamedPipe(hBroadcast);
	CloseHandle(hBroadcast);

	//Dinamyc Mem
	if (hMovBola != NULL)
		free(hMovBola);
	free(ballThreadId);
	free(players);
	free(bars);
	free(bricks);

	//Semaphore Players
	CloseHandle(hCanRead);
	CloseHandle(hCanWrite);

	//Semaphore GameData
	CloseHandle(hCanReadBroad);
	CloseHandle(hCanWriteBroad);

	//Mutexes
	if (hMutex != NULL)
		CloseHandle(hMutex);
	if (hMutexBroad != NULL)
		CloseHandle(hMutexBroad);

	//Shared Mem
	CloseHandle(hMemPlayers);
	CloseHandle(hFilePlayers);
	CloseHandle(hMemGame);
	CloseHandle(hFileGame);

	//Threads
	//CloseHandle(hCons);
	//CloseHandle(hMovBola);
	//CloseHandle(hInput);

	return EXIT_SUCCESS;
}

//TODO: Implementar consola para o servidor

DWORD WINAPI ServerConsole()
{
	TCHAR local[MAX];
	local[0] = '\0';

	while (LIVE == true)
	{
		fgetwc(stdin);

		_tprintf(TEXT("Command -> "));
		_tscanf_s(TEXT("%[^\n]s"), local, MAX - 1);

		if (_tcscmp(local, TEXT("close")) == 0)
		{
			if (nPlayers == 0)
			{
				message.players[message.in].code = SERVERCLOSE;
				BuildReply(&message.players[message.in]);
				ReleaseSemaphore(hCanWriteBroad, 1, NULL);
				ReleaseSemaphore(hCanWrite, 1, NULL);
				LIVE = false;
			}
			else
			{
				_tprintf(TEXT("Ainda existem %d utilizadores ligados!\n"), nPlayers);
				//_tcscpy_s(local, _tcslen(TEXT("a")), TEXT("a"));
			}
		}
		else if (_tcscmp(local, TEXT("ball")) == 0)
			for (int i = 0; i < nBalls; i++)
				_tprintf(__T("BALL -> x: %d y: %d id: %d\n"), balls[i].x, balls[i].y, balls[i].id);
		else if (_tcscmp(local, TEXT("users")) == 0)
			PrintPlayers();
		else if (_tcscmp(local, TEXT("add_ball")) == 0)
		{
			AddBall();
		}
		else if (_tcscmp(local, TEXT("rem_ball")) == 0)
		{
			RemoveBall();
		}
		else if (_tcscmp(local, TEXT("start_game")) == 0) 
		{
			
			for (int i = 0; i < nBalls; i++)
				ResumeThread(hMovBola[i]);

			_tprintf(TEXT("Jogo Iniciado!\n"));
		}
	};

	//_tprintf(TEXT("%s %d\n"), local, LIVE);

	return 0;
}

void PrintPlayers()
{
	for (int i = 0; i < nPlayers; i++)
	{
		_tprintf(TEXT("%s\t%d\t%d\n"), players[i].username, players[i].id, players[i].code);
	}
}

DWORD WINAPI ServerInput()
{
	PLAYERS pAction;

	while (LIVE == true)
	{
		PrintPlayers();
		pAction = RecieveRequest();
		HandleAction(pAction, NULL);
	};

	return 0;
}

DWORD WINAPI ServerInputPipes(LPVOID param) {

	PLAYERS pAction = {0};
	HANDLE pipe = (HANDLE) param;

	while (LIVE == true) {
		if (pAction.code == -1)
			break;
		PrintPlayers();
		pAction = ReceiveRequestFromPipeConnection(pipe);
		if (pAction.code != -1)
			HandleAction(pAction, pipe);
		

	};

	DisconnectNamedPipe(pipe);

	return 0;
}

//TODO: Comunicação servidor-cliente

BOOL HandleAction(PLAYERS pAction, HANDLE pipeConection)
{
	BOOL validID = getPlayerId(pAction.id);
	BOOL validUsername = getPlayerUsername(pAction.username);

	//_tprintf(TEXT("ID: %d\tCommand: %s\n"), pAction.id, pAction.command);

	if (!validID && !validUsername && !gameOn && nPlayers < MAX_PLAYERS)
	{
		pAction = AddPlayerToArray(&pAction);
		nBars++;
		bars = CreatePlayerBar(bars, &pAction.id);
		//_tprintf(TEXT("%d"), pAction.code);
		nPlayers++;
		if (pipeConection == NULL)
			memPlayers++;
		pAction.life = initLife;
		_tprintf(TEXT("Vidas %s: %d\n"), pAction.username, pAction.life);
	}
	else if (!validID && !validUsername && nPlayers >= MAX_PLAYERS)
	{
		pAction.code = USRINVALID;
	}
	else if (validID && validUsername && _tcscmp(pAction.command, TEXT("top10")) == 0)
	{

		pAction = SaveTopTen(&pAction);
		//pAction = ShowTop10(&pAction);
		//_tprintf(TEXT("%d\n"), pAction.top.points[2]);
	}

	else if (validID && validUsername && _tcscmp(pAction.command, TEXT("logout")) == 0)
	{
		RemovePlayerFromArray(&pAction);
		_tprintf(TEXT("Removed player: %s\n"), pAction.username);
		nPlayers--;
		if (pipeConection == NULL)
			memPlayers--;
		pAction.code = LOGOUTSUCCESS;
	}

	if (pipeConection == NULL)
		BuildReply(&pAction);
	else
		SendAnswerToClientPipe(&pAction, pipeConection);

	return FALSE;
}

PLAYERS RecieveRequest()
{
	_tprintf(TEXT("Waiting for connection or requests\n"));
	WaitForSingleObject(hCanRead, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

	message = *pBuf;

	if (message.out == 10)
		message.out = 0;

	_tprintf(TEXT("ID: %d OUT: %d COMMAND: %s\n"), message.players[message.out].id, message.out, message.players[message.out].command);

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hCanWrite, 1, NULL);

	_tprintf(TEXT("Client or request request received from %s %d!\n"), message.players[message.out].username, message.players[message.out].id);
	return message.players[message.in];
}

//TODO: Manage de Jogadores no Array de jogadores

BOOL RemovePlayerFromArray(PLAYERS* pPlayers)
{
	int pos = -1;

	for (int i = 0; i < MAX_PLAYERS; i++)
		if (pPlayers->id == players[i].id)
			pos = i;

	if (pos != -1)
	{
		for (int i = pos; i < nPlayers; i++)
		{
			if ((i + 1) != nPlayers)
				players[i] = players[i + 1];
		}

		return true;
	}
	return false;
}

PLAYERS AddPlayerToArray(PLAYERS* pAction)
{
	pAction->score = 0;
	pAction->code = USRVALID;
	players[nPlayers] = *pAction;
	_tprintf(TEXT("Sucess %d\n"), pAction->code);
	return *pAction;
}

int getPlayerId(int pid)
{
	for (int i = 0; i < nPlayers; i++)
	{
		if (players[i].id == pid)
			return 1;
	}
	return 0;
}

int getPlayerUsername(TCHAR* nome)
{
	for (int i = 0; i < nPlayers; i++)
	{
		if (_tcscmp(players[i].username, nome) == 0)
			return 1;
	}
	return 0;
}

//TODO: Envio de respostas

BOOL BuildReply(PLAYERS* pAction)
{
	WaitForSingleObject(hCanWrite, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

	message.players[message.in] = *pAction;

	message.in = (message.in)++ % BUFFERS;
	if (message.in == 10)
		message.in = 0;
	CopyMemory(pBuf, &message, sizeof(SHAREDMEM));

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hCanRead, 1, NULL);
	return true;
}

BOOL SendBroadcast(BALL* ball)//Deprecated in Final release
{

	WaitForSingleObject(hCanWriteBroad, INFINITE);
	WaitForSingleObject(hMutexBroad, INFINITE);

	for (int i = 0; i < nBalls; i++)
		gamedata.ball[gamedata.in][i] = ball[i];
	gamedata.nBalls = nBalls;
	for (int i = 0; i < NUMBER_TOTAL_BRIKS; i++)
		gamedata.bricks[gamedata.in][i] = bricks[i];
	gamedata.nBricks = NUMBER_TOTAL_BRIKS;
	//_tprintf(__T("BALL -> x: %d y: %d IN: %d OUT: %d\n"), gamedata.ball[gamedata.in].x, gamedata.ball[gamedata.in].y, gamedata.in, gamedata.out);

	if (gamedata.out == 10)
		gamedata.out = 0;

	if (gamedata.in == 10)
		gamedata.in = 0;
	else
		gamedata.in++;

	CopyMemory(pGameDataShared, &gamedata, sizeof(GAMEDATA));

	gamedata.out++;

	ReleaseMutex(hMutexBroad);
	ReleaseSemaphore(hCanReadBroad, 1, NULL);
	return true;
}

//TODO: Lógica Jogo

//Tem controlo de esclusao mutua para a var Trajectory, que pode nao pode ser alterada
//no momento em que a thread está a ler o seu valor.
//
// para a versão meta 2, fazer consulta das posicoes dos blocos.
DWORD WINAPI BallMovement(LPVOID lparam)
{
	int* id = (int*)lparam;
	balls[nBalls - 1].x = MAX_SCREEN_WIDTH / 2;  // metade de 1 ecra HD, fica ao centro.
	balls[nBalls - 1].y = MAX_SCREEN_HEIGHT / 2; // metade de 1 ecra HD, fica ao centro.
	balls[nBalls - 1].trajectory = MOVE_BALL_UPRIGHT;
	balls[nBalls - 1].id = *id;
	balls[nBalls - 1].accel = initAccel;
	BALL* ball; // aux
	int indexNumBalls = nBalls - 1; // aux
	bool freeway = TRUE;

	while (LIVE == true && nBalls != indexNumBalls && indexNumBalls >=0)
	{
		// o Tricky disto é que mudamos a posição atual da bola e mudamos a trajetoria (para fazermos o proximo check, and so on...)
		Sleep(2);
		ball = &balls[indexNumBalls];
		
		switch (ball->trajectory)
		{
	
		case MOVE_BALL_UPRIGHT:
			
			//obstaculo canto
			if( ball->x + 1 >= MAX_SCREEN_WIDTH  &&  ball->y-1 <= 0 )
				ball->trajectory = MOVE_BALL_DOWNRIGHT;
			// obstaculo superior
			else if (ball->y - 1 <= 0) {
				ball->x+=1;
				ball->y+=1;
				ball->trajectory = MOVE_BALL_DOWNRIGHT;
			}
			// obstaculo direito
			else if (ball->x + 1 >= MAX_SCREEN_WIDTH) {
				ball->x-=1;
				ball->y-=1;
				ball->trajectory = MOVE_BALL_UPLEFT;
			}
			// tijolo? let's check
			else {
				for (int i = 0; i < NUMBER_TOTAL_BRIKS; i++)
				{
					// tijolo sup
					if (ball->x >= bricks[i].x && ball->x <= bricks[i].x + BRICK_WIDTH){	 // esta enquadrada na coluna dos xx
						if (ball->y - 1 >= bricks[i].y && ball->y - 1 <= bricks[i].y + BRICK_HEIGHT) { // dentro da espessura, do tijolo superior
							bricks[i].health--;
							ball->x++;
							ball->y++;
							freeway = FALSE;
							break;
						}
					}
					// tijolo dir
					else if (ball->y >= bricks[i].y && ball->y <= bricks[i].y + BRICK_HEIGHT) { // enquadramento dos yy
							if (ball->x + 1 >= bricks[i].x && ball->x <= bricks[i].x + BRICK_WIDTH) {		// dentro da lateral xx ?
								bricks[i].health--;
								ball->x--;
								ball->y--;
								freeway = FALSE;
								break;
							}
					}
				}
			}
			// livre
			if (freeway){
				ball->x+=1;
				ball->y-=1;
			}
			
			//tornar a variavel freeWay virgem do caso dos tijolos
			freeway = TRUE;
			break;
		case MOVE_BALL_DOWNRIGHT:
			//obstaculo canto
			if ( ball->x + 1 >= MAX_SCREEN_WIDTH && ball->y + 1 >= MAX_SCREEN_HEIGHT )
				ball->trajectory = MOVE_BALL_UPLEFT;
			// obstaculo inferior
			else if (ball->y + 1 >= MAX_SCREEN_HEIGHT) {
				ball->x+= 1;
				ball->y-= 1;
				ball->trajectory = MOVE_BALL_UPRIGHT;
			}
			// obstaculo direito
			else if (ball->x + 1 >= MAX_SCREEN_WIDTH) {
				ball->x-= 1;
				ball->y+= 1;
				ball->trajectory = MOVE_BALL_DOWNLEFT;
			}
			// embateu barra
			//else if(ball->x-1 >= barra x && ball->x+1 <= barra x + sizebarra)
			//	if (ball->y + 1 >= barra y) {
			//		ball->x++;
			//		ball->y--;
			//freeway = FALSE;
			//		break; // para o ciclio das barras
			//	}
			else{
					for (int i = 0; i < NUMBER_TOTAL_BRIKS; i++)
					{
						//tijolo inferior
						if (ball->x >= bricks[i].x && ball->x <= bricks[i].x+BRICK_WIDTH) {  // enquadramento xx
							if (ball->y+1 >= bricks[i].y ) {
								bricks[i].health--;
								ball->x++;
								ball->y--;
								freeway = false;
								break;
							}
						}
						else if (ball->y >= bricks[i].y && ball->y <= bricks[i].y + BRICK_HEIGHT ) {   // enquadramento yy
							if (ball->x+1 >= bricks[i].x ) {
								bricks[i].health--;
								ball->x--;
								ball->y++;
								freeway = false;
								break;
							}
						}
					}
			}
			//// livre
			if(freeway){
				ball->x+= 1;
				ball->y+= 1;
			}

			freeway = TRUE;
			break;
		
		case MOVE_BALL_DOWNLEFT:
			//obstaculo canto
			if (ball->x - 1 <= 0 && ball->y + 1 >= MAX_SCREEN_HEIGHT )
				ball->trajectory = MOVE_BALL_UPRIGHT;
			// obstaculo inferior
			else if (ball->y + 1 >= MAX_SCREEN_HEIGHT) {
				ball->x-= 1;
				ball->y-= 1;
				ball->trajectory = MOVE_BALL_UPLEFT;
			}
			// obstaculo esquerdo
			else if (ball->x - 1 <= 0) {
				ball->x+= 1;
				ball->y+= 1;
				ball->trajectory = MOVE_BALL_DOWNRIGHT;
			}
			// embateu barra
			//else if(ball->x-1 >= barra x && ball->x+1 <= barra x + sizebarra)
			//	if (ball->y + 1 >= barra y) {
			//		ball->x++;
			//		ball->y--;
			//freeway = FALSE;
			//		break; // para o ciclio das barras
			//	}
			else {
				for (int i = 0; i < NUMBER_TOTAL_BRIKS; i++)
				{
					//tijolo inferior
					if (ball->x >= bricks[i].x && ball->x <= bricks[i].x + BRICK_WIDTH) {  // enquadramento xx
						if (ball->y + 1 >= bricks[i].y) {
							bricks[i].health--;
							ball->x--;
							ball->y--;
							freeway = false;
							break;
						}
					}
					else if (ball->y >= bricks[i].y && ball->y <= bricks[i].y + BRICK_HEIGHT) {   // enquadramento yy
						if (ball->x - 1 <= bricks[i].x + BRICK_WIDTH) {
							bricks[i].health--;
							ball->x++;
							ball->y--;
							freeway = false;
							break;
						}
					}
				}
			}
			//// livre
			if (freeway) {
				ball->x += 1;
				ball->y += 1;
			}
			freeway = TRUE;
			
		break;
		case MOVE_BALL_UPLEFT:
			//obstaculo canto

			if (ball->x - 1 <= 0 && ball->y - 1 <= 0)
				ball->trajectory = MOVE_BALL_DOWNRIGHT;
			// obstaculo superior
			else if (ball->y - 1 <= 0) {
				ball->x-=1;
				ball->y+=1;
				ball->trajectory = MOVE_BALL_DOWNLEFT;
			}
			// obstaculo esquerdo
			else if (ball->x - 1 <= 0) {
				ball->x+=1;
				ball->y-=1;
				ball->trajectory = MOVE_BALL_UPRIGHT;
			}
			else {
				for (int i = 0; i < NUMBER_TOTAL_BRIKS; i++)
				{
					// tijolo sup
					if (ball->x >= bricks[i].x && ball->x <= bricks[i].x + BRICK_WIDTH) {	 // esta enquadrada na coluna dos xx
						if (ball->y - 1 >= bricks[i].y && ball->y - 1 <= bricks[i].y + BRICK_HEIGHT) { // dentro da espessura, do tijolo superior
							bricks[i].health--;
							ball->x--;
							ball->y++;
							freeway = FALSE;
							break;
						}
					}
					// tijolo esq
					else if (ball->y >= bricks[i].y && ball->y - 1 <= bricks[i].y + BRICK_HEIGHT) { // enquadramento dos yy
						if (ball->x-1 <= bricks[i].x + BRICK_WIDTH) {		// dentro da lateral xx ?
							bricks[i].health--;
							ball->x++;
							ball->y--;
							freeway = FALSE;
							break;
						}
					}
				}
			}
			// livre
			if (freeway) {
				ball->x += 1;
				ball->y -= 1;
			}

			//tornar a variavel freeWay virgem do caso dos tijolos
			freeway = TRUE;
			break;


		default:
			break;
		}

		if(nPipes > 0)
			SendBroadcastPipe(balls);
		if(memPlayers > 0)
			SendBroadcastPipe(balls);
	}
	return 0;
}

BOOL AddBall()
{
	if (nBalls < 5)
	{
		nBalls++;
		hMovBola = CreateHandleArray(hMovBola, &nBalls);
		ballThreadId = ballIdArray(ballThreadId, &nBalls);
		balls = CreateBallArray(balls, &nBalls);
		hMovBola[nBalls - 1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BallMovement, &ballThreadId[nBalls - 1], 0, (LPDWORD)& ballThreadId[nBalls - 1]);

		if (hMovBola != NULL)
			_tprintf(TEXT("Lancei uma thread com id %d\n"), ballThreadId[nBalls - 1]);
		else
		{
			_tprintf(TEXT("Erro ao criar Thread BallMovement"));
			return EXIT_FAILURE;
		}
	}
	else
	{
		_tprintf(TEXT("Máximo de bolas atingido!\n"));
		return 0;
	}

	return 1;
}

BRICK * CreateBricks(BRICK *varBricks)
{
	srand(time(NULL));
	int randNum;
	varBricks = (BRICK *)malloc(sizeof(BRICK) * NUMBER_TOTAL_BRIKS);
	if (varBricks == NULL)
		return varBricks;

	for (int i = 0, line=0, col = 0; i < NUMBER_TOTAL_BRIKS; i++)
	{
		// set da coordenada xy.
		if (col++ < NUMBER_BRIKS_COL) {
			varBricks[i].x = X_STARPOINT_BRICKS + col * BRICK_WIDTH;
			varBricks[i].y = Y_STARPOINT_BRICKS + line * BRICK_HEIGHT;
		}
		else {
			col = 0;
			line++;
			varBricks[i].x = X_STARPOINT_BRICKS + col * BRICK_WIDTH;
			varBricks[i].y = Y_STARPOINT_BRICKS + line * BRICK_HEIGHT;
		}

		randNum = rand() % 100;
		if (randNum > 40) {
			varBricks[i].health = 1;
			varBricks[i].type = STD_BRICK;
		}
		else if (randNum > 30) {
			varBricks[i].type = STD_BRICK;
			varBricks[i].health = 2;
		}
		else if (randNum > 20) {
			varBricks[i].type = STD_BRICK;
			varBricks[i].health = 3;
		}
		else if (randNum > 10) {
			varBricks[i].type = STD_BRICK;
			varBricks[i].health = 4;
		}
		else {
			varBricks[i].health = 1;
			randNum = rand() % 4;
			if (randNum == 1)
				varBricks[i].type = BRICK_SPEEDUP;
			else if (randNum == 2)
				varBricks[i].type = BRICK_SLOWDOWN;
			else if (randNum == 3)
				varBricks[i].type = BRICK_EXTRALIFE;
			else
				varBricks[i].type = BRICK_TRIPLE;
		}
	}
	return varBricks;
}

BOOL RemoveBall()
{
	if (nBalls > 1)
	{
		nBalls--;
		hMovBola = CreateHandleArray(hMovBola, &nBalls);
		ballThreadId = ballIdArray(ballThreadId, &nBalls);
		balls = CreateBallArray(balls, &nBalls);
	}
	else
	{
		_tprintf(TEXT("Sem bolas para eleminar!\n"));
		return 0;
	}
	return 1;
}

//TODO: Top 10

PLAYERS SaveTopTen(PLAYERS* pAction)
{
	int mod = 0;
	int iSize;
	int iResult, iNpreenchidos = 10;
	int* newValues = (int*)malloc(sizeof(int) * 10);
	for (int i = 0; i < 10; i++)
		newValues[i] = values[i];

	HKEY hkChave;
	TCHAR tp[10][MAXT] = { TEXT("T1"), TEXT("T2"), TEXT("T3"), TEXT("T4"), TEXT("T5"), TEXT("T6"), TEXT("T7"), TEXT("T8"), TEXT("T9"), TEXT("T10") };
	TCHAR** newNome = (TCHAR * *)malloc(sizeof(TCHAR*) * 11);
	for (int i = 0; i < 11; i++)
		newNome[i] = (TCHAR*)malloc(sizeof(TCHAR) * MAXT);

	if (newNome == NULL)
	{
		_tprintf(TEXT("ERRO a alocar memória!\n"));
		return *pAction;
	}

	for (int i = 0; i < 10; i++)
	{
		_tcscpy_s(newNome[i], sizeof(newNome[i]) + 10, nome[i]);
		_tprintf(TEXT("USER: %s\n"), newNome[i]);
	}

	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Arkanoid"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkChave, (LPDWORD)& iResult) == ERROR_SUCCESS)
	{
		if (iResult == REG_CREATED_NEW_KEY)
		{
			_tprintf(TEXT("Chave criada com sucesso!"));
			//!Cria a nova chave do registro com os dados especificados
			for (int i = 0; i < iNpreenchidos; i++)
			{
				RegSetValueEx(hkChave, tp[i], 0, REG_SZ, (LPBYTE)newNome[i], (MAXT * sizeof(TCHAR)));
				RegSetValueEx(hkChave, newNome[i], 0, REG_BINARY, (LPBYTE)& newValues[i], sizeof(int));
			}

			for (int i = 0; i < iNpreenchidos; i++)
			{
				iSize = MAXT * sizeof(TCHAR);
				RegQueryValueEx(hkChave, tp[i], NULL, NULL, (LPBYTE)& pAction->top.names[i], (LPDWORD)& iSize);
				iSize = sizeof(int);
				RegQueryValueEx(hkChave, newNome[i], NULL, NULL, (LPBYTE)& pAction->top.points[i], (LPDWORD)& iSize);
			}
		}
		else if (iResult == REG_OPENED_EXISTING_KEY)
		{
			/*for (int i = 0; i < iNpreenchidos; i++)
			{
				if (pAction->score > newValues[i]) {
					for (int j = i, k = i + 1; j < iNpreenchidos, k < 11; j++, k++)
					{
						//_tprintf(TEXT("Valor: %d"), j);
						if (j < iNpreenchidos - 1) {
							_tcscpy_s(newNome[k], _countof(newNome[k]) + 10, newNome[j + 1]);
							newValues[k] = newValues[j + 1];
							//_tprintf(__T("Top %d -> Autor: %s Pontuação: %d\n"), j, newNome[j], newValues[j]);
						}
					}
					_tcscpy_s(newNome[i], _countof(newNome[i]) + 10, pAction->username);
					newValues[i] = pAction->score;
					mod = 1;
					break;
				}
				else {
					continue;
				}
			}

			if (mod == 1) {
				for (int i = 0; i < iNpreenchidos; i++)
				{
					RegSetValueEx(hkChave, tp[i], 0, REG_SZ, (LPBYTE) newNome[i], (MAXT * sizeof(TCHAR)));
					RegSetValueEx(hkChave, newNome[i], 0, REG_BINARY, (LPBYTE) &newValues[i], sizeof(int));
				}
				mod = 0;
			}*/

			for (int i = 0; i < iNpreenchidos; i++)
			{
				iSize = MAXT * sizeof(TCHAR);
				RegQueryValueEx(hkChave, tp[i], NULL, NULL, (LPBYTE)& pAction->top.names[i], (LPDWORD)& iSize);
				iSize = sizeof(int);
				RegQueryValueEx(hkChave, newNome[i], NULL, NULL, (LPBYTE)& pAction->top.points[i], (LPDWORD)& iSize);
			}
		}
		RegCloseKey(hkChave);
	}

	/*for (int i = 0; i < 10; i++) {
		_tcscpy_s(nome[i], _countof(nome[i]) + 10, newNome[i]);
		values[i] = newValues[i];
	}*/

	//free(newNome);
	//free(newValues);

	return *pAction;
}

HANDLE* CreateHandleArray(HANDLE* handle, int* tam)
{
	//HANDLE *aux;

	handle = (HANDLE*)realloc(handle, sizeof(HANDLE) * (*tam));

	if (handle == NULL)
	{
		_tprintf(TEXT("Memory allocation error! CODE: %d\n"), GetLastError());
		exit(GetLastError());
	}

	//handle = aux;
	//free(aux);
	return handle;
}

int* ballIdArray(int* threadId, int* tam)
{
	//int* aux;

	threadId = (int*)realloc(threadId, sizeof(int) * (*tam));

	if (threadId == NULL)
	{
		_tprintf(TEXT("Memory allocation error! CODE: %d\n"), GetLastError());
		exit(GetLastError());
	}

	//threadId = aux;
	//free(aux);
	return threadId;
}

BALL* CreateBallArray(BALL* ball, int* tam)
{
	//BALL* aux;

	ball = (BALL*)realloc(ball, sizeof(BALL) * (*tam));

	if (ball == NULL)
	{
		_tprintf(TEXT("Memory allocation error! CODE: %d\n"), GetLastError());
		exit(GetLastError());
	}

	//ball = aux;
	//free(aux);
	return ball;
}

//PIPE Related functions

DWORD WINAPI PipeRoutine(LPVOID param)
{
	OVERLAPPED stOverlapped;
	HANDLE pipeConections[MAX_PLAYERS];
	HANDLE clientConectionThreads[MAX_PLAYERS];
	DWORD clientConectionThreadID[MAX_PLAYERS];
	HANDLE tmpNamedPipeHandle;

	for (HANDLE conections : pipeConections)
		conections = INVALID_HANDLE_VALUE;
	
	for (HANDLE conectionThreads : clientConectionThreads)
		conectionThreads = INVALID_HANDLE_VALUE;


	hEventOver = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hEventOver == NULL) {
		_tprintf(TEXT("(CreateEvent) failed! GLE: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	else 
		stOverlapped.hEvent = hEventOver;
	
	for (DWORD index = 0; index < MAX_PLAYERS; index++) {
		if (LIVE == FALSE)
			break;

		tmpNamedPipeHandle = CreateNamedPipe(PIPE_SERVER_NAME,
											 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
											 PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 
											 MAX_PLAYERS, 
											 sizeof(PLAYERS), 
											 sizeof(PLAYERS),
											 1000, 
											 &sa);
		
		if (tmpNamedPipeHandle == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe) GLE: %d"), GetLastError());
			break;
		}

		ConnectNamedPipe(tmpNamedPipeHandle, &stOverlapped);
		WaitForSingleObject(hEventOver, INFINITE);
		nPipes++;

		if (LIVE == FALSE)
			break;

		clientConectionThreads[index] = CreateThread(NULL, 
										0, 
										(LPTHREAD_START_ROUTINE) ServerInputPipes, 
										(LPVOID)tmpNamedPipeHandle, 
										0, 
										&clientConectionThreadID[index]);

		pipeConections[index] = tmpNamedPipeHandle;
		
	}

	WaitForMultipleObjects(MAX_PLAYERS, clientConectionThreads, TRUE, INFINITE);

	for (HANDLE connections : clientConectionThreads)
		if (connections != INVALID_HANDLE_VALUE) {
			DisconnectNamedPipe(connections);
			CloseHandle(connections);
		}

	return 0;
}

PLAYERS ReceiveRequestFromPipeConnection(HANDLE pipe) {
	PLAYERS request;
	DWORD dwBytesRead;
	BOOL readSuccess = FALSE;

	_tprintf(TEXT("Waiting for connection or requests\n"));
	request.code = -1;

	readSuccess = ReadFile(pipe, &request, sizeof(PLAYERS), &dwBytesRead, NULL);

	if (readSuccess == FALSE) {
		_tprintf(TEXT("(ReadFile) Error while reading pipe! GLE: %d\n"), GetLastError());
		return request;
	}

	_tprintf(TEXT("Client or request request received from %s %d!\n"), request.username, request.id);

	return request;
}

BOOL SendAnswerToClientPipe(PLAYERS *pAction, HANDLE pipe) {
	DWORD dwBytesWritten;
	BOOL writeSuccess = FALSE;

	writeSuccess = WriteFile(pipe, pAction, sizeof(PLAYERS), &dwBytesWritten, NULL);

	if (writeSuccess == FALSE) {
		_tprintf(TEXT("(WriteFile) Error while writting to pipe! GLE: %d\n"), GetLastError());
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI PipeBroadcast(LPVOID param) {

	HANDLE tmpNamedPipeHandle = INVALID_HANDLE_VALUE;
	//SECURITY_ATTRIBUTES secAtt = *static_cast<SECURITY_ATTRIBUTES*>(param);

	for (HANDLE broad : hBroadcast)
		broad = INVALID_HANDLE_VALUE;

	hEventOverBroad = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (hEventOverBroad == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("(CreateEvent) failed! GLE: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	else
		stOverlappedBroad.hEvent = hEventOver;


	for (DWORD i = 0; i < MAX_PLAYERS; i++)
	{
		if (LIVE == FALSE)
			break;

		tmpNamedPipeHandle = CreateNamedPipe(PIPE_SERVER_NAME_DATA,
			PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			MAX_PLAYERS,
			sizeof(GAMEDATAPIPE),
			sizeof(GAMEDATAPIPE),
			1000,
			&sa);

		if (tmpNamedPipeHandle == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe) GLE: %d"), GetLastError());
			return EXIT_FAILURE;
		}

		ConnectNamedPipe(tmpNamedPipeHandle, &stOverlappedBroad);
		WaitForSingleObject(hEventOver, INFINITE);

		hBroadcast[i] = tmpNamedPipeHandle;
	
	}
	return EXIT_SUCCESS;
}

BOOL SendBroadcastPipe(BALL *balls) {
	DWORD dwBytesWritten;
	BOOL writeSuccess = FALSE;
	GAMEDATAPIPE data;

	data.nBalls = nBalls;
	data.nBricks = NUMBER_TOTAL_BRIKS;
	data.nBars = nBars;
	for (int i = 0; i < data.nBalls; i++)
		data.ball[i] = balls[i];
	for (int i = 0; i < data.nBricks; i++)
		data.bricks[i] = bricks[i];
	for (int i = 0; i < data.nBars; i++)
		data.bars[i] = bars[i];
	
	for (int j = 0; j < nPipes; j++) {

		writeSuccess = WriteFile(hBroadcast[j], &data, sizeof(GAMEDATAPIPE), &dwBytesWritten, NULL);

		if (!writeSuccess && GetLastError() == ERROR_NO_DATA) {
			CloseHandle(hBroadcast[j]);
			hBroadcast[j] = INVALID_HANDLE_VALUE;
			nPipes--;
		}

	}

	SetEvent(hEventOverBroad);

	return FALSE;
}

// Buffer clean up routine
void Cleanup(PSID pEveryoneSID, PSID pAdminSID, PACL pACL, PSECURITY_DESCRIPTOR pSD)
{
	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);
}

void Seguranca(SECURITY_ATTRIBUTES* sa)
{
	PSECURITY_DESCRIPTOR pSD;
	PACL pAcl;
	EXPLICIT_ACCESS ea;
	PSID pEveryoneSID = NULL, pAdminSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	TCHAR str[256];

	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL) {
		_tprintf(TEXT("Erro LocalAlloc!!!"));
		return;
	}
	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		_tprintf(TEXT("Erro IniSec!!!"));
		return;
	}

	// Create a well-known SID for the Everyone group.
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
	{
		_stprintf_s(str, 256, TEXT("AllocateAndInitializeSid() error %u\n"), GetLastError());
		_tprintf(str);
		Cleanup(pEveryoneSID, pAdminSID, NULL, pSD);
	}
	else
		_tprintf(TEXT("AllocateAndInitializeSid() for the Everyone group is OK\n"));

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

	ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	if (SetEntriesInAcl(1, &ea, NULL, &pAcl) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro SetAcl!!!"));
		return;
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pAcl, FALSE)) {
		_tprintf(TEXT("Erro IniSec!!!"));
		return;
	}
	
	sa->nLength = sizeof(*sa);
	sa->lpSecurityDescriptor = pSD;
	sa->bInheritHandle = TRUE;
}

void ReadGameSettings(wchar_t* fileName) {
	FILE* set;

	_tprintf(TEXT("Nome: %s\n"), fileName);

	errno_t err = _wfopen_s(&set, fileName, TEXT("w+"));
	if(err) {
		_tprintf(TEXT("[ERRO] Erro ao abrir ficheiro de configuração! GLE:%d\n"), GetLastError());
		return;
	}
	else {
		fwprintf_s(set, TEXT("%d %d"), 50, 3);
		// Set pointer to beginning of file:
		fseek(set, 0L, SEEK_SET);
		//Lê dados do ficheiro de configuração
		fwscanf_s(set, TEXT("%d"), &initAccel);
		fwscanf_s(set, TEXT("%d"), &initLife);
	}
	fclose(set);
}

BAR* CreatePlayerBar(BAR *bar, int *id) {
	bar = (BAR*)realloc(bar, sizeof(BAR) * nBars);

	if (bar == NULL)
		return bar;
	
	bar[nBars - 1].idPlayer = *id;
	bar[nBars - 1].size = BAR_SIZE;
	bar[nBars - 1].x = 500;
	bar[nBars - 1].y = 100;

	return bar;
}
