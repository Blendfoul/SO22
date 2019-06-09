// MappedDLL.cpp : Defines the exported functions for the DLL application.
//
#include <windows.h>
#include "header.h"

SHAREDMEM player;
SHAREDMEM* pShared;
GAMEDATA* pSharedGame;

TCHAR NameCanWrite[] = TEXT("Semaphore_1");
TCHAR NameCanRead[] = TEXT("Semaphore_2");

//SHAREDMEM
HANDLE hCanWrite, hCanRead, hCanWriteBroad, hCanReadBroad;
HANDLE hMem, hMemGame;
HANDLE hFile, hFileGame;
HANDLE mutex_1, mutex_2;

//PIPES
HANDLE hPipe = NULL;
HANDLE hThread;
OVERLAPPED overlapped_structure;
HANDLE hBroad;

SECURITY_ATTRIBUTES sa;

//PIPE_NAME
wchar_t PIPE_SERVER_NAME[100];
wchar_t PIPE_SERVER_NAME_DATA[100];

BOOL InitPipes(TCHAR* ipAdress);
BOOL InitSharedMem();
void Seguranca(SECURITY_ATTRIBUTES* sa);
void Cleanup(PSID pEveryoneSID, PSID pAdminSID, PACL pACL, PSECURITY_DESCRIPTOR pSD);

BOOL TesteDLL(PLAYERS* client)
{
	printf("TesteServer");
	//printf("\nIN: %d\n", client->id);
	return true;
}

GAMEDATA RecieveBroadcast(GAMEDATA* pGame)
{

	WaitForSingleObject(hCanReadBroad, INFINITE);
	WaitForSingleObject(mutex_2, INFINITE);

	pGame = pSharedGame;

	ReleaseMutex(mutex_2);
	ReleaseSemaphore(hCanWriteBroad, 1, NULL);

	return *pGame;
}

GAMEDATAPIPE RecieveBroadcastPipe(GAMEDATAPIPE* pGame, TCHAR* ipAdress, int command) {

	DWORD dwBytesRead;
	BOOL readSuccess = FALSE;

	readSuccess = ReadFile(hBroad, pGame, sizeof(GAMEDATAPIPE), &dwBytesRead, NULL);

	if (!readSuccess || !dwBytesRead) {
		_tprintf(TEXT("[LEITOR] %d %d... (ReadFile)\n"), readSuccess, dwBytesRead);
		return *pGame;
	}
	else if (command == 9)
		DisconnectNamedPipe(hBroad);

	return *pGame;
}

PLAYERS RecieveMessage(PLAYERS* client)
{
	if (pShared != NULL)
	{
		WaitForSingleObject(hCanRead, INFINITE);
		WaitForSingleObject(mutex_1, INFINITE);

		player = *pShared;

		if (player.out == 10)
			player.out = 0;

		client = &player.players[player.out];

		if (player.players[player.out].code != SERVERCLOSE)
			(player.out)++;

		ReleaseMutex(mutex_1);
		ReleaseSemaphore(hCanWrite, 1, NULL);
	}
	return *client;
}

PLAYERS RecieveMessage(PLAYERS* client, TCHAR* ipAdress)
{
	_tprintf(TEXT("ID: %d\n"), client->id);

	DWORD dwBytesRead;
	BOOL readSuccess = FALSE;

	readSuccess = ReadFile(hPipe, client, sizeof(PLAYERS), &dwBytesRead, NULL);

	if (readSuccess == FALSE) {
		_tprintf(TEXT("(ReadFile) Error while reading pipe! GLE: %d\n"), GetLastError());
		return *client;
	}
	else if (client->code == 9)
		DisconnectNamedPipe(hPipe);

	return *client;
}

BOOL SendMessages(PLAYERS* client)
{
	_tprintf(TEXT("ID: %d IN: %d\n"), client->id, player.in);
	WaitForSingleObject(hCanWrite, INFINITE);
	WaitForSingleObject(mutex_1, INFINITE);

	player.players[player.in] = *client;
	CopyMemory(pShared, &player, sizeof(SHAREDMEM));
	if (player.in == 10)
		player.in = 0;
	else
		(player.in)++;

	ReleaseMutex(mutex_1);
	ReleaseSemaphore(hCanRead, 1, NULL);

	return true;
}

BOOL SendMessages(PLAYERS* client, TCHAR* ipAdress){
	DWORD dwBytesWritten;
	BOOL writeSuccess = FALSE;

	writeSuccess = WriteFile(hPipe, client, sizeof(PLAYERS), &dwBytesWritten, NULL);

	if (writeSuccess == FALSE) {
		_tprintf(TEXT("(WriteFile) Error while writting to pipe! GLE: %d\n"), GetLastError());
		return TRUE;
	}

	return FALSE;
}

BOOL Login(PLAYERS* client)
{

	InitSharedMem();
	SendMessages(client);

	return true;
}

BOOL Login(PLAYERS* client, TCHAR* ipAdress)
{

	_tprintf(TEXT("IP Recebido: %s\n"), ipAdress);
	if (InitPipes(ipAdress) == FALSE)
		SendMessages(client, ipAdress);
	else {
		_tprintf(TEXT("Failed to connect!\n"));
		return TRUE;
	}

	return FALSE;
}

void CloseVars()
{
	UnmapViewOfFile(pSharedGame);
	UnmapViewOfFile(pShared);
	CloseHandle(hCanRead);
	CloseHandle(hCanWrite);
	CloseHandle(hMem);
	CloseHandle(hMemGame);
}

BOOL InitPipes(TCHAR* ipAdress)
{

	swprintf(PIPE_SERVER_NAME, 100, TEXT("\\\\%s\\pipe\\arkanoid"), (wchar_t*)ipAdress);
	_tprintf(TEXT("%s\n"), PIPE_SERVER_NAME);

	swprintf(PIPE_SERVER_NAME_DATA, 100, TEXT("\\\\%s\\pipe\\arkanoidDATA"), (wchar_t*)ipAdress);
	_tprintf(TEXT("%s\n"), PIPE_SERVER_NAME_DATA);

	Seguranca(&sa);

	hPipe = CreateFile(PIPE_SERVER_NAME, GENERIC_READ | GENERIC_WRITE, 0, &sa, OPEN_EXISTING, 0, NULL);
	
	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile) GLE: %d\n"), PIPE_SERVER_NAME, GetLastError());
		return TRUE;
	}

	_tprintf(TEXT("Connect Success CLIENT!\n"));
	
	hBroad = CreateFile(PIPE_SERVER_NAME_DATA, GENERIC_READ, 0, &sa, OPEN_EXISTING, 0, NULL);
	
	if (hBroad == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile) GLE: %d\n"), PIPE_SERVER_NAME_DATA, GetLastError());
		return TRUE;
	}

	_tprintf(TEXT("Connect Success DATA!\n"));
	return FALSE;
}

void DisconnectPipes()
{
	CloseHandle(hPipe);
}

BOOL InitSharedMem()
{
	mutex_1 = OpenMutex(MUTEX_ALL_ACCESS, 0, TEXT("Mutex_1"));
	if (mutex_1 == NULL)
	{
		_tprintf(TEXT("Servidor n�o est� ligado!\n"));
		//CloseHandle(mutex_1);
		exit(EXIT_FAILURE);
	}

	mutex_2 = OpenMutex(MUTEX_ALL_ACCESS, 0, TEXT("Mutex_2"));
	if (mutex_2 == NULL)
	{
		CloseHandle(mutex_1);
		//CloseHandle(mutex_2);
		return EXIT_FAILURE;
	}

	hCanRead = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, NameCanRead);
	hCanWrite = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, NameCanWrite);
	hCanWriteBroad = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, TEXT("Semaphore_3"));
	hCanReadBroad = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, TEXT("Semaphore_4"));

	if (hCanRead == NULL || hCanWrite == NULL || hCanReadBroad == NULL || hCanWriteBroad == NULL)
	{

		return EXIT_FAILURE;
	}

	hFile = CreateFile(TEXT("Shared"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL)
		return EXIT_FAILURE;

	hMem = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, sizeof(SHAREDMEM), TEXT("Shared_1"));
	if (hMem == NULL)
		return EXIT_FAILURE;

	hFileGame = CreateFile(TEXT("Shared2"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileGame == NULL)
		return EXIT_FAILURE;

	hMemGame = CreateFileMapping(hFileGame, NULL, PAGE_READWRITE, 0, sizeof(GAMEDATA), TEXT("Shared_2"));
	if (hMemGame == NULL)
		return EXIT_FAILURE;

	pShared = (SHAREDMEM*)MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHAREDMEM));
	if (pShared == NULL)
	{
		_tprintf_s(TEXT("Erro de cria��o da view of file %lu\n"), GetLastError());

		CloseHandle(hMem);
		return EXIT_FAILURE;
	}

	pSharedGame = (GAMEDATA*)MapViewOfFile(hMemGame, FILE_MAP_READ, 0, 0, sizeof(GAMEDATA));
	if (pSharedGame == NULL)
	{
		_tprintf_s(TEXT("Erro de cria��o da view of file %lu\n"), GetLastError());

		CloseHandle(hMem);
		CloseHandle(hMemGame);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
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