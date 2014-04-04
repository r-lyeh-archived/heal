#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"advapi32.lib")
#endif

#define $yes(...) __VA_ARGS__
#define $no(...)

#ifdef _WIN32
#define $win32 $yes
#define $welse $no
#else
#define $win32 $no
#define $welse $yes
#endif


struct window
{
    bool closing;

    $welse(
        typedef int HWND;
        typedef unsigned DWORD;
    )

    HWND hWnd;

    window() : hWnd(0), closing(true)
    {}

    bool create( const std::string &title = std::string() )
    {
        $win32(
        WNDCLASSW   wc;
        ZeroMemory( &wc, sizeof( WNDCLASSW ) );

        wc.style            = 0; // CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc      = DefWindowProc;
        wc.hInstance        = GetModuleHandle(NULL);
        wc.lpszClassName    = L"kult::win32";
        wc.hCursor          = LoadCursor( NULL, IDC_ARROW );

        RegisterClassW( &wc );

        hWnd = CreateWindowExW( (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) , wc.lpszClassName, L"", (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),

            //-1, -1, 1004, 650,
            CW_USEDEFAULT, CW_USEDEFAULT, 1004, 650,

            NULL, NULL, GetModuleHandle(NULL), NULL );

        SetWindowTextA( hWnd, title.c_str() );
        ShowWindow( hWnd, SW_SHOW );
        SetForegroundWindow( hWnd );
        SetFocus( hWnd );

        closing = false;
        return true;
        )
        return false;
    }

    void set_title( const std::string &title ) {
        $win32(
        SetWindowTextA( hWnd, title.c_str() );
        )
    }

    bool is_open()
    {
        if( closing ) return false;
        $win32(
        return IsWindowVisible( hWnd ) == FALSE ? false : true;
        )
        return false;
    }

    void pump()
    {
        $win32(
        MSG msg;
        // If we have a message from windows..
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            // .. give it to the input handler to process
            // GwenInput.ProcessMessage( msg );

            // if it's QUIT then quit..
            if ( msg.message == WM_QUIT )
                { closing = false; return; }

            if ( msg.message == WM_PAINT )
            {
                // This doesn't actually draw it, it just marks it
                // so it will redraw when next checked (NeedsRedraw)
                // pCanvas->Redraw();
            }

            // Handle the regular window stuff..
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        )
    }
};

struct wingrid
{
    $welse(
        typedef int HWND;
        typedef unsigned DWORD;
    )

    HWND list;

    wingrid() : list(0) {
    }

    ~wingrid() {
        destroy();
    }

private:
    HWND CreateListView (HWND hwndParent)
    {
        $win32(
        INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        RECT rcClient;                       // The parent window's client area.
        GetClientRect (hwndParent, &rcClient);

        // Create the list-view window in report view with label editing enabled.
        HWND hWndListView;

        if(1)
        // not w
        hWndListView = CreateWindowA(WC_LISTVIEW,
                                         "",
                                         WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                                         0, 0,
                                         rcClient.right - rcClient.left,
                                         rcClient.bottom - rcClient.top,
                                         hwndParent,
                                         (HMENU)0,
                                         (HINSTANCE)GetModuleHandle(0),
                                         NULL);
        else
        // working
        hWndListView = CreateWindowA(WC_LISTVIEW,
                                        0,
                                        WS_VISIBLE|WS_SIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|LVS_REPORT,
                                        CW_USEDEFAULT,CW_USEDEFAULT,
                                        CW_USEDEFAULT,CW_USEDEFAULT,
                                        (HWND)hwndParent,
                                        (HMENU)0,
                                        (HINSTANCE)0,
                                        0);

        ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_INFOTIP|LVS_EX_ONECLICKACTIVATE|0x10000);
        SetWindowLong( ListView_GetHeader(hWndListView),GWL_STYLE,GetWindowLong(ListView_GetHeader(hWndListView), GWL_STYLE)^HDS_BUTTONS );

        return (hWndListView);
        )

        return 0;
    }

    // SetView: Sets a list-view's window style to change the view.
    // hWndListView: A handle to the list-view control.
    // dwView:       A value specifying the new view style.
    //
    void SetView(HWND hWndListView, DWORD dwView)
    {
        $win32(
        // Retrieve the current window style.
        DWORD dwStyle = GetWindowLong(hWndListView, GWL_STYLE);

        // Set the window style only if the view bits changed.
        if ((dwStyle & LVS_TYPEMASK) != dwView)
        {
            SetWindowLong(hWndListView,
                          GWL_STYLE,
                          (dwStyle & ~LVS_TYPEMASK) | dwView);
        }               // Logical OR'ing of dwView with the result of
                        // a bitwise AND between dwStyle and
                        // the Unary complenent of LVS_TYPEMASK.
        )
    }

public:

    bool create( HWND parent = 0 )
    {
        $win32(
        if(0)
        list=CreateWindowA(WC_LISTVIEW,0,WS_VISIBLE|WS_SIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|LVS_REPORT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,(HWND)parent,(HMENU)0,(HINSTANCE)0,0);
        else
        list=CreateListView(parent);
        )
        return true;
    }

    void destroy() {
        $win32(
        if( list )
            DestroyWindow(list);
        list = 0;
        )
    }

    void insert_column( char* txt, int row, int col, int len ) {
        $win32(
        LVCOLUMN c={LVCF_WIDTH|LVCF_TEXT,0,len,txt,col};
        ListView_InsertColumn(list,col,&c);
        )
    }

    void insert_row( char* txt, int row, int col, int typ ) {
        $win32(
        LVITEM v={LVIF_TEXT,row,0,0,0,txt};
        ListView_InsertItem (list,&v);
        )
    }

    void modify_field( char* txt, int row, int col, int typ ) {
        $win32(
        LVITEM v={LVIF_TEXT,row,0,0,0,txt};
        ListView_SetItemText(list,row,col,txt);
        )
    }
};

#undef $yes
#undef $no

#undef $win32
#undef $welse