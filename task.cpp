// [ref] http://msdn.microsoft.com/en-us/library/windows/desktop/bb760441(v=vs.85).aspx
// [ref] http://msdn.microsoft.com/en-us/library/windows/desktop/bb760540(v=vs.85).aspx
// [ref] http://www.codeproject.com/Articles/16806/Vista-Goodies-in-C-Using-TaskDialogIndirect-to-Bui
// [ref] http://msdn.microsoft.com/en-us/library/vstudio/dd234915.aspx

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifndef TD_SHIELD_ICON
#define TD_SHIELD_ICON          MAKEINTRESOURCEW(-4)
#endif

int main1()
{
    int nButton;
    TaskDialog(NULL, NULL, L"TaskDialog Test", L"Hello, World!", L"This is a TaskDialog demonstration", TDCBF_OK_BUTTON,

#if 0
               TD_INFORMATION_ICON,
#else
                TD_SHIELD_ICON,
#endif

               &nButton);
return nButton;
}


#if 1
#include <iostream>
#include <map>
struct ui_details {
    enum icons { info, warning, error, shield, none, total };
    bool should_close;
    int progress;
    int main_icon, footer_icon;
    double timeout;
    std::wstring title, head, body, footer;

    ui_details() : should_close(true), progress(~0), timeout(10), main_icon(info), footer_icon(warning) {
    }

    void reset() {
        *this = ui_details();
    }
};
ui_details& get()  {
    static ui_details ui;
    return ui;
}
/*
push, pop, diff, flat(), unflat()
top, bottom
template<typename T>
void push( const T& t ) {
    static std::map< std::thread::id, std::vector<T> > all;
    std::thread::id self = std::this_thread::get_id();
    return ( all[ self ] = all[ self ] ) = t;
}
*/
#endif

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"shell32.lib")

HRESULT CALLBACK TDCallback (
    HWND hwnd, UINT uNotification, WPARAM wParam,
    LPARAM lParam, LONG_PTR dwRefData )
{
    switch ( uNotification )
    {
        case TDN_DIALOG_CONSTRUCTED:

            SendMessage( hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, IDOK, 1 );

            SendMessage( hwnd, TDM_SET_MARQUEE_PROGRESS_BAR, 0 /*off*/, 0 );                // marquee off
            SendMessage( hwnd, TDM_SET_PROGRESS_BAR_MARQUEE, 1 /*on*/, 0 /*30ms*/ );        // marquee speed

            SendMessage( hwnd, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 100) );         // progress range
            SendMessage( hwnd, TDM_SET_PROGRESS_BAR_POS, int(0), 0 );                       // progress at

            get().progress = 0;
            get().timeout = 10;

            return TDCallback( hwnd, TDN_TIMER, wParam, lParam, dwRefData ), S_OK;

        case TDN_DESTROYED :
            break;

        case TDN_HYPERLINK_CLICKED:
            ShellExecute( hwnd, L"open", (LPCWSTR) lParam, NULL, NULL, SW_SHOW );

            // you can also elevate a process by using explorer. ie,
            // system("explorer.exe path/to/cmd.exe");
            /*
            ShellExecute(
                NULL,
                L"runas",    // Trick for requesting elevation, this is a verb not listed in the documentation above.
                L"cmd.exe",
                NULL,        // params
                NULL,        // directory
                SW_HIDE);
            */

            break;

        case TDN_TIMER: {
            // reconstruct progress bar if needed
            ui_details &ui = get();

            if( ui.timeout > 0 )
                ui.timeout -= 0.200;

            int &now = ui.progress, before = now;
            int was_valid = ( ui.progress >= 0 && ui.progress <= 100);

            if( GetAsyncKeyState(VK_F10) ) {
                ui.progress -= 10;
            } else
            if( GetAsyncKeyState(VK_F11) ) {
                ui.progress += 10;
            }

            int is_valid = ( ui.progress >= 0 && ui.progress <= 100);
            int diff = now - before;

            /**/ if( (!is_valid) && was_valid ) {
                SendMessage( hwnd, TDM_SET_MARQUEE_PROGRESS_BAR, 1 /*show*/, 0 );           // out of range; marquee on
            }
            else if( is_valid && (!was_valid) ) {
                SendMessage( hwnd, TDM_SET_MARQUEE_PROGRESS_BAR, 0 /*show*/, 0 );           // in range; marquee off
            }
            else if( diff ) {
                SendMessage( hwnd, TDM_SET_PROGRESS_BAR_POS, ui.progress, 0 );              // at
            }

            static int timer = 0; timer = (++timer)%4;
            std::wstring bar = std::wstring( 1, L"|/-\\"[timer] );
            std::wstring dots = std::wstring( timer, L'.' );

            ui.title = L"Modal test";

            ui.head = L"Change Password";

            ui.body = L"Remember your changed password.";
            ui.body += dots;

            ui.footer = L"This is a footer. <a href=\"http://www.google.com/\">Full details about this update</a>";
            ui.footer += L"[";
            ui.footer += bar;
            ui.footer += L"]";
            ui.footer += dots;

            // window title
            SetWindowText( hwnd, ui.title.c_str() );

            // head
            SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_MAIN_INSTRUCTION, (LPARAM)( ui.head.c_str() ));

            // body
            SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_CONTENT, (LPARAM)( ui.body.c_str() ));

            // footer
            SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_FOOTER, (LPARAM)( ui.footer.c_str() ));

            // expanded
            // SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_EXPANDED_INFORMATION, (LPARAM)( get().footer.c_str() ));

            LPWSTR icons[] = { TD_INFORMATION_ICON, TD_WARNING_ICON, TD_ERROR_ICON, TD_SHIELD_ICON, NULL  };

            // update big icon
            if( ui.main_icon < 0 || ui.main_icon >= ui_details::total )
                ui.main_icon = ui_details::none;
            SendMessage(hwnd, TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_MAIN, (LPARAM)icons[ui.main_icon] );

            // update footer icon
            if( ui.footer_icon < 0 || ui.footer_icon >= ui_details::total )
                ui.footer_icon = ui_details::none;
            SendMessage(hwnd, TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_FOOTER, (LPARAM)icons[ui.footer_icon] );
        }

        /* following break is intentionally commented */
        // break;

        default:
           return get().should_close ? S_OK : S_FALSE;
    }
}

int main2()
{
    int nButtonPressed                  = 0;
    TASKDIALOGCONFIG config             = {0};
    const TASKDIALOG_BUTTON buttons[]   = {
                                            { IDOK, L"Change password" },
                                            { IDCANCEL, L"Skip this" }
                                          };

    config.cbSize                       = sizeof(config);
    config.hInstance                    = 0; //hInst;
    config.dwCommonButtons              = 0; //TDCBF_CANCEL_BUTTON;
    // if flat buttons
    config.pButtons                     = buttons;
    config.cButtons                     = ARRAYSIZE(buttons);
    config.dwFlags                     |= TDF_USE_COMMAND_LINKS;

    config.pszMainInstruction           = L"X";
    config.pszContent                   = L"X";
    config.pszFooter                    = L"X";
    //config.pszExpandedInformation       = L"X";

    config.pszMainIcon                  = 0;
    config.pszFooterIcon                = 0;

    config.dwFlags |= TDF_SHOW_PROGRESS_BAR | TDF_CAN_BE_MINIMIZED;
    config.dwFlags |= TDF_ENABLE_HYPERLINKS; //TDF_ALLOW_DIALOG_CANCELLATION

    config.pfCallback = TDCallback;
    config.lpCallbackData = (LONG_PTR) (0);
    config.dwFlags |= TDF_CALLBACK_TIMER;

    HRESULT hr = TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);

    if( SUCCEEDED(hr) )
    switch (nButtonPressed)
    {
        case IDYES:
            std::cout << "yes" << std::endl;
            break;
        case IDOK:
            std::cout << "ok" << std::endl;
            break; // the user pressed button 0 (change password).
        case IDCANCEL:
            std::cout << "cancel" << std::endl;
            break; // user canceled the dialog
        default:
            std::cout << "??" << std::endl;
            break; // should never happen
    }

    return 0;
}

int main() {
    main1();
    main2();
}
