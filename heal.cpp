/*
 * Heal is a lightweight C++ framework to aid and debug applications.
 * Copyright (c) 2011-2013 Mario 'rlyeh' Rodriguez

 * Callstack code is based on code by Magnus Norddahl (See http://goo.gl/LM5JB)

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * - rlyeh // ~listening to Kalas - Monuments to Ruins
 */

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#   include <Windows.h>
#   if defined(DEBUG) || defined(_DEBUG)
#       include <Crtdbg.h>
#   endif
#   include <Dbghelp.h>
#   pragma comment( lib, "dbghelp.lib" )
#   pragma comment( lib, "kernel32.lib" )
#   pragma comment( lib, "user32.lib" )
#else
#   include <unistd.h>
#   include <signal.h>
#   include <execinfo.h>
#   if defined(HAVE_SYS_SYSCTL_H) && \
        !defined(_SC_NPROCESSORS_ONLN) && !defined(_SC_NPROC_ONLN)
#       include <sys/time.h>
#       include <sys/types.h>
#       include <sys/param.h>
#       include <sys/sysctl.h>
#   else
#       include <sys/time.h>
#       include <sys/types.h>
#   endif
#endif

#if defined(__GNUC__) //&& defined(HAVE_CXA_DEMANGLE)
#   include <cxxabi.h>
#endif

// API

#include "heal.hpp"

// OS

#define $yes(...) __VA_ARGS__
#define $no(...)

#ifdef _WIN32
#   define $win32   $yes
#   define $defined $yes
#else
#   define $win32   $no
#endif

#ifdef __linux__
#   define $linux   $yes
#   define $defined $yes
#else
#   define $linux   $no
#endif

#ifdef __APPLE__
#   define $apple   $yes
#   define $defined $yes
#else
#   define $apple   $no
#endif

#ifdef $defined
#   undef  $defined
#   define $undefined $no
#else
#   define $undefined $yes
#endif

// COMPILERS

#ifdef _MSC_VER
#   define $msvc $yes
#else
#   define $msvc $no
#endif

#ifdef __GNUC__
#   define $gcc $yes
#else
#   define $gcc $no
#endif

#   define $other $yes

#if defined(NDEBUG) || defined(_NDEBUG)
#   define $release $yes
#   define $debug   $no
#else
#   define $debug   $yes
#   define $release $no
#endif

// INFO MESSAGES

#ifdef _MSC_VER
#    pragma message( "<heal/heal.cpp> says: do not forget /Zi, /Z7 or /C7 compiler settings! /Oy- also helps" )
#endif

#if   defined(__clang__)
#    warning "<heal/heal.cpp> says: do not forget -g compiler settings!"
#elif defined(__GNUC__)
#    warning "<heal/heal.cpp> says: do not forget -g -lpthread compiler settings!"
#endif

// ASSERT

namespace
{
    void default_assert_cb( const std::string &assertion, const char *at_file, int at_line )
    {
        errorbox( assertion );

        if( !debugger() ) {
            ; // oops :s
        }

        die( -1 );
    }
}

namespace cb
{
    void (*custom_assert_cb)( const std::string &assertion, const char *at_file, int at_line ) = default_assert_cb;
}

void setup_assert( void (*custom_assert_cb)( const std::string &assertion, const char *at_file, int at_line ) ) {
    cb::custom_assert_cb = custom_assert_cb ? custom_assert_cb : default_assert_cb;
}

bool is_asserting()
{
    /*
    static bool enabled = ( enabled = false, assert( enabled ^= true ), enabled );
    return enabled;
    */
    /*
    static struct once { bool are_enabled; once() : are_enabled(false) {
        assert( are_enabled ^= true );
    } } asserts;
    return asserts.are_enabled;
    */
    bool asserting = false;
    assert( asserting |= true );
    return asserting;
}

// IS_DEBUG
// IS_RELEASE

bool is_debug() {
    $debug(
    return true;
    )
    $release(
    return false;
    )
}
bool is_release() {
    $release(
    return true;
    )
    $debug(
    return false;
    )
}

// DEBUGGER

#ifdef __linux__

    // enable core dumps for debug builds
    // after a crash try to do something like 'gdb ./a.out core'
#   if defined(NDEBUG) || defined(_NDEBUG)
        const bool are_coredumps_enabled = false;
#   else
#       include <sys/resource.h>
        rlimit core_limit = { RLIM_INFINITY, RLIM_INFINITY };
        const bool are_coredumps_enabled = setrlimit( RLIMIT_CORE, &core_limit ) == 0;
#   endif

    struct file {
        static bool exists( const std::string &pathfile) {
        /*struct stat buffer;
          return stat( pathfile.c_str(), &buffer ) == 0; */
          return access( pathfile.c_str(), F_OK ) != -1; // _access(fn,0) on win
        }
    };

    bool has( const std::string &app ) {
        return file::exists( std::string("/usr/bin/") + app );
    }

    std::string pipe( const std::string &sys, const std::string &sys2 = std::string() ) {
        char buf[512];
        std::string out;

        FILE *fp = popen( (sys+sys2).c_str(), "r" );
        if( fp ) {
            while( !std::feof(fp) ) {
                if( std::fgets(buf,sizeof(buf),fp) != NULL ) {
                    out += buf;
                }
            }
            pclose(fp);
        }

        return out;
    }

    // gdb apparently opens FD(s) 3,4,5 (whereas a typical prog uses only stdin=0, stdout=1,stderr=2)
    // Silviocesare and xorl
    bool detect_gdb(void)
    {
        bool rc = false;
        FILE *fd = fopen("/tmp", "r");

        if( fileno(fd) > 5 )
            rc = true;

        fclose(fd);
        return rc;
    }

#endif

static inline void _breakpoint() {
// os based

    $win32(
    DebugBreak();
    )
    $linux(
    raise(SIGTRAP);
//    asm("trap");
//    asm("int3");
//    kill( getpid(), SIGINT );
    /*
    kill( getpid(), SIGSTOP );
    kill( getpid(), SIGTERM );
    kill( getpid(), SIGHUP );
    kill( getpid(), SIGTRAP );
    */
    // kill( getpid(), SIGSEGV );
    // raise(SIGTRAP); //POSIX
    // raise(SIGINT);  //POSIX
    )

// compiler based

    //msvc
    $msvc(
    // "With native code, I prefer using int 3 vs DebugBreak, because the int 3 occurs right in the same stack frame and module as the code in question, whereas DebugBreak occurs one level down in another DLL, as shown in this callstack:"
    // [ref] http://blogs.msdn.com/b/calvin_hsia/archive/2006/08/25/724572.aspx
    // __debugbreak();
    )

    // gcc
    $gcc(
    //__builtin_trap();
    //__asm__ __volatile__("int3");
    )

// standard

    //abort();
    //assert( !"<heal/heal.cpp> says: debugger() has been requested" );
    // still here? duh, maybe we are in release mode...

// host based

    //macosx: asm {trap}            ; Halts a program running on PPC32 or PPC64.
    //macosx: __asm {int 3}         ; Halts a program running on IA-32.

    //$x86( // ifdef _M_X86
    //__asm int 3;
    //)
}

void breakpoint() {
    _breakpoint();
}

bool debugger( const std::string &reason )
{
    if( reason.size() > 0 )
        errorbox( reason );

// os based

    $win32(
        if( IsDebuggerPresent() )
            return _breakpoint(), true;
    )

    $linux(
        if( detect_gdb() ) {
            return _breakpoint(), true;
        }
        // else try to invoke && attach to current process

        static std::string sys, tmpfile;
        sys = ( has("ddd") && false ? "/usr/bin/ddd" : ( has("gdb") ? "/usr/bin/gdb" : "" ));
        tmpfile = "./heal.tmp.tmp"; //pipe("tempfile");
        if( !sys.empty() ) {
            std::string pid = std::to_string( getpid() );
            // [ok]
            // eval-command=bt
            // -ex "bt full"
            // gdb --batch --quiet -ex "thread apply all bt full" -ex "quit" ${exe} ${corefile}
            sys = sys + (" --tui -q -ex 'set pagination off' -ex 'shell rm " +tmpfile+ "' -ex 'continue' -ex 'finish' -ex 'finish' -ex 'finish' --pid=") + pid + " --args `cat /proc/" + pid + "/cmdline`";
            if( has("xterm") && false ) {
                sys = std::string("/usr/bin/xterm 2>/dev/null -maximized -e \"") + sys + "\"";
            } else {
                //sys = std::string(/*"exec"*/ "/usr/bin/splitvt -upper \"") + sys + "\"";
                //sys = std::string("/bin/bash -c \"") + sys + " && /usr/bin/reset\"";
                sys = std::string("/bin/bash -c \"") + sys + "\"";
            }

        pipe( "echo heal.cpp says: waiting for debugger to catch pid > ", tmpfile );
        std::thread( system, sys.c_str() ).detach();
        while( file::exists(tmpfile) )
            usleep( 250000 );
        _breakpoint();
        return true;
        }
    )

    errorbox( "<heal/heal.cpp> says:\n\nDebugger invokation failed.\nPlease attach a debugger now.", "Error!");
    _breakpoint();
    return false;
}

// ERRORBOX

namespace {

    template<typename T>
    std::string stringme( const T &t ) {
        std::stringstream ss;
        ss.precision( 20 );
        ss << t;
        return ss.str();
    }

    template<>
    std::string stringme( const bool &boolean ) {
        return boolean ? "true" : "false";
    }

    void show( const std::string &body = std::string(), const std::string &title = std::string(), bool is_error = false ) {
        $win32(
            MessageBoxA( 0, body.c_str(), title.size() ? title.c_str() : "", 0 | ( is_error ? MB_ICONERROR : 0 ) | MB_SYSTEMMODAL );
            return;
        )
        $linux(
            if( has("whiptail") ) {
                // gtkdialog3
                // xmessage -file ~/.bashrc -buttons "Ok:1, Cancel:2, Help:3" -print -nearmouse
                //std::string cmd = std::string("/usr/bin/zenity --information --text \"") + body + std::string("\" --title=\"") + title + "\"";
                //std::string cmd = std::string("/usr/bin/dialog --title \"") + title + std::string("\" --msgbox \"") + body + "\" 0 0";
                std::string cmd = std::string("/usr/bin/whiptail --title \"") + title + std::string("\" --msgbox \"") + body + "\" 0 0";
                //std::string cmd = std::string("/usr/bin/xmessage \"") + title + body + "\"";
                std::system( cmd.c_str() );
            } else {
                fprintf( stderr, "%s", ( title.size() > 0 ? title + ": " + body + "\n" : body + "\n" ).c_str() );
            }
            return;
        )
        $undefined(
            fprintf( stderr, "%s", ( title.size() > 0 ? title + ": " + body + "\n" : body + "\n" ).c_str() );
            return;
        )
    }
}

void    alert(                                                   ) { show();                        }
void    alert( const        char *text, const std::string &title ) { show( text, title );           }
void    alert( const std::string &text, const std::string &title ) { show( text, title );           }
void    alert( const      size_t &text, const std::string &title ) { show( stringme(text), title ); }
void    alert( const      double &text, const std::string &title ) { show( stringme(text), title ); }
void    alert( const       float &text, const std::string &title ) { show( stringme(text), title ); }
void    alert( const         int &text, const std::string &title ) { show( stringme(text), title ); }
void    alert( const        char &text, const std::string &title ) { show( stringme(text), title ); }
void    alert( const        bool &text, const std::string &title ) { show( stringme(text), title ); }
void errorbox( const std::string &body, const std::string &title ) { show( body, title, true );     }

// DEMANGLE

std::string demangle( const std::string &name ) {
    $win32({
    char demangled[1024];
    return (UnDecorateSymbolName(name.c_str(), demangled, sizeof( demangled ), UNDNAME_COMPLETE)) ? demangled : name;
    })
    $gcc({
    char demangled[1024];
    size_t sz = sizeof(demangled);
    int status;
    abi::__cxa_demangle(name.c_str(), demangled, &sz, &status);
    return !status ? demangled : name;
    })
    $linux({
    FILE *fp = popen( (std::string("echo -n \"") + name + std::string("\" | c++filt" )).c_str(), "r" );
    if (!fp) { return name; }
    char demangled[1024];
    char *line_p = fgets(demangled, sizeof(demangled), fp);
    pclose(fp);
    return line_p;
    })
    $undefined(
    return name;
    )
}

// CALLSTACK

namespace //heal
{
    class string : public std::string
    {
        public:

        string() : std::string()
        {}

        template <typename T>
        explicit string( const T &t ) : std::string()
        {
            std::stringstream ss;
            if( ss << t )
                this->assign( ss.str() );
        }

        template <typename T1, typename T2>
        explicit string( const char *fmt12, const T1 &_t1, const T2 &_t2 ) : std::string()
        {
            string t1( _t1 );
            string t2( _t2 );

            string &s = *this;

            while( *fmt12 )
            {
                if( *fmt12 == '\1' )
                    s += t1;
                else
                if( *fmt12 == '\2' )
                    s += t2;
                else
                    s += *fmt12;

                fmt12++;
            }
        }

        size_t count( const std::string &substr ) const
        {
            size_t n = 0;
            std::string::size_type pos = 0;
            while( (pos = this->find( substr, pos )) != std::string::npos ) {
                n++;
                pos += substr.size();
            }
            return n;
        }
    };

    typedef std::vector<std::string> strings;

    size_t capture_stack_trace(int frames_to_skip, int max_frames, void **out_frames, unsigned int *out_hash = 0)
    {
        $win32(
            if (max_frames > 32)
                    max_frames = 32;

            unsigned short capturedFrames = 0;

            // RtlCaptureStackBackTrace is only available on Windows XP or newer versions of Windows
            typedef WORD(NTAPI FuncRtlCaptureStackBackTrace)(DWORD, DWORD, PVOID *, PDWORD);

            static struct raii
            {
                raii() : module(0), ptrRtlCaptureStackBackTrace(0)
                {
                    module = LoadLibraryA("kernel32.dll");
                    if( module )
                        ptrRtlCaptureStackBackTrace = (FuncRtlCaptureStackBackTrace *)GetProcAddress(module, "RtlCaptureStackBackTrace");
                }
                ~raii() { if(module) FreeLibrary(module); }

                HMODULE module;
                FuncRtlCaptureStackBackTrace *ptrRtlCaptureStackBackTrace;
            } module;

            if (module.ptrRtlCaptureStackBackTrace)
                    capturedFrames = module.ptrRtlCaptureStackBackTrace(frames_to_skip+1, max_frames, out_frames, (DWORD *) out_hash);

            if (capturedFrames == 0 && out_hash)
                    *out_hash = 0;

            return capturedFrames;
        )
        $gcc(
            // Ensure the output is cleared
            memset(out_frames, 0, (sizeof(void *)) * max_frames);

            if (out_hash)
                    *out_hash = 0;

            return (backtrace(out_frames, max_frames));
        )
        $other(
            return 0;
        )
    }

    strings get_stack_trace(void **frames, int num_frames)
    {
        $win32(
            // this mutex is used to prevent race conditions.
            // however, it is constructed with heap based plus placement-new just to meet next features:
            // a) ready to use before program begins.
            // our memtracer uses callstack() and mutex is ready to use before the very first new() is made.
            // b) ready to use after program ends.
            // our memtracer uses callstack() when making the final report after the whole program has finished.
            // c) allocations free: memory is taken from heap, and constructed thru placement new
            // we will avoid recursive deadlocks that would happen in a new()->memtracer->callstack->new()[...] scenario.
            // d) leaks free: zero global allocations are made.
            // we don't polute memmanager/memtracer reports with false positives.
            static std::mutex *mutex = 0;
            if( !mutex )
            {
                static char placement[ sizeof(std::mutex) ];
                mutex = (std::mutex *)placement; // no leak and no memory traced :P
                new (mutex) std::mutex();        // memtraced recursion safe; we don't track placement-news
            }

            mutex->lock();

            BOOL result = SymInitialize(GetCurrentProcess(), NULL, TRUE);
            if (!result)
                return mutex->unlock(), strings();

            strings backtrace_text;
            for( int i = 0; i < num_frames; i++ )
            {
                char buffer[sizeof(IMAGEHLP_SYMBOL64) + 128];
                IMAGEHLP_SYMBOL64 *symbol64 = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buffer);
                memset(symbol64, 0, sizeof(IMAGEHLP_SYMBOL64) + 128);
                symbol64->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
                symbol64->MaxNameLength = 128;

                DWORD64 displacement = 0;
                BOOL result = SymGetSymFromAddr64(GetCurrentProcess(), (DWORD64) frames[i], &displacement, symbol64);
                if (result)
                {
                    IMAGEHLP_LINE64 line64;
                    DWORD displacement = 0;
                    memset(&line64, 0, sizeof(IMAGEHLP_LINE64));
                    line64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                    result = SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64) frames[i], &displacement, &line64);
                    if (result)
                    {
                        std::string line = string(symbol64->Name) + " (" + line64.FileName + ", line " + string(line64.LineNumber) + ")";
                        backtrace_text.push_back( line );
                    }
                    else
                    {
                        backtrace_text.push_back(symbol64->Name);
                    }
                }
            }

            SymCleanup(GetCurrentProcess());
            return mutex->unlock(), backtrace_text;
        )

        $gcc(
            char **_strings;
            _strings = backtrace_symbols(frames, num_frames);
            if (!_strings)
            {
                return strings();
            }

            strings backtrace_text;

            for( int cnt = 0; cnt < num_frames; cnt++ )
            {
                // Decode the strings
                char *ptr = _strings[cnt];
                char *filename = ptr;
                const char *function = "";

                // Find function name
                while(*ptr)
                {
                    if (*ptr=='(')  // Found function name
                    {
                        *(ptr++) = 0;
                        function = ptr;
                        break;
                    }
                    ptr++;
                }

                // Find offset
                if (function[0])        // Only if function was found
                {
                    while(*ptr)
                    {
                        if (*ptr=='+')  // Found function offset
                        {
                            *(ptr++) = 0;
                            break;
                        }
                        if (*ptr==')')  // Not found function offset, but found, end of function
                        {
                            *(ptr++) = 0;
                            break;
                        }
                        ptr++;
                    }
                }

                int status;
                char *new_function = abi::__cxa_demangle(function, 0, 0, &status);
                if (new_function)       // Was correctly decoded
                    function = new_function;

                backtrace_text.push_back( string(function) + " (" + string(filename) + ")" );

                if (new_function)
                    free(new_function);
            }

            free (_strings);
            return backtrace_text;
        )
        $apple(
        )
        $other(
            return strings();
        )
    }
}

namespace
{
    bool is_callstack_available = false;
}

callstack::callstack() // save
{
    // When calling callstack().str() from a secondary thread will retrieve empty stack until a callstack is retrieved from main thread.
    // This happens on my machine (Win7, 64bit, msvc 2011) under some circumstances. So I've added '100 tries' instead of 'try once'.
    // @Todo: examine & fix issue

    static size_t tries = 0;
    if( (!is_callstack_available) && tries < 100 )
    {
        ++tries;

        num_frames = capture_stack_trace( 0, max_frames, frames );
        for( int i = num_frames; i < max_frames; ++i ) frames[ i ] = 0;

        strings stack_trace = get_stack_trace( frames, num_frames );
        string out;

        for( size_t i = 0; i < stack_trace.size(); i++ )
            out += stack_trace[i] + '|';

        is_callstack_available = ( out.count("callstack") > 0 );
    }

    if( !is_callstack_available )
        return;

    num_frames = capture_stack_trace( 1, max_frames, frames );
    for( int i = num_frames; i < max_frames; ++i ) frames[ i ] = 0;
}

std::string callstack::str( const char *format12, size_t skip_initial )
{
    string out;

    if( !is_callstack_available )
        return out;

    strings stack_trace = get_stack_trace( frames, num_frames );

    for( size_t i = skip_initial; i < stack_trace.size(); i++ )
        out += string( format12, (int)i, stack_trace[i] );

    return out;
}

std::string stacktrace( const char *format12, size_t skip_initial ) {
    return callstack().str( format12, skip_initial );
}

// DIE

void die( const std::string &reason, int errorcode )
{
    if( reason.size() )
    {
        std::string append = stacktrace();

        if( append.size() )
        {
            fprintf( stderr, "%s\n%s", reason.c_str(), append.c_str() );
            errorbox( reason + '\n' + append, "Error" );
        }
        else
        {
            fprintf( stderr, "%s", reason.c_str() );
            errorbox( reason, "Error" );
        }
    }

    $win32(
    FatalExit( errorcode );
    )

    // standard ; thread friendly?
    std::exit( errorcode );
}

void die( int errorcode, const std::string &reason )
{
    die( reason, errorcode );
}

// HEXDUMP
// @todo: maxwidth != 80 doesnt work

std::string hexdump( const void *data, size_t num_bytes, const void *self )
{
#   ifdef _MSC_VER
#       pragma warning( push )
#       pragma warning( disable : 4996 )
#       define $vsnprintf _vsnprintf
#   else
#       define $vsnprintf  vsnprintf
#   endif

    struct local {
        static std::string format( const char *fmt, ... )
        {
            int len;
            std::string self;

            using namespace std;

            // Calculate the final length of the formatted string
            {
                va_list args;
                va_start( args, fmt );
                len = $vsnprintf( 0, 0, fmt, args );
                va_end( args );
            }

            // Allocate a buffer (including room for null termination
            char* target_string = new char[++len];

            // Generate the formatted string
            {
                va_list args;
                va_start( args, fmt );
                $vsnprintf( target_string, len, fmt, args );
                va_end( args );
            }

            // Assign the formatted string
            self.assign( target_string );

            // Clean up
            delete [] target_string;

            return self;
        }
    };

	using std::max;
    unsigned maxwidth = 80;
    unsigned width = 16; //column width
    unsigned width_offset_block = (8 + 1);
    unsigned width_chars_block  = (width * 3 + 1) + sizeof("asc");
    unsigned width_hex_block    = (width * 3 + 1) + sizeof("hex");
    unsigned width_padding = max( 0, int( maxwidth - ( width_offset_block + width_chars_block + width_hex_block ) ) );
	unsigned blocks = width_padding / ( width_chars_block + width_hex_block ) ;

    unsigned dumpsize = ( num_bytes < width * 16 ? num_bytes : width * 16 ); //16 lines max

    std::string result;

    result += local::format( "%-*s %-.*s [ptr=%p sz=%d]\n", width_offset_block - 1, "offset", width_chars_block - 1, "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", self ? self : data, num_bytes );

    if( !num_bytes )
        return result;

    blocks++;

    const unsigned char *p = reinterpret_cast<const unsigned char *>( data );
    size_t i = 0;

    while( i < dumpsize )
    {
        //offset block
        result += local::format("%p ", (size_t)(p + i) ); //%08x, %08zx

        //chars blocks
        for( unsigned b = 0; b < blocks; b++)
        {
            for( unsigned c = 0 ; c < width ; c++ )
                result += local::format(" %c ", i + c >= dumpsize ? '.' : p[i + c] < 32 || p[i + c] >= 127 ? '.' : p[i + c]);

            result += "asc\n";
        }

        //offset block
        result += local::format("%p ", (size_t)(p + i) ); //%08x, %08zx

        //hex blocks
        for( unsigned b = 0; b < blocks; b++)
        {
            for( unsigned c = 0; c < width ; c++)
                result += local::format( i + c < dumpsize ? "%02x " : "?? ", p[i + c]);

            result += "hex\n";
        }

        //next line
        //result += '\n';
        i += width * blocks;
    }

    return result;

#   undef $vsnprintf
#   ifdef _MSC_VER
#       pragma warning( pop )
#   endif
}





/*
 * Simple prompt dialog (based on legolas558's code). MIT license.
 * - rlyeh
 */

#ifndef _WIN32

std::string prompt( const std::string &title, const std::string &current_value, const std::string &caption )
{
    std::string out;

    if( has("whiptail") && false )
    {
        std::string out = pipe( std::string() +
            "/usr/bin/whiptail 3>&1 1>&2 2>&3 --title \"" +title+ "\" --inputbox \"" +caption+ "\" 0 0 \"" +current_value+ "\"" );
    }
    else
    {
        if( title.size() && caption.size() )
            fprintf(stdout, "%s", title.c_str());
        else
            fprintf(stdout, "%s", title.empty() ? caption.c_str() : title.c_str());

        if( !current_value.empty() )
            fprintf(stdout, " (enter defaults to '%s')", current_value.c_str());

        fprintf(stdout, "%s", "\n");

        std::getline( std::cin, out );

        if( out.empty() )
            out = current_value;
    }

    return out;
}
#endif



#ifdef _MSC_VER
#   pragma warning( push )
#   pragma warning( disable : 4996 )
#endif

#ifdef _WIN32

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

#pragma message("<heal/heal.cpp> says: dialog aware dpi fix (@todo)")

std::string prompt( const std::string &title, const std::string &current_value, const std::string &caption )
{
    class InputBox
    {
        private:

        HWND                hwndParent,
                            hwndInputBox,
                            hwndQuery,
                            hwndOk,
                            hwndCancel,
                            hwndEditBox;
        LPSTR               szInputText;
        WORD                wInputMaxLength, wInputLength;
        bool                bRegistered,
                            bResult;

        HINSTANCE           hThisInstance;

        enum
        {
            CIB_SPAN = 10,
            CIB_LEFT_OFFSET = 6,
            CIB_TOP_OFFSET = 4,
            CIB_WIDTH = 300,
            CIB_HEIGHT = 130,
            CIB_BTN_WIDTH = 60,
            CIB_BTN_HEIGHT = 20
        };

        public:

#       define CIB_CLASS_NAME "CInputBoxA"

        static LRESULT CALLBACK CIB_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            InputBox *self;
            self = (InputBox *)GetWindowLong(hWnd, GWL_USERDATA);

            switch (msg)
            {
                case WM_CREATE:
                    self = (InputBox *) ((CREATESTRUCT *)lParam)->lpCreateParams;
                    SetWindowLong(hWnd, GWL_USERDATA, (long)self);
                    self->create(hWnd);
                break;
                case WM_COMMAND:
                    switch(LOWORD(wParam)) {
                        case IDOK:
                            self->submit();
                        case IDCANCEL:
                            self->close();
                        break;
                    }
                    break;
                case WM_CLOSE:
                    self->hide();
                    return 0;
                case WM_DESTROY:
                    self->destroy();
                    break;
            }
            return(DefWindowProc (hWnd, msg, wParam, lParam));
        }

        InputBox( HINSTANCE hInst ) :
            hwndParent(0),
            hwndInputBox(0),
            hwndQuery(0),
            hwndOk(0),
            hwndCancel(0),
            hwndEditBox(0),
            szInputText(""),
            wInputMaxLength(0), wInputLength(0),
            bRegistered(false),
            bResult(false),
            hThisInstance(0)
        {
            WNDCLASSEX wndInputBox;
            RECT rect;

            memset(&wndInputBox, 0, sizeof(WNDCLASSEX));

            hThisInstance = hInst;

            wndInputBox.cbSize                  = sizeof(wndInputBox);
            wndInputBox.lpszClassName           = CIB_CLASS_NAME;
            wndInputBox.style                   = CS_HREDRAW | CS_VREDRAW;
            wndInputBox.lpfnWndProc             = CIB_WndProc;
            wndInputBox.lpszMenuName            = NULL;
            wndInputBox.hIconSm                 = NULL;
            wndInputBox.cbClsExtra              = 0;
            wndInputBox.cbWndExtra              = 0;
            wndInputBox.hInstance               = hInst;
            wndInputBox.hIcon                   = LoadIcon(NULL, IDI_WINLOGO);
            wndInputBox.hCursor                 = LoadCursor(NULL, IDC_ARROW);
            wndInputBox.hbrBackground           = (HBRUSH)(COLOR_WINDOW);

            RegisterClassEx(&wndInputBox);

            if (hwndParent)
                GetWindowRect(hwndParent, &rect); //always false?
            else
                GetWindowRect(GetDesktopWindow(), &rect);

            hwndInputBox = CreateWindow(    CIB_CLASS_NAME, "",
                            (WS_BORDER | WS_CAPTION), rect.left+(rect.right-rect.left-CIB_WIDTH)/2,
                            rect.top+(rect.bottom-rect.top-CIB_HEIGHT)/2,
                            CIB_WIDTH, CIB_HEIGHT, hwndParent, NULL,
                            hThisInstance, this);
        }

        void destroy()
        {
            EnableWindow(hwndParent, true);
            SendMessage(hwndInputBox, WM_CLOSE/*WM_DESTROY*/, 0, 0);
        }

        ~InputBox()
        {
            UnregisterClass(CIB_CLASS_NAME, hThisInstance);
        }

        void submit()
        {
            wInputLength = (int)SendMessage(hwndEditBox, EM_LINELENGTH, 0, 0);
            if (wInputLength) {
                *((LPWORD)szInputText) = wInputMaxLength;
                wInputLength = (WORD)SendMessage(hwndEditBox, EM_GETLINE, 0, (LPARAM)szInputText);
            }
            szInputText[wInputLength] = '\0';
            bResult = true;
        }

        void create(HWND hwndNew)
        {
            static HFONT myFont = NULL;

            if( myFont != NULL )
            {
                DeleteObject( myFont );
                myFont = NULL;
            }

            hwndInputBox = hwndNew;

            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);

            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
            {
#if 0
                LOGFONT lf;
                memset(&lf,0,sizeof(LOGFONT));

                lf.lfWeight= FW_NORMAL;
                lf.lfCharSet= ANSI_CHARSET;
                //lf.lfPitchAndFamily = 35;
                lf.lfHeight= 10;
                strcpy(lf.lfFaceName, "Tahoma");
                myFont=CreateFontIndirect(&lf);
#else
                myFont = CreateFontIndirect(&ncm.lfMessageFont);
#endif
            }
            else
            {
                myFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            }

            //  SetWindowPos(hwndInputBox, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            hwndQuery = CreateWindow("Static", "", WS_CHILD | WS_VISIBLE,
                                    CIB_LEFT_OFFSET, CIB_TOP_OFFSET,
                                    CIB_WIDTH-CIB_LEFT_OFFSET*2, CIB_BTN_HEIGHT*2,
                                    hwndInputBox, NULL,
                                    hThisInstance, NULL);
            hwndEditBox = CreateWindow("Edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, CIB_LEFT_OFFSET,
                                    CIB_TOP_OFFSET + CIB_BTN_HEIGHT*2, CIB_WIDTH-CIB_LEFT_OFFSET*3, CIB_BTN_HEIGHT,
                                    hwndInputBox,   NULL,
                                    hThisInstance, NULL);
            hwndOk =        CreateWindow("Button", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                    CIB_WIDTH/2 - CIB_SPAN*2 - CIB_BTN_WIDTH, CIB_HEIGHT - CIB_TOP_OFFSET*4 - CIB_BTN_HEIGHT*2,
                                    CIB_BTN_WIDTH, CIB_BTN_HEIGHT, hwndInputBox, (HMENU)IDOK,
                                    hThisInstance, NULL);
            hwndCancel =    CreateWindow("Button", "Cancel",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                    CIB_WIDTH/2 + CIB_SPAN, CIB_HEIGHT - CIB_TOP_OFFSET*4 - CIB_BTN_HEIGHT*2,  CIB_BTN_WIDTH, CIB_BTN_HEIGHT,
                                    hwndInputBox, (HMENU)IDCANCEL,
                                    hThisInstance, NULL);

        //  SendMessage(hwndInputBox,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndQuery,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndEditBox,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndOk,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndCancel,WM_SETFONT,(WPARAM)myFont,FALSE);
        }

        void close()
        {
            PostMessage(hwndInputBox, WM_CLOSE, 0, 0);
        }

        void hide()
        {
            ShowWindow(hwndInputBox, SW_HIDE);
        }

        void show(LPCSTR lpszTitle, LPCSTR  lpszQuery)
        {
            SetWindowText(hwndInputBox, lpszTitle);
            SetWindowText(hwndEditBox, szInputText);
            SetWindowText(hwndQuery, lpszQuery);
            SendMessage(hwndEditBox, EM_LIMITTEXT, wInputMaxLength, 0);
            SendMessage(hwndEditBox, EM_SETSEL, 0, -1);
            SetFocus(hwndEditBox);
            ShowWindow(hwndInputBox, SW_NORMAL);
        }

        int show(HWND hwndParentWindow, LPCTSTR lpszTitle, LPCTSTR lpszQuery, LPSTR szResult, WORD wMax)
        {
            MSG msg;
            BOOL    bRet;
            hwndParent = hwndParentWindow;
            szInputText = szResult;
            wInputMaxLength = wMax;

            bResult = false;

        //  EnableWindow(hwndParent, false);

            show(lpszTitle, lpszQuery);

            while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
            {
                if (msg.message==WM_KEYDOWN) {
                    switch (msg.wParam) {
                    case VK_RETURN:
                        submit();
                    case VK_ESCAPE:
                        close();
                        break;
                    default:
                        TranslateMessage(&msg);
                        break;
                    }
                } else
        //      if (!IsDialogMessage(hwndInputBox, &msg)) {
                    TranslateMessage(&msg);
        //      }
                DispatchMessage(&msg);
                if (msg.message == WM_CLOSE)
                    break;
            }

        //  EnableWindow(hwndParent, true);

            return bResult;
        }

    #   undef CIB_CLASS_NAME
    }
    myinp(GetModuleHandle(0));

    char *result = new char [2048+1];

    memset( result, 0, 2048+1 ); //default value

    strcpy( result, current_value.c_str() );

    myinp.show(0, title.c_str(), caption.size() ? caption.c_str() : title.c_str(), result, 2048);

    std::string _r = result;

    delete [] result;

    return _r;
}

#endif

#ifdef _MSC_VER
#   pragma warning( pop )
#endif

#undef $debug
#undef $release
#undef $other
#undef $gcc
#undef $msvc

#undef $undefined
#undef $apple
#undef $linux
#undef $win32

#undef $no
#undef $yes

