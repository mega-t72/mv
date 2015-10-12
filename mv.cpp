// mv.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mv.h"

#define MAX_LOADSTRING 100

typedef struct tag_mon{
    RECT            rc;
    std::wstring    name;
}mon;

// Global Variables:
std::vector< mon >mons;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MV));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
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
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MV));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MV);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx( 0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
BOOL CALLBACK monEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ){
    mon m;
    MONITORINFOEXW mi;
    //
    mi.cbSize   = sizeof( mi );
    ::GetMonitorInfoW( hMonitor, &mi );
    //
    m.rc    = *lprcMonitor;
    m.name  = mi.szDevice;
    //
    mons.push_back( m );
    return TRUE;
}
//
void update( void ){
    mons.clear();
    EnumDisplayMonitors( nullptr, nullptr, monEnumProc, 0 );
}
//
void scale( int wsrc, int hsrc, int wdst, int hdst, int *cx, int *cy ){
    for( *cx = wsrc, *cy = hsrc ; ( *cx > wdst ) || ( *cy > hdst ) ; ){
        if( *cx > wdst ){
            *cx = wdst;
            *cy = ( wdst * hsrc ) / wsrc;
        }
        if( *cy > hdst ){
            *cy = hdst;
            *cx = ( hdst * wsrc ) / hsrc;
        }
    }
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MONITORINFOEXW mi;
    ICONINFO ii;
    CURSORINFO mouse;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
    RECT rc;
	HDC hdc, hdtop, hMemDC;
    HGDIOBJ hMemBitmap, hSysBitmap;
    HCURSOR hcur;
    size_t selected;
    mon sel;
    int x, y, cx, cy, wsrc, hsrc, wdst, hdst;

	switch (message)
	{
	case WM_CREATE:
        update();
        ::SetTimer( hWnd, 12345, 10, nullptr );
        break;
	case WM_TIMER:
        update();
        ::InvalidateRect( hWnd, nullptr, TRUE );
        break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
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
		break;
	case WM_ERASEBKGND:
        return 1;
	case WM_PAINT:
        //  cur mon
        mi.cbSize   = sizeof( mi );
        ::GetMonitorInfoW( ::MonitorFromWindow( hWnd, 0 ), &mi );
        //  mouse mon
        selected        = 0;
        mouse.cbSize    = sizeof( mouse );
        ::GetCursorInfo( &mouse );
        ::GetClientRect( hWnd, &rc );
        for( auto i : mons ){
            if( ::PtInRect( &i.rc, mouse.ptScreenPos ) ){
                break;
            }
            ++selected;
        };
        sel     = mons[selected];
        //  paint
		hdc         = BeginPaint( hWnd, &ps );
        hMemDC      = ::CreateCompatibleDC( hdc );
        hMemBitmap  = ::CreateCompatibleBitmap( hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top );
        hSysBitmap  = ::SelectObject( hMemDC, hMemBitmap );
        ::SetWindowOrgEx( hMemDC, ps.rcPaint.left, ps.rcPaint.top, nullptr );
        ::FillRect( hMemDC, &ps.rcPaint, (HBRUSH)::GetStockObject( WHITE_BRUSH ) );
        ::SetBkMode( hMemDC, TRANSPARENT );
        ::SelectObject( hMemDC, ::GetStockObject( ANSI_VAR_FONT ) );
        if( wcscmp( sel.name.c_str(), mi.szDevice ) ){
            wdst    = rc.right - rc.left;
            hdst    = rc.bottom - rc.top;
            wsrc    = sel.rc.right - sel.rc.left;
            hsrc    = sel.rc.bottom - sel.rc.top;
            scale( wsrc, hsrc, wdst, hdst, &cx, &cy );
            hdtop   = ::CreateDC( TEXT( "DISPLAY" ), nullptr, nullptr, nullptr );
            ::StretchBlt( hMemDC, ( wdst - cx ) / 2, ( hdst - cy ) / 2, cx, cy, hdtop, sel.rc.left, sel.rc.top, wsrc, hsrc, SRCCOPY );
            ::DeleteDC( hdtop );
            //
            if( mouse.flags & CURSOR_SHOWING ){
                hcur    = ::GetCursor();
                x       = mouse.ptScreenPos.x - sel.rc.left;
                y       = mouse.ptScreenPos.y - sel.rc.top;
                ::GetIconInfo( hcur, &ii );
                ::DrawIcon( hMemDC, x * cx / wsrc + ( wdst - cx ) / 2 - ii.xHotspot, y * cy / hsrc + ( hdst - cy ) / 2 - ii.yHotspot, hcur );
            }
        }else{
            ::DrawTextW( hMemDC, L"Текущий монитор, изображение не дублируется", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS );
        }
        ::InflateRect( &rc, -5, -5 );
        ::DrawTextW( hMemDC, ( L"Монитор: " + sel.name ).c_str(), -1, &rc, DT_LEFT | DT_TOP );
        ::BitBlt( hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, hMemDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY );
        ::SelectObject( hMemDC, hSysBitmap );
        ::DeleteDC( hMemDC );
        ::DeleteObject( hMemBitmap );
		::EndPaint( hWnd, &ps );
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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
