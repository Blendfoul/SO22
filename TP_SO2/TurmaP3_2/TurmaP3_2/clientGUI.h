#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <tchar.h>
#include <fcntl.h>
#include "../../MappedDLL/header.h"

//Global vars
HANDLE hConsole, hBallControl;
PLAYERS aux;
GAMEDATA game;
GAMEDATAPIPE gameP;

LRESULT CALLBACK CallTop10(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EventLogin(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CallUserStats(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Ball(LPVOID param);