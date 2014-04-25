// save ref: http://msdn.microsoft.com/en-us/library/ms646928(v=vs.85).aspx

#include <cstdlib>

#include <chrono>
#include <thread>
#include <string>

#ifdef _WIN32
#   include <Windows.h>
#   include <Shlobj.h>                     // SHGetFolderPath
#   pragma comment(lib, "Shell32.lib")     // SHGetFolderPath
#   include <Shellapi.h>                   // ShellExecuteA
#   pragma comment(lib,"Shell32.lib")      // ShellExecuteA
#   pragma comment(lib,"user32.lib")       // clipboard, messageboxa
#   include <Commdlg.h>
#   pragma comment(lib,"Comdlg32.lib")
#endif

#include "heal.hpp"
#include "heal2.hpp"

namespace
{
    void preclose_app();

    volatile bool running = ( std::atexit(preclose_app), true );

    void preclose_app() {
        running = false;
        sleep_app( 0.5 );
    }

    std::string repath( std::string t, char from, char to ) {
        for( std::string::iterator it = t.begin(), end = t.end(); it != end; ++it ) {
            if( *it == from ) {
                *it = to;
            }
        }
        return t;
    }

    std::string get_environment( const char *const k ) {
        return getenv(k) ? getenv(k) : "";
    }

    $windows(
        // http://www.guidgen.com/
        // http://support.microsoft.com/kb/243953/en-us
        // This code is from Q243953 in case you lose the article and wonder where this code came from.
        class limit_single_instance
        {
            protected:

            DWORD  last_error;
            HANDLE mutex;

            public:

            limit_single_instance( const char *guidgen_sz ) {
                //Make sure that you use a name that is unique for this application otherwise
                //two apps may think they are the same if they are using same name for
                //3rd parm to CreateMutex
                mutex = CreateMutexA(NULL, FALSE, guidgen_sz); //do early
                last_error = GetLastError(); //save for use later...
            }

            ~limit_single_instance() {
                if( mutex )
                {
                    CloseHandle(mutex);
                    mutex = NULL;
                }
            }

            bool is_another_instance_running() {
                return (ERROR_ALREADY_EXISTS == last_error ? true : false);
            }
        };

        //limit_single_instance obj( "Global\\{83D5A466-6473-4FF1-9544-93FBF84DC225}" );
        limit_single_instance obj( "Global\\{AppName:Moon9Sample}" );
    )
}

bool is_gold() {
    return $gold( true ) + false;
}

bool is_coder() {
    $gold( return false );
    static bool CODER = ( get_clipboard() == "CODER" || get_environment("USERLEVEL") == "CODER" );
    return CODER;
}
bool is_tester() {
    $gold( return false );
    static bool TESTER = ( get_clipboard() == "TESTER" || get_environment("USERLEVEL") == "TESTER" );
    return TESTER;
}
bool is_enduser() {
    $gold( return true );
    static bool ENDUSER = (!is_coder()) && (!is_tester());
    return ENDUSER;
}

void browse( const std::string &url ) {
    $windows(
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    )
}

bool make_dir( const std::string &fullpath ) {
    $windows(
        int ret = SHCreateDirectoryExA(0, repath( fullpath, '/', '\\' ).c_str(), 0);
        return ( ret == ERROR_SUCCESS || ret == ERROR_ALREADY_EXISTS );
    )
    $welse(
        //  read/write/search permissions for owner /* and group, and with read/search permissions for others. */
        int ret = mkdir( fullpath.c_str(), S_IRWXU  /*| S_IRWXG | S_IROTH | S_IXOTH */ ) == 0;
        return ( ret == 0 || ret == EEXIST );
    )
}

std::string get_homedir() {
    // get user documents path
    $windows(
        char cpath[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_DEFAULT, cpath);
        return repath( cpath, '\\', '/' );
    )
    $welse(
        return getenv("HOME");
    )
}

std::string get_tempdir() {
    $windows(
        return repath( getenv("LOCALAPPDATA"), '\\', '/' );
    )
    $welse(
        return getenv("TEMP");
    )
}

bool is_app_instanced() {
    $windows(
        return obj.is_another_instance_running();
    )
    $welse(
        return false;
    )
}

std::string get_app_name() {
    $windows(
        HMODULE self = GetModuleHandle(0);
        char filename[512];
        DWORD result = GetModuleFileNameA(self,filename,sizeof(filename));
        return result > 0 ? filename : std::string();
    )
    $welse(
        return std::string();
    )
}

void attach_console() {
    $windows(
        // these next few lines create and attach a console
        // to this process.  note that each process is only allowed one console.
        AllocConsole() ;
        //AttachConsole( GetCurrentProcessId() ) ;
        //freopen( "CON", "w", stdout ) ;
        //printf("HELLO!!! I AM THE CONSOLE!" ) ;
    )
}

void detach_console() {
    $windows(
        FreeConsole();
    )
}

void set_title_console( const std::string &title ) {
    $windows(
        SetConsoleTitleA( title.c_str() );
    )
}

void sleep_app( double seconds ) {
    std::chrono::microseconds duration( (int)(seconds * 1000000) );
    std::this_thread::sleep_for( duration );
}

void spawn_app( const std::string &appname ) {
    sleep_app( 0.5 );
    std::thread( [=](){ std::system( appname.c_str() ); } ).detach();
}

void respawn_app() {
    spawn_app( get_app_name() );
}

bool is_app_running() {
    return running;
}

bool is_app_closing() {
    return !running;
}

void close_app() {
    std::atexit( preclose_app );
    std::exit( 0 );
}

void restart_app() {
#if 0
    respawn();
    close();
#else
    std::atexit( respawn_app );
    close_app();
#endif
}

std::string get_clipboard() {
    std::string result;
    $windows(
        if( IsClipboardFormatAvailable(CF_TEXT) != 0 && OpenClipboard(0) != 0 ) {
            HGLOBAL hglb = GetClipboardData(CF_TEXT);
            if (hglb != NULL) {
                LPSTR lpstr = (LPSTR)GlobalLock(hglb);
                if (lpstr != NULL) {
                    result = lpstr;
                    GlobalUnlock(hglb);
                }
            }
            CloseClipboard();
        }
    )
    return result;
}
void set_clipboard( const std::string &_msg ) {
    $windows(
        // try to copy error to clipboard
        if( OpenClipboard(0) ) {
            HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, _msg.size()+1 );
            if( hglbCopy ) {
                // Lock the handle and copy the text to the buffer.
                LPSTR lpstrCopy = (LPSTR)GlobalLock(hglbCopy);
                if( lpstrCopy ) {
                    strcpy( lpstrCopy, _msg.c_str() );
                    GlobalUnlock(hglbCopy);

                    EmptyClipboard();
                    SetClipboardData( CF_TEXT, hglbCopy );
                }
            }

            CloseClipboard();
        }
    )
}

bool confirm( const std::string &question, const std::string &title ) {
    $windows( return MessageBoxA( HWND_DESKTOP, question.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO | MB_SYSTEMMODAL ) == IDYES );
    $linux( return std::system( std::string("zenity --question --title=\"") + title + ("\" --text \"") + question + ("\"").c_str() ) != 0 );
    return false;
}

namespace
{
    $windows(
    std::string load_dialog( const char *filter = "All Files (*.*)\0*.*\0", HWND owner = 0 )
    {
        OPENFILENAMEA ofn;
        char fileName[ MAX_PATH ] = "";
        ZeroMemory( &ofn, sizeof(ofn) );

        ofn.lStructSize = sizeof( OPENFILENAMEA );
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = ""; // optional

        return GetOpenFileNameA( &ofn ) == TRUE ? fileName : std::string();
    }

    std::string save_dialog( const char *filter = "All Files (*.*)\0*.*\0", HWND owner = 0 )
    {
        OPENFILENAMEA ofn;
        char fileName[ MAX_PATH ] = "";
        ZeroMemory( &ofn, sizeof(ofn) );

        ofn.lStructSize = sizeof( OPENFILENAMEA );
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = ""; // optional

        return GetSaveFileNameA( &ofn ) == TRUE ? fileName : std::string();
    }
    )
}

std::string load_dialog( const char *filter ) {
    return $windows( load_dialog( filter, 0 ) ) $welse("");
}

std::string save_dialog( const char *filter ) {
    return $windows( save_dialog( filter, 0 ) ) $welse("");
}
