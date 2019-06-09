//Base.c
#include "resource.h"
#include "clientGUI.h"

int tipe = -1;
BOOL LIVE = FALSE;
BOOL LOCALON = FALSE;
/* ===================================================== */
/* Programa base (esqueleto) para aplicações Windows */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
// Composto por 2 funções:
// WinMain() = Ponto de entrada dos programas windows
// 1) Define, cria e mostra a janela
// 2) Loop de recepção de mensagens provenientes do Windows
// TrataEventos()= Processamentos da janela (pode ter outro nome)
// 1) É chamada pelo Windows (callback)
// 2) Executa código em função da mensagem recebida

// Nome da classe da janela (para programas de uma só janela, normalmente este nome é
// igual ao do próprio programa) "szprogName" é usado mais abaixo na definição das
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("Base");
// ============================================================================
// FUNÇÃO DE INÍCIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa começa sempre a sua execução na função WinMain()que desempenha // o papel da função main() do C em modo consola WINAPI indica o "tipo da função" (WINAPI // para todas as declaradas nos headers do Windows e CALLBACK para as funções de // processamento da janela)
// Parâmetros:
// hInst: Gerado pelo Windows, é o handle (número) da instância deste programa
// hPrevInst: Gerado pelo Windows, é sempre NULL para o NT (era usado no Windows 3.1)
// lpCmdLine: Gerado pelo Windows, é um ponteiro para uma string terminada por 0
// destinada a conter parâmetros para o programa
// nCmdShow: Parâmetro que especifica o modo de exibição da janela (usado em
// ShowWindow()

HWND hWnd;

int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
			  // hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		  // MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp; // WNDCLASSEX é uma estrutura cujos membros servem para
	DWORD threadID;
	aux.id = GetCurrentProcessId();
	aux.score = 0;
	// definir as características da classe da janela
	// ============================================================================
	// 1. Definição das características da janela "wcApp"
	// (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		   // Instância da janela actualmente exibida
	// ("hInst" é parâmetro de WinMain e vem
	// inicializada daí)
	wcApp.lpszClassName = szProgName;						   // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;						   // Endereço da função de processamento da janela // ("TrataEventos" foi declarada no início e // encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;					   // Estilo da janela: Fazer o redraw se for // modificada horizontal ou verticalmente
	wcApp.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)); // "hIcon" = handler do ícon normal
	//"NULL" = Icon definido no Windows
	// "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = NULL; // "hIconSm" = handler do ícon pequeno
	//"NULL" = Icon definido no Windows
	// "IDI_INF..." Ícon de informação
	wcApp.hCursor = NULL; // "hCursor" = handler do cursor (rato)
	// "NULL" = Forma definida no Windows
	// "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); // Classe do menu que a janela pode ter
	// (NULL = não tem menu)
	wcApp.cbClsExtra = 0; // Livre, para uso particular
	wcApp.cbWndExtra = 0; // Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por // "GetStockObject".Neste caso o fundo será branco
	// ============================================================================
	// 2. Registar a classe "wcApp" no Windows
	// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return (0);
	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	
	LIVE = TRUE;
	hBallControl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Ball, NULL, 0, &threadID);
	if (hBallControl == NULL)
	{
		_tprintf(TEXT("Error creating thread\n"));
		return EXIT_FAILURE;
	}

	DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	
	hWnd = CreateWindow(
		szProgName,							 // Nome da janela (programa) definido acima
		TEXT("Arkanoid"),					 // Texto que figura na barra do título
		dwStyle,							 // Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,						 // Posição x pixels (default=à direita da última)
		CW_USEDEFAULT,						 // Posição y pixels (default=abaixo da última)
		MAX_SCREEN_WIDTH,					 // Largura da janela (em pixels)
		MAX_SCREEN_HEIGHT,					 // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,					 // handle da janela pai (se se criar uma a partir de
											 // outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL,	  // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é
		// passado num dos parâmetros de WinMain()
		0);						// Não há parâmetros adicionais para a janela
								// ============================================================================
								// 4. Mostrar a janela
								// ============================================================================
	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
	// "CreateWindow"; "nCmdShow"= modo de exibição (p.e.
	// normal/modal); é passado como parâmetro de WinMain()
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma
	// mensagem para pintar, mostrar dados, (refrescar)…
	// ============================================================================
	// 5. Loop de Mensagens
	// ============================================================================
	// O Windows envia mensagens às janelas (programas). Estas mensagens ficam numa fila de
	// espera até que GetMessage(...) possa ler "a mensagem seguinte"
	// Parâmetros de "getMessage":
	// 1)"&lpMsg"=Endereço de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no
	// início de WinMain()):
	// HWND hwnd handler da janela a que se destina a mensagem
	// UINT message Identificador da mensagem
	// WPARAM wParam Parâmetro, p.e. código da tecla premida
	// LPARAM lParam Parâmetro, p.e. se ALT também estava premida
	// DWORD time Hora a que a mensagem foi enviada pelo Windows
	// POINT pt Localização do mouse (x, y)
	// 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem // receber as mensagens para todas as janelas pertencentes à thread actual)
	// 3)Código limite inferior das mensagens que se pretendem receber
	// 4)Código limite superior das mensagens que se pretendem receber
	// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
	// terminando então o loop de recepção de mensagens, e o programa
	

	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		TranslateMessage(&lpMsg); // Pré-processamento da mensagem (p.e. obter código
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg); // Enviar a mensagem traduzida de volta ao Windows, que
								 // aguarda até que a possa reenviar à função de
								 // tratamento da janela, CALLBACK TrataEventos (abaixo)
	}

	WaitForSingleObject(hBallControl, INFINITE);
	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	CloseVars();
	return ((int)lpMsg.wParam); // Retorna sempre o parâmetro wParam da estrutura lpMsg
}
// ============================================================================
// FUNÇÃO DE PROCESSAMENTO DA JANELA
// Esta função pode ter um nome qualquer: Apenas é necesário que na inicialização da // estrutura "wcApp", feita no início de WinMain(), se identifique essa função. Neste // caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pré-processadas // no loop "while" da função WinMain()
//
// Parâmetros:
// hWnd O handler da janela, obtido no CreateWindow()
// messg Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
// wParam O parâmetro wParam da estrutura messg (a mensagem)
// lParam O parâmetro lParam desta mesma estrutura
//
// NOTA:Estes parâmetros estão aqui acessíveis o que simplifica o acesso aos seus valores
//
// A função EndProc é sempre do tipo "switch..." com "cases" que descriminam a mensagem // recebida e a tratar. Estas mensagens são identificadas por constantes (p.e.
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h ============================================================================
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HDC memdc = NULL;
	static TCHAR c;
	static int x = 10, y = 10, xi = 0, yi = 0, xf = 0, yf = 0;
	PAINTSTRUCT ps;

	HBITMAP fundo;

	switch (messg)
	{
		// outra) ou HWND_DESKTOP se a janela for a primeira,
	case WM_DESTROY: // Destruir a janela e terminar o programa
		// "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		switch (tipe)
		{
		case 1:
			_tcscpy_s(aux.command, TEXT("logout"));
			SendMessages(&aux, aux.ipAdress);
			aux = RecieveMessage(&aux, aux.ipAdress);
			LIVE = FALSE;
			if (aux.code == LOGOUTSUCCESS)
				MessageBox(hWnd, TEXT("Logout com sucesso"), TEXT("Servidor"), MB_OK);
			PostQuitMessage(0);
			break;
		case 0:
			_tcscpy_s(aux.command, TEXT("logout"));
			SendMessages(&aux);
			aux = RecieveMessage(&aux);
			LIVE = FALSE;
			if (aux.code == LOGOUTSUCCESS)
				MessageBox(hWnd, TEXT("Logout com sucesso"), TEXT("Servidor"), MB_OK);
			PostQuitMessage(0);
			break;
		default:
			MessageBox(hWnd, TEXT("Sem utilizador logado"), TEXT("Servidor"), MB_OK);
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SOBRE:
			MessageBox(hWnd, TEXT("\tArkanoid\n\n\nDeveloped by:\nJoão Rodrigues 21240370\nMarco Ferreira 21260***"), TEXT("Sobre"), MB_OK);
			break;
		case ID_CLIENTE_REMOTO:
			switch (tipe)
			{
			case -1:
				tipe = 1;
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)EventLogin);
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG4), NULL, (DLGPROC)CallUserStats);			
				break;
			default:
				MessageBox(hWnd, TEXT("Cliente já logado!"), TEXT("Servidor"), MB_OK);
				break;
			}
			break;
		case ID_CLIENTE_LOCAL:
			switch (tipe)
			{
			case -1:
				tipe = 0;
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), hWnd, (DLGPROC)EventLogin);
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG4), NULL, (DLGPROC)CallUserStats);
				break;
			default:
				MessageBox(hWnd, TEXT("Cliente já logado!"), TEXT("Servidor"), MB_OK);
				break;
			}
			break;
		case ID_JOGO_TOP10:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, (DLGPROC)CallTop10);
			break;
		case ID_JOGO_SAIR:
			switch (tipe)
			{
			case 1:
				_tcscpy_s(aux.command, TEXT("logout"));
				SendMessages(&aux, aux.ipAdress);
				aux = RecieveMessage(&aux, aux.ipAdress);
				if (aux.code == LOGOUTSUCCESS)
					MessageBox(hWnd, TEXT("Logout com sucesso"), TEXT("Servidor"), MB_OK);
				PostQuitMessage(0);
				break;
			case 0:
				_tcscpy_s(aux.command, TEXT("logout"));
				SendMessages(&aux);
				aux = RecieveMessage(&aux);
				if (aux.code == LOGOUTSUCCESS)
					MessageBox(hWnd, TEXT("Logout com sucesso"), TEXT("Servidor"), MB_OK);
				PostQuitMessage(0);
				break;
			default:
				MessageBox(hWnd, TEXT("Sem utilizador logado"), TEXT("Servidor"), MB_OK);
				PostQuitMessage(0);
				break;
			}
			break;
		}
		break;
	case WM_CREATE:
		hdc = GetDC(hWnd);
		memdc = CreateCompatibleDC(hdc);

		fundo = CreateCompatibleBitmap(hdc, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT);
		SelectObject(memdc, GetStockObject(DC_BRUSH));
		SetDCBrushColor(memdc, RGB(62, 178, 72));//verde escuro o fundo
		PatBlt(memdc, 0, 0, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT, PATCOPY);
		
		ReleaseDC(hWnd, hdc);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT, memdc, 0, 0, SRCCOPY);
		int i;
		hdc = GetDC(hWnd);

		switch (tipe) {
		case 0:
			for (i = 0; i < game.nBalls; i++) {
				Ellipse(hdc, game.ball[game.out][i].x - 4, game.ball[game.out][i].y - 4, game.ball[game.out][i].x + 4, game.ball[game.out][i].y + 4);
				Ellipse(memdc, game.ball[game.out][i].x - 4, game.ball[game.out][i].y - 4, game.ball[game.out][i].x + 4, game.ball[game.out][i].y + 4);
			}
			break;
		case 1:
			for (i = 0; i < gameP.nBalls; i++) {
				Ellipse(hdc, gameP.ball->x - 4, gameP.ball->y - 4, gameP.ball->x + 4, gameP.ball->y + 4);
				Ellipse(memdc, gameP.ball->x - 4, gameP.ball->y - 4, gameP.ball->x + 4, gameP.ball->y + 4);
			}
			break;
		}


		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_RIGHT)
		{
			x += 10;
			/*hdc = GetDC(hWnd);
			TextOut(hdc, x, y, &c, 1);
			ReleaseDC(hWnd, hdc);*/
			InvalidateRect(hWnd, NULL, 1);
		}
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar") // não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return DefWindowProc(hWnd, messg, wParam, lParam);
		break;
	}
	return (0);
}

LRESULT CALLBACK EventLogin(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	switch (messg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			//buscar texto das caixas de texto)

			GetDlgItemText(hWnd, IDC_EDIT1, aux.username, MAX);
			GetDlgItemText(hWnd, IDC_IPADDRESS1, aux.ipAdress, 20);

			switch (tipe)
			{
			case 1:
				Login(&aux, aux.ipAdress);
				aux = RecieveMessage(&aux, aux.ipAdress);
				LIVE = TRUE;

				if (aux.code == 1)
				{
					MessageBox(hWnd, TEXT("[REMOTO]Login com sucesso!"), TEXT("Login Result"), MB_OK);
				}
				else
					MessageBox(hWnd, TEXT("[REMOTO]Login sem sucesso!"), TEXT("Login Result"), MB_OK);

				EndDialog(hWnd, 0);
				return TRUE;
			case 0:
				Login(&aux);
				aux = RecieveMessage(&aux);
				LIVE = TRUE;
				LOCALON = TRUE;
				if (aux.code == 1)
				{
					MessageBox(hWnd, TEXT("[LOCAL] Login com sucesso!"), TEXT("Login Result"), MB_OK);
				}
				else
					MessageBox(hWnd, TEXT("[REMOTO]Login sem sucesso!"), TEXT("Login Result"), MB_OK);

				EndDialog(hWnd, 0);
				return TRUE;
			}
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hWnd, 0);
			return FALSE;
		}
	}
	return FALSE;
}

LRESULT CALLBACK CallTop10(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR string[MAX];
	LRESULT dwPos;
	HWND hWndList;

	switch (messg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_INITDIALOG:
		if (tipe != -1)
		{
			_tcscpy_s(aux.command, TEXT("top10"));
			switch (tipe)
			{
			case 0:
				SendMessages(&aux);
				aux = RecieveMessage(&aux);
				break;
			case 1:
				SendMessages(&aux, aux.ipAdress);
				aux = RecieveMessage(&aux, aux.ipAdress);
				break;
			}
			hWndList = GetDlgItem(hWnd, IDC_LIST1);
			for (int i = 0; i < 10; i++)
			{
				_stprintf_s(string, MAX, __T("%s    %d"), aux.top.names[i], aux.top.points[i]);
				dwPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
				SendMessage(hWndList, LB_SETITEMDATA, dwPos, (LPARAM)i + 1);
			}
		}
		else
		{
			MessageBox(hWnd, TEXT("Login please!"), TEXT("TOP 10"), MB_OK);
			EndDialog(hWnd, 0);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hWnd, 0);
			return TRUE;
		}
	}

	return FALSE;
}

LRESULT CALLBACK CallUserStats(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndList;
	LRESULT lrPos;
	TCHAR string[MAX];
	PLAYERS view = aux;

	switch (messg)
	{
	case WM_INITDIALOG:
		hWndList = GetDlgItem(hWnd, IDC_LIST1);
		_stprintf_s(string, MAX, __T("Username: %s"), aux.username);
		lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
		_stprintf_s(string, MAX, __T("Vida: %d"), aux.life);
		lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
		_stprintf_s(string, MAX, __T("Pontuação: %d"), aux.score);
		lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
		_stprintf_s(string, MAX, __T("Ultimo comando: %s"), aux.command);
		lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
		break;

	default:
		if (view.score != aux.score || view.life != aux.life || _tcscmp(view.command, aux.command) != 0)
		{
			view = aux;
			hWndList = GetDlgItem(hWnd, IDC_LIST1);
			SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
			_stprintf_s(string, MAX, __T("Username: %s"), view.username);
			lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
			_stprintf_s(string, MAX, __T("Vida: %d"), view.life);
			lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
			_stprintf_s(string, MAX, __T("Pontuação: %d"), view.score);
			lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
			_stprintf_s(string, MAX, __T("Ultimo comando: %s"), view.command);
			lrPos = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)string);
		}
		break;
	}

	return FALSE;
}

DWORD WINAPI Ball(LPVOID param)
{
	RECT info;
	info.top = info.left = 0;
	info.right = MAX_SCREEN_WIDTH;
	info.bottom = MAX_SCREEN_HEIGHT;

	while (LIVE == TRUE)
	{
		switch (tipe)
		{
		case 0:
			if(LOCALON == TRUE)
			game = RecieveBroadcast(&game);
			break;
		case 1:
			gameP = RecieveBroadcastPipe(&gameP, aux.ipAdress, aux.code);
			break;
		default:
			break;
		}
		InvalidateRect(hWnd, &info, TRUE);
		UpdateWindow(hWnd);
	};

	return EXIT_FAILURE;
}
