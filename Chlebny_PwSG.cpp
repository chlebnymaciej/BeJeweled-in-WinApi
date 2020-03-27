// Chlebny_PwSG.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Chlebny_PwSG.h"

#include <list>
#include <iterator>
#include <string>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <wingdi.h>
#include <math.h>

#define MAX_LOADSTRING 100

/////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////

#define SMALL_GAME 720
#define MEDIUM_GAME 800
#define BIG_GAME 840

#define SMALL_G 80
#define MEDIUM_G 70
#define BIG_G 60
#define P_SIZE 8
#define MARGIN 5
#define MAX 144
#define TIMER_UPDATE 1111


/////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////
typedef struct Kwadrat {
    HWND hWnd;
    int x;
    int y;
    int color;
	int size;
	char kill_or_survive;
	// kill_on_survive 
	// 0 - survive gray
	// 1 - survive color
	// 2 - kill
	// 3 - choosen
	// 4 - gray choosen
} Kwadrat_t;

typedef struct Mouse {
	Kwadrat_t first, second;
	bool firstTrue;
}Mouse_t;

typedef struct Series {
	int i, j; // poziome
}Series_t;

typedef struct atom
{
	int x, y, vx, vy, s, c;
}atom_t;

/////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////
HINSTANCE hInst;                               
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];
HWND hMain, hTrans;

std::vector<POINT> grid_points;
std::vector<Kwadrat_t> gems;
std::list<atom_t> particles;
HDC buffor;
HBITMAP buff;
Mouse_t mouse;
Kwadrat_t** pgems = nullptr;
char hasGameStarted = 0;
int size_g = 8;
int screenX = GetSystemMetrics(SM_CXFULLSCREEN);
int screenY = GetSystemMetrics(SM_CYFULLSCREEN);
char debug = 0;
char moveOrNot = 0;
int row = -1;

HBRUSH color[7] = {
					CreateSolidBrush(RGB(255,0,0)),
					CreateSolidBrush(RGB(255,255,0)),
					CreateSolidBrush(RGB(255,0,255)),
					CreateSolidBrush(RGB(0,255,0)),
					CreateSolidBrush(RGB(0,255,255)),
					CreateSolidBrush(RGB(0,0,255)),
					CreateSolidBrush(RGB(120, 80, 40))
};
COLORREF colorr[6] = {
					RGB(255,0,0),
					RGB(255,255,0),
					RGB(255,0,255),
					RGB(0,255,0),
					RGB(0,255,255),
					RGB(0,0,255)
};
HBRUSH colorCross[6] = {
					CreateHatchBrush(HS_CROSS, 	RGB(255,0,0)),
					CreateHatchBrush(HS_CROSS, 	RGB(255,255,0)),
					CreateHatchBrush(HS_CROSS, 	RGB(255,0,255)),
					CreateHatchBrush(HS_CROSS, RGB(0,255,0)),
					CreateHatchBrush(HS_CROSS, RGB(0,255,255)),
					CreateHatchBrush(HS_CROSS, RGB(0,0,255))
};

HBRUSH gray = CreateSolidBrush(RGB(100, 100, 100));
HBRUSH frame = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
/////////////////////////////////////////////////////////////////////
// Main headers
/////////////////////////////////////////////////////////////////////
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcGem(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcTransparent(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
/////////////////////////////////////////////////////////////////////
// Headers
/////////////////////////////////////////////////////////////////////
ATOM MyRegisterGem(HINSTANCE hInstance);
ATOM MyRegisterTransparent(HINSTANCE hInstance);

POINT Center(HWND);
void SetClientSize(HWND hwnd, int clientWidth, int clientHeight); // https://cboard.cprogramming.com/windows-programming/80365-adjustwindowrect-setting-client-size.html
void Grid(int size);
void CreateBoard(HWND hWnd, HINSTANCE hInstance, int size);
void DestroyKids();
void ChangeCheckedItem(HMENU, UINT);
Kwadrat_t gemWindow(HWND hwnd);
POINT getCords(HWND hwnd);
void CreateParticle(HWND hWnd);
void newGame();
void MouseFunction(HWND hWnd);
void MouseFunctionBeforeGame(HWND);
void CorrectColor(HWND);
bool Logic();
bool LogicBoolOnly();
void newColors(int i, int j);
void background(HBRUSH color);
void clearOneByOne();
bool Update();

bool isMovePosible();
/////////////////////////////////////////////////////////////////////
// Main Functions
/////////////////////////////////////////////////////////////////////


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHLEBNYPWSG, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterGem(hInstance);
	MyRegisterTransparent(hInstance);
	mouse.firstTrue = false;
    
	// Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHLEBNYPWSG));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHLEBNYPWSG));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHLEBNYPWSG);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
ATOM MyRegisterGem(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcGem;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHLEBNYPWSG));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(100, 100, 100));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Gem";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
ATOM MyRegisterTransparent(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcTransparent;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHLEBNYPWSG));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TransparentWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   HWND hWnd = CreateWindowW(szWindowClass,L"BeWindowed 2020 a.k.a. Bejeweled in WinAPI", WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX,
      0, 0, SMALL_GAME, SMALL_GAME, nullptr, nullptr, hInstance, nullptr);
   
   hMain = hWnd;
   if (!hWnd)
   {
	   return FALSE;
   }
   SetClientSize(hWnd, SMALL_GAME, SMALL_GAME);
   POINT p = Center(hWnd);
   SetWindowPos(hWnd, HWND_TOP, p.x, p.y, 0, 0, SWP_NOSIZE);
   
   HWND hWndTransparent = CreateWindowExW(WS_EX_LAYERED,
	   L"TransparentWindow", NULL, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN),
	   GetSystemMetrics(SM_CYFULLSCREEN),hMain , NULL, hInstance, NULL);
   if (!hWndTransparent)
   {
	   return FALSE;
   }
   SetLayeredWindowAttributes(hWndTransparent, RGB(0, 0, 0), 0, LWA_COLORKEY);
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   ShowWindow(hWndTransparent, nCmdShow);

   Grid(8);
   CreateBoard(hWnd, hInstance, 8);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	HMENU menu = GetMenu(hWnd);
    
	switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break; 
			case ID_GAME_SMALLGAME:
				hasGameStarted = 1;
				newGame();
				break;
            case ID_BOARDSIZE_SMALL:
			{
				hasGameStarted = 0;
				ChangeCheckedItem(menu, ID_BOARDSIZE_SMALL);
				DestroyKids();
				SetClientSize(hWnd, SMALL_GAME, SMALL_GAME);
				Grid(8);
				CreateBoard(hWnd, hInst, 8);
			}
                break;
            case ID_BOARDSIZE_MEDIUM:
			{
				hasGameStarted = 0;
				ChangeCheckedItem(menu, ID_BOARDSIZE_MEDIUM);
				DestroyKids();
				SetClientSize(hWnd, MEDIUM_GAME, MEDIUM_GAME);
				Grid(10);
				CreateBoard(hWnd, hInst, 10);
			}
				break;
            case ID_BOARDSIZE_BIG:
			{
				hasGameStarted = 0;
				ChangeCheckedItem(menu, ID_BOARDSIZE_BIG);
				DestroyKids();
				SetClientSize(hWnd, BIG_GAME, BIG_GAME);
				Grid(12);
				CreateBoard(hWnd, hInst, 12);
			}
				break;
			case ID_HELP_DEBUG:
				if (debug == 0)
				{
					CheckMenuItem(menu, ID_HELP_DEBUG, MF_CHECKED);
					debug = 1;
				}
				else {
					CheckMenuItem(menu, ID_HELP_DEBUG, MF_UNCHECKED);
					debug = 0;
				}
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
		break;
	case WM_CREATE:
	{
		SetTimer(hWnd, TIMER_UPDATE, 250, NULL);
	} break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_TIMER:
		if (wParam == TIMER_UPDATE) {
			if (row < size_g && row>-1) 
			{
				clearOneByOne();
				row++;
			}
			else if (row == size_g) 
			{
				Update();
			}
			else
			{
				row = -1;
			}
		} break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
LRESULT CALLBACK WndProcTransparent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		break;
	case WM_CREATE:
		{
		SetTimer(hWnd, 0, 1000/60, NULL);
		HDC hdc = GetDC(hWnd);
		buffor = CreateCompatibleDC(hdc);
		buff = CreateCompatibleBitmap(hdc, screenX,
		screenY);
		SelectObject(buffor, buff);
		ReleaseDC(hWnd, hdc);
		}	break;
	case WM_TIMER: 
	{
		if (wParam == 0)
		{
			RECT rc;
			GetWindowRect(hWnd, &rc);
			FillRect(buffor, &rc, frame);
			if (debug == 1) 
			{
				TCHAR s[256];
				swprintf_s(s, 256, _T("Particles: %d"), (int)particles.size());
				
				HFONT font = CreateFont(
					-MulDiv(24, GetDeviceCaps(buffor, LOGPIXELSY), 72), // Height
					0, // Width
					0, // Escapement
					0, // Orientation
					FW_BOLD, // Weight
					false, // Italic
					FALSE, // Underline
					0, // StrikeOut
					EASTEUROPE_CHARSET, // CharSet
					OUT_DEFAULT_PRECIS, // OutPrecision
					CLIP_DEFAULT_PRECIS, // ClipPrecision
					DEFAULT_QUALITY, // Quality
					DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
					_T(" Verdana "));
				SetTextColor(buffor, RGB(255, 0, 0));
				SetBkMode(buffor, TRANSPARENT);

				SelectObject(buffor, font);
				DrawText(buffor, s, (int)_tcslen(s), &rc, DT_CENTER | DT_TOP | DT_SINGLELINE);
				DeleteObject(font);
			}
			auto it = particles.begin();
			for (; it != particles.end();) {				
				rc = { it->x,it->y,it->x + it->s,it->y + it->s };
				FillRect(buffor, &rc, color[it->c]);
				
				it->x += it->vx;
				it->y += it->vy;
				
				if (it->x + it->s <= 0 || it->x >= screenX || 
					it->y + it->s <= 0 || it->y >= screenY) it = particles.erase(it);
				else it++;
			}
			
			BitBlt(GetDC(hWnd), 0, 0, screenX,
				screenY, buffor, 0, 0, SRCCOPY);
		}
	}	break;
	case WM_ERASEBKGND:
		return 1;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK WndProcGem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
	case WM_ERASEBKGND:
		return 1;
    case WM_PAINT:
    {		
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
		POINT p = getCords(hWnd);
		int i = p.y, j = p.x;
		RECT rc = { 0,0,pgems[i][j].size + 4, pgems[i][j].size + 4 };
		switch (pgems[p.y][p.x].kill_or_survive)
			{
			case 0:
			{
				FillRect(hdc, &rc, gray);
			} break;
			case 1:
			{
				int i = p.y, j = p.x;
				RECT rc;
				rc.left = 0;
				rc.top = 0;
				rc.right = pgems[i][j].size + 4;
				rc.bottom = pgems[i][j].size + 4;

				FillRect(hdc, &rc, color[pgems[i][j].color]);
			} break;
			case 2:
			{
				RECT rc;
				GetWindowRect(hWnd, &rc);
				FillRect(hdc, &rc, colorCross[pgems[p.y][p.x].color]);
			} break;
			case 3:
			{

				Kwadrat_t k = gemWindow(hWnd);
				RECT rc;
				GetClientRect(hWnd, &rc);
				FillRect(hdc, &rc, frame);

				RECT rc2 = {4,4,rc.right-rc.left-4,rc.right - rc.left - 4 };
				FillRect(hdc, &rc2, color[k.color]);
			}
			break;

		} 
		
        EndPaint(hWnd, &ps);

    }
    break;
	
	// -------------------------------------------------------------------
	// https://github.com/marchmello/WINAPI-TUTORIAL/blob/master/Hover.cpp
	// -------------------------------------------------------------------
	case WM_MOUSEMOVE: {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hWnd;

		TrackMouseEvent(&tme);
		break;
	}

	case WM_MOUSEHOVER: 
	{
		if (moveOrNot == 0) {
			Kwadrat_t k = gemWindow(hWnd);
			MoveWindow(hWnd, k.x - 2, k.y - 2, k.size + 4, k.size + 4, TRUE);
		}
		break;
	}

	case WM_MOUSELEAVE: 
	{
		if (moveOrNot == 0) {
			SetTimer(hWnd, 1, 50, NULL);
		}
		break;
	}
	case WM_TIMER:
	{
		if (wParam == 2000) {
			POINT p = getCords(hWnd);
			pgems[p.y][p.x].color = rand() % 6;
			pgems[p.y][p.x].kill_or_survive = 1;
			
			InvalidateRect(hWnd, 0, 1);
			UpdateWindow(hWnd);
			KillTimer(hWnd, 2000);

		}
		if (wParam == 1) {
			Kwadrat_t k = gemWindow(hWnd);
			RECT rc;
			GetWindowRect(hWnd, &rc);
			int side = rc.bottom - rc.top;

			if (side > k.size) {
				MoveWindow(hWnd, k.x, k.y, side - 1, side - 1, TRUE);
			}
			else {
				KillTimer(hWnd, 1);
			}
		}
	} break;
	case WM_LBUTTONDOWN:
		if (moveOrNot == 0) {
			if (hasGameStarted == 0) {
				MouseFunctionBeforeGame(hWnd);
			}
			else {
				MouseFunction(hWnd);
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/////////////////////////////////////////////////////////////////////
// Implementations
/////////////////////////////////////////////////////////////////////

POINT Center(HWND hWnd) {
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    RECT rc;
    GetWindowRect(hWnd, &rc);

    POINT p;
    p.x = screenX / 2 - (rc.right - rc.left) / 2;
    p.y = screenY / 2 - (rc.bottom - rc.top) / 2;

    return p;
}

void SetClientSize(HWND hwnd, int clientWidth, int clientHeight)
{
    if (IsWindow(hwnd))
    {

        DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        HMENU menu = GetMenu(hwnd);

        RECT rc = { 0, 0, clientWidth, clientHeight };

        if (!AdjustWindowRectEx(&rc, dwStyle, menu ? TRUE : FALSE, dwExStyle))
            MessageBox(NULL, L"AdjustWindowRectEx Failed!", L"Error", MB_OK);


		MoveWindow(hwnd, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		
		POINT p = Center(hwnd);
		SetWindowPos(hwnd, HWND_TOP, p.x, p.y, 0, 0, SWP_NOSIZE);

		UpdateWindow(hwnd);
    }
}

void Grid(int size) 
{   
    int s = SMALL_G;
    
	switch (size) {
    case 8:
        s = SMALL_G;
        break;
    case 10:
        s = MEDIUM_G;
        break;
    case 12:
        s = BIG_G;
        break;
    }

    POINT p;
    int mar_big = 2 * MARGIN;
    int last_x = MARGIN;
    int last_y = MARGIN;

    for (int i = 0; i < size; i++)
    {
        last_x = MARGIN;

        for (int j = 0; j < size; j++) {
            p.x = last_x;
            p.y = last_y;;
            grid_points.push_back(p);
            last_x += mar_big + s;
        }
        last_y += mar_big + s;
    }
}

void CreateBoard(HWND hWnd,HINSTANCE hInstance,int size)
{

    int s = SMALL_G;
	int old_size = size_g;
	size_g = size;
	switch (size) {
    case 8:
        s = SMALL_G;
        break;
    case 10:
        s = MEDIUM_G;
        break;
    case 12:
        s = BIG_G;
        break;
    }

	// home part
	if(pgems!=nullptr)
	{	
		for (int i = 0; i < old_size; i++)
			delete[] pgems[i];
		delete[] pgems;
	}

	pgems = new Kwadrat_t * [size_g];
	for (int i = 0; i < size_g; i++)
		pgems[i] = new Kwadrat_t[size_g];
	
	int i = 0, j = 0;
    
	auto it = grid_points.begin();
    for (; it != grid_points.end(); it++)
    {
        HWND hGemTmp = CreateWindow(L"Gem", NULL, WS_CHILD,
            it->x, it->y, s, s, hWnd, nullptr, hInstance, nullptr);
		
        Kwadrat_t k;
        k.hWnd = hGemTmp;
        k.x = it->x;
        k.y = it->y;
		k.color = rand() % 6;
		k.size = s;
		k.kill_or_survive = 0;
        gems.push_back(k);
        ShowWindow(hGemTmp, SW_SHOW);
        UpdateWindow(hGemTmp);
		pgems[i++/size_g][j++%size_g] = k;
		CorrectColor(hGemTmp);
    }

}

void DestroyKids()
{
	auto it = gems.begin();
	for (; it != gems.end(); it++) {
		DestroyWindow(it->hWnd);
	}

	gems.clear();
	grid_points.clear();
}

void ChangeCheckedItem(HMENU menu, UINT itemToCheck) 
{

	CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_UNCHECKED);
	CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_UNCHECKED);
	CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_UNCHECKED);

	CheckMenuItem(menu, itemToCheck, MF_CHECKED);
}

Kwadrat_t gemWindow(HWND hwnd)
{

	for(int i = 0;i<size_g;i++)
		for (int j = 0; j < size_g; j++) {
			if (pgems[i][j].hWnd == hwnd)
				return pgems[i][j];
		}
	return Kwadrat_t();
}

POINT getCords(HWND hwnd) {
	for (int i = 0; i < size_g; i++)
		for (int j = 0; j < size_g; j++) {
			if (pgems[i][j].hWnd == hwnd)
			{
				POINT p;
				p.x = j;
				p.y = i;
				return p;
			}
		}
	return POINT();
}

void CreateParticle(HWND hWnd) {
	Kwadrat_t k = gemWindow(hWnd);
	POINT p = { k.x,k.y };
	ClientToScreen(hMain, &p);
	for (int i = 0; i < 100; i++) {
		int vx = rand() % 15 + 1;
		int vy = rand() % 15 + 1;

		if (rand() % 2 == 0) vx *= -1;
		if (rand() % 2 == 0) vy *= -1;

		// normalizacja wektora predkosci
		double len = sqrt(vx * vx + vy * vy);
		vx = (int)(vx * 10 / len);
		vy = (int)(vy * 10 / len);

		atom_t a = { p.x + k.size / 2,p.y + k.size / 2, vx,vy, P_SIZE, k.color };
		particles.push_back(a);
	}
}

void newGame() 
{
	hasGameStarted = 0;
	for(int i=0;i<size_g;i++)
	{
		for (int j = 0; j < size_g; j++)
		{
			pgems[i][j].kill_or_survive = 1;
			
			UpdateWindow(pgems[i][j].hWnd);
			HDC hdc = GetDC(pgems[i][j].hWnd);
			RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = pgems[i][j].size + 4;
			rc.bottom = pgems[i][j].size + 4;
			pgems[i][j].color = rand() % 6;

			FillRect(hdc, &rc, color[pgems[i][j].color]);
			ReleaseDC(pgems[i][j].hWnd, hdc);
			Sleep(50);
		}
	}
	hasGameStarted = 1;

	if (LogicBoolOnly())
	{
		moveOrNot = 1;
		background(frame);
		Logic();
		row = 0;
	}

}

void CorrectColor(HWND hwnd)
{
	Kwadrat_t k = gemWindow(hwnd);	
	switch (k.kill_or_survive) 
	{
	case 0:
	{
		HDC hdc = GetDC(k.hWnd);
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = k.size + 4;
		rc.bottom = k.size + 4;
		FillRect(hdc, &rc, gray);
		ReleaseDC(k.hWnd, hdc);
	}
		break;
	case 1:
	{
		HDC hdc = GetDC(k.hWnd);
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = k.size + 4;
		rc.bottom = k.size + 4;
		FillRect(hdc, &rc, color[k.color]);
		ReleaseDC(k.hWnd, hdc);
	}
		break;
	case 2:
	{
		HDC hdc = GetDC(k.hWnd);
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = k.size + 4;
		rc.bottom = k.size + 4;
		FillRect(hdc, &rc, CreateHatchBrush(HS_CROSS, colorr[k.color]));
		ReleaseDC(k.hWnd, hdc);
	} break;
	case 3:
		HDC hdc = GetDC(k.hWnd);
		RECT rc = { 4,4,k.size - 4,k.size - 4 };
		FillRect(hdc, &rc, color[k.color]);
		ReleaseDC(k.hWnd, hdc);
		break;
	}
}

bool isMovePosible() 
{
	POINT first = getCords(mouse.first.hWnd);
	POINT second = getCords(mouse.second.hWnd);

	if ((second.x == first.x && (second.y + 1 == first.y || second.y - 1 == first.y)) ||
		(second.y == first.y && (second.x + 1 == first.x || second.x - 1 == first.x)))
		return LogicBoolOnly();
	else return false;

}

void MouseFunctionBeforeGame(HWND hWnd) 
{
	if (mouse.firstTrue) {
		POINT p1 = getCords(mouse.first.hWnd);
		pgems[p1.y][p1.x].kill_or_survive = 0;
		mouse.firstTrue = false;
		InvalidateRect(mouse.first.hWnd, 0, 1);
		return;
	}
	else 
	{
		POINT p1 = getCords(hWnd);
		pgems[p1.y][p1.x].kill_or_survive = 4;
		mouse.first = gemWindow(hWnd);
		mouse.firstTrue = true;
		
		InvalidateRect(hWnd, 0, 1);
	}
}

void MouseFunction(HWND hWnd) {
	if (mouse.firstTrue) {
		mouse.second = gemWindow(hWnd);
		POINT p1 = getCords(hWnd);
		POINT p2 = getCords(mouse.first.hWnd);

		pgems[p1.y][p1.x].color = mouse.first.color;
		pgems[p2.y][p2.x].color = mouse.second.color;

		if (!isMovePosible())
		{
			pgems[p1.y][p1.x].color = mouse.second.color;
			pgems[p2.y][p2.x].color = mouse.first.color;
			
			pgems[p2.y][p2.x].kill_or_survive = 1;
			mouse.firstTrue = false;
			InvalidateRect(hWnd, 0, 1);
			InvalidateRect(mouse.first.hWnd, 0, 1);
			UpdateWindow(hWnd);
			UpdateWindow(mouse.first.hWnd);
		}
		else
		{
			mouse.firstTrue = false;
			pgems[p2.y][p2.x].kill_or_survive = 1;
			InvalidateRect(mouse.first.hWnd, 0, 1);
			UpdateWindow(mouse.first.hWnd);

			moveOrNot = 1;
			background(frame);
			Logic();
			row = 0;
		}
	}
	else {
		POINT p1 = getCords(hWnd);
		pgems[p1.y][p1.x].kill_or_survive = 3;
		InvalidateRect(hWnd, 0, 1);
		UpdateWindow(hWnd);
		
		mouse.first = gemWindow(hWnd);
		mouse.firstTrue = true;
		InvalidateRect(hWnd, 0, 1);
		UpdateWindow(hWnd);
		
	}
}

bool Logic() 
{
	int tmpColor = -1;
	int sum = 0;
	int k = 0;
	bool any = false;
	for (int i = 0; i < size_g; i++) 
	{
		for (int j = 0; j < size_g; j++)
		{
			tmpColor = pgems[i][j].color;
			sum = 1;
			for(k = 1; j+k<size_g;k++)
			{
				if (pgems[i][j + k].color == tmpColor) sum++;
				
				if (pgems[i][j + k].color != tmpColor || j + k == (size_g - 1))
				{
					if (sum > 2)
					{
						any = true;
						for (int u = 0; u < sum; u++)
						{
							pgems[i][j + u].kill_or_survive = 2;
							CorrectColor(pgems[i][j + u].hWnd);
							CreateParticle(pgems[i][j + u].hWnd);
						}
					}
					break;
				}
			}
			j = j - 1 + k;
		}
	}
	
	for (int i = 0; i < size_g; i++)
	{
		for (int j = 0; j < size_g; j++)
		{
			tmpColor = pgems[j][i].color;
			sum = 1;
			for (k = 1; j + k < size_g; k++)
			{
				if (pgems[j+k][i].color == tmpColor) sum++;
				
				if (pgems[j + k][i].color != tmpColor || j + k == (size_g - 1)) 
				{
					if (sum > 2)
					{
						any = true;
						for (int u = 0; u < sum; u++)
						{
							pgems[j+u][i].kill_or_survive = 2;
							CorrectColor(pgems[j+u][i].hWnd);
							CreateParticle(pgems[j + u][i].hWnd);
						}
						
					}
					break;
				}
			}
			j = j - 1 + k;
		}
	}
	return any;
}

bool LogicBoolOnly()
{
	int tmpColor = -1;
	int sum = 0;
	int k = 0;
	for (int i = 0; i < size_g; i++)
	{
		for (int j = 0; j < size_g; j++)
		{
			tmpColor = pgems[i][j].color;
			sum = 1;
			for (k = 1; j + k < size_g; k++)
			{
				if (pgems[i][j + k].color == tmpColor) sum++;

				if (pgems[i][j + k].color != tmpColor || j + k == (size_g - 1))
				{
					if (sum > 2)
					{
						return true;
					}
					break;
				}
			}
			j = j - 1 + k;
		}
	}
	for (int i = 0; i < size_g; i++)
	{
		for (int j = 0; j < size_g; j++)
		{
			tmpColor = pgems[j][i].color;
			sum = 1;
			for (k = 1; j + k < size_g; k++)
			{
				if (pgems[j + k][i].color == tmpColor) sum++;

				if (pgems[j + k][i].color != tmpColor || j + k == (size_g - 1))
				{
					if (sum > 2)
					{
						return true;
					}
					break;
				}
			}
			j = j - 1 + k;
		}
	}
	return false;
}

void newColors(int i, int j)
{
	if (pgems[i][j].kill_or_survive == 1 ||
		pgems[i][j].kill_or_survive == 0) return;

	int tmpColorOld = rand() % 6;
	int tmpColor = rand() % 6;
	if (i == 0) {
		pgems[i][j].color = rand() % 6;
		pgems[i][j].kill_or_survive = 1;
		CorrectColor(pgems[i][j].hWnd);
		return;
	}
	for (int k = 0; k <= i; k++) 
	{
		tmpColorOld = pgems[k][j].color;
		pgems[k][j].color = tmpColor;
		tmpColor = tmpColorOld;
		if (pgems[k][j].kill_or_survive == 2)
			pgems[k][j].kill_or_survive = 1;
		CorrectColor(pgems[k][j].hWnd);
	}
}

void clearOneByOne() {
	for (int j = 0; j < size_g; j++) 
	{
		newColors(row, j);
	}
}

void background(HBRUSH color)
{
	HDC hdc = GetDC(hMain);
	RECT rc;
	GetClientRect(hMain, &rc);
	FillRect(hdc, &rc, color);
	ReleaseDC(hMain, hdc);
}

bool Update() 
{
	if (Logic())
	{
		row = 0;
		return true;
	}
	else
	{
		moveOrNot = 0;
		background(white);
		row = -1;
		return false;
	}
}