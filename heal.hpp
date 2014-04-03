#ifndef __HEALHPP__
#define __HEALHPP__

#include <stdio.h>

#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
	

// OS utils. Here is where the fun starts... good luck

#define $quote(...)   #__VA_ARGS__

#define $yes(...)     __VA_ARGS__
#define $no(...)

#if defined(_WIN32) || defined(_WIN64)
#   define $windows   $yes
#   define $welse     $no
#else
#   define $windows   $no
#   define $welse     $yes
#endif

#ifdef __APPLE__
#   define $apple     $yes
#   define $aelse     $no
#else
#   define $apple     $no
#   define $aelse     $yes
#endif

#ifdef __linux__
#   define $linux     $yes
#   define $lelse     $no
#else
#   define $linux     $no
#   define $lelse     $yes
#endif

// Compiler utils

#if defined(NDEBUG) || defined(_NDEBUG)
#   define $release   $yes
#   define $debug     $no
#else
#   define $release   $no
#   define $debug     $yes
#endif

#ifdef __GNUC__
#   define $gnuc      $yes
#   define $gelse     $no
#else
#   define $gnuc      $no
#   define $gelse     $yes
#endif

#ifdef _MSC_VER
#   define $msvc      $yes
#   define $melse     $no
#else
#   define $msvc      $no
#   define $melse     $yes
#endif

#ifdef __clang__
#   define $clang     $yes
#   define $celse     $no
#else
#   define $clang     $no
#   define $celse     $yes
#endif

#define $on(v)        (0 v(+1))  // usage: #if $on($msvc)
#define $is           $on        // usage: #if $is($debug)
#define $has(...)     $clang(__has_feature(__VA_ARGS__)) $celse(__VA_ARGS__) // usage: #if $has(cxx_exceptions)

#if $on($msvc) || $on($gnuc) || $on($clang)
#define $undefined_compiler $no
#else
#define $undefined_compiler $yes
#endif

#if $on($windows) || $on($linux) || $on($apple)
#define $undefined_os $no
#else
#define $undefined_os $yes
#endif

// try to detect if exceptions are enabled...

#if (defined(_HAS_EXCEPTIONS) && (_HAS_EXCEPTIONS > 0)) || \
    (defined(_STLP_USE_EXCEPTIONS) && (_STLP_USE_EXCEPTIONS > 0)) || \
    (defined(HAVE_EXCEPTIONS)) || \
    (defined(__EXCEPTIONS)) || \
    (defined(_CPPUNWIND)) || \
    ($has(cxx_exceptions)) /*(__has_feature(cxx_exceptions))*/
#   define $throw     $yes
#   define $telse     $no
#else
#   define $throw     $no
#   define $telse     $yes
#endif

// create a $warning(...) macro

/*
#if $on($msvc)
#   define $warning(msg) __pragma( message( msg ) )
#elif defined(__GNUC__) && defined(GCC_VERSION) && GCC_VERSION >= 40400
#   define $warning$message$impl(msg) _Pragma(#msg)
#   define $warning(msg) $warning$message$impl( message( msg ) )
#else
#   define $warning(msg)
#endif
*/

// usage: $warning("this is shown at compile time")
#define $heal$todo$stringize$impl(X) #X
#define $heal$todo$stringize(X) $heal$todo$stringize$impl(X)
#if defined(_MSC_VER)
#   define $heal$todo$message(msg) __FILE__ "(" $heal$todo$stringize(__LINE__)") : $warning - " msg " - [ "__FUNCTION__ " ]"
#   define $warning(msg) __pragma( message( $heal$todo$message(msg) ) )
#elif defined(__GNUC__) && defined(GCC_VERSION) && GCC_VERSION >= 40400
#   define $heal$todo$message(msg) __FILE__ "(" $heal$todo$stringize(__LINE__)") : $warning - " msg " - [ "__func__ " ]"
#   define $heal$todo$message$impl(msg) _Pragma(#msg)
#   define $warning(msg) $heal$todo$message$impl( message( $heal$todo$message(msg) ) )
#else
#   define $warning(msg)
#endif

/* public API */

typedef std::function< int( const std::string &in ) > heal_callback_in;
typedef std::function< int( std::ostream &os, const std::string &in ) > heal_callback_inout;

extern std::vector< heal_callback_in > warns;
extern std::vector< heal_callback_in > fails;

void warn( const std::string &error );
void fail( const std::string &error );

void add_worker( heal_callback_in fn );
void add_webmain( int port, heal_callback_inout fn );

void die( const std::string &reason, int errorcode = -1 );
void die( int errorcode = -1, const std::string &reason = std::string() );

void breakpoint();
bool debugger( const std::string &reason = std::string() );

void alert();
void alert( const          char *t, const std::string &title = std::string() );
void alert( const   std::string &t, const std::string &title = std::string() );
void alert( const std::istream &is, const std::string &title = std::string() );
void alert( const        size_t &t, const std::string &title = std::string() );
void alert( const        double &t, const std::string &title = std::string() );
void alert( const         float &t, const std::string &title = std::string() );
void alert( const           int &t, const std::string &title = std::string() );
void alert( const          char &t, const std::string &title = std::string() );
void alert( const          bool &t, const std::string &title = std::string() );
void errorbox( const std::string &body = std::string(), const std::string &title = std::string() );
std::string prompt( const std::string &title = std::string(), const std::string &current_value = std::string(), const std::string &caption = std::string() );

bool is_debug();
bool is_release();
bool is_asserting();

size_t get_mem_peak(); // peak of resident set size (physical memory use) measured in bytes, or zero
size_t get_mem_current(); // curent set size (physical memory use) measured in bytes, or zero
size_t get_mem_size(); // size of physical memory (RAM) in bytes
double get_time_cpu(); // amount of CPU time used by the current process, in seconds, or -1.0
double get_time_clock(); // real time, in seconds, or -1.0
std::string get_mem_peak_str();    // peak of resident set size (physical memory use) measured in bytes, or zero
std::string get_mem_current_str(); // curent set size (physical memory use) measured in bytes, or zero
std::string get_mem_size_str();    // size of physical memory (RAM) in bytes
std::string get_time_cpu_str();    // amount of CPU time used by the current process, in seconds, or -1.0
std::string get_time_clock_str();  // real time, in seconds, or -1.0

struct benchmark : public std::string {
    double mem;
    double time;
    bool stopped;

    explicit
    benchmark( const char *text ) : std::string(text), mem(0), time(0), stopped(1)
    {}

    void start() {
        stopped = false;
        do { mem = get_mem_current(); } while( !mem );
        do { time = get_time_clock(); } while( time < 0 );
    }

    void stop() {
        //if( !stopped )
        {
            stopped = true;
            double time; do { time = get_time_clock(); } while ( time < 0 );
            double mem;  do { mem = get_mem_current(); } while ( !mem );
            this->mem = mem - this->mem;
            this->time = time - this->time;
        }
    }

    void cancel() {
        mem = time = 0;
        stopped = true;
    }

    std::string &name() {
        return *this;
    }
    const std::string &name() const {
        return *this;
    }

    template<class T>
    T &print( T &os ) const {
        os << name() << " = " << (mem/1024.0) << "Kb, " << (unsigned(time*100)/100.0) << "s" << std::endl;
        return os;
    }
};

template< std::ostream &OSS = std::cout >
struct scoped_benchmark : public benchmark {
    explicit
    scoped_benchmark( const char *text ) : benchmark(text) {
        this->start();
    }
    
    ~scoped_benchmark() {
        this->stop();
        this->print( OSS );
    }
};


    //std::vector<const void*> callstack();

    struct callstack {
        enum { max_frames = 128 };
        std::vector<void *> frames;
        callstack( bool autosave = false );
        size_t space() const;
        void save( unsigned frames_to_skip = 0 );
        std::vector<std::string> unwind( unsigned from = 0, unsigned to = ~0 ) const;
        std::vector<std::string> str( const char *format12 = "#\1 \2\n", size_t skip_begin = 0 );
    };

    template<typename T>
    static inline
    std::string lookup( T *ptr ) {
        callstack cs;
        cs.frames.push_back( (void *)ptr );
        std::vector<std::string> stacktrace = cs.unwind();
        return stacktrace.size() ? stacktrace[0] : std::string("????");
    }

std::string demangle( const std::string &mangled );
std::vector<std::string> stacktrace( const char *format12 = "#\1 \2\n", size_t skip_initial = 0 );
std::string stackstring( const char *format12 = "#\1 \2\n", size_t skip_initial = 0 );


std::string hexdump( const void *data, size_t num_bytes, const void *self = 0 );

template<typename T> inline std::string hexdump( const T& obj ) {
    return hexdump( obj.data(), obj.size() * sizeof(*obj.begin()), &obj );
}
template<> inline std::string hexdump( const std::nullptr_t &obj ) {
    return hexdump( 0,0,0 );
}
template<> inline std::string hexdump( const char &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const short &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const long &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const long long &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const unsigned char &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const unsigned short &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const unsigned long &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const unsigned long long &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const float &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const double &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}
template<> inline std::string hexdump( const long double &obj ) {
    return hexdump( &obj, sizeof(obj), &obj );
}/*
template<size_t N> inline std::string hexdump( const char (&obj)[N] ) {
    return hexdump( &obj, sizeof(char) * N, &obj );
}*/

template<typename T> inline std::string hexdump( const T* obj ) {
    if( !obj ) return hexdump(0,0,0);
    return hexdump( *obj );
}
template<> inline std::string hexdump( const char *obj ) {
    if( !obj ) return hexdump(0,0,0);
    return hexdump( std::string(obj) );
}

std::string timestamp();

bool is_root();
std::string get_app_path();
bool add_firewall_rule( const std::string &name, bool on, const std::string &binpath );
bool add_firewall_rule( const std::string &name, bool on, bool tcp, int port );
std::string get_pipe( const std::string &cmd, int *retcode = 0 );

#endif

#ifdef assert1
#   undef assert1
#endif

#ifdef assert2
#   undef assert2
#endif

#ifdef assert3
#   undef assert3
#endif

#ifdef assert4
#   undef assert4
#endif

#if defined(NDEBUG) || defined(_NDEBUG)
#   define assert1(A)
#   define assert2(A,...)
#   define assert3(A,op,B)
#   define assert4(A,op,B,...)
#else
#   define assert1(A)             assertX( true, A, !=, 00,           0 )
#   define assert2(A,...)         assertX( true, A, !=, 00, __VA_ARGS__ )
#   define assert3(A,OP,B)        assertX( true, A, OP,  B,           0 )
#   define assert4(A,OP,B,...)    assertX( true, A, OP,  B, __VA_ARGS__ )
#   define assertX(SIGN,A,OP,B,...) [&]() -> bool { \
        std::stringstream _SS_, _TT_; \
        auto _A_ = (A); auto _B_ = decltype(_A_)(B); auto _OK_ = ( _A_ OP _B_ ); if( !SIGN ) _OK_ = !_OK_; \
        _SS_ << ( _OK_ ? "[ OK ]" : "[FAIL]" ) << "Assert "; \
        _SS_ << " at " __FILE__ ":" << __LINE__; if( !_OK_ ) { std::string _EXPR_; \
         if( std::string("0") != #__VA_ARGS__ ) _SS_ << " - ", _SS_ << __VA_ARGS__; \
         _EXPR_  = "\n\t" #A;      _TT_ << "\n\t" << _A_; if( std::string("00") != #B ) {\
         _EXPR_ += " " #OP " " #B; _TT_ << " " #OP " " << _B_; } \
         _SS_ << ( _TT_.str() != _EXPR_ ? _EXPR_ : "" ) << _TT_.str(); \
         _SS_ << "\n\t" << ( _OK_ ^ (!SIGN) ? "true (false expected)" : "false (true expected)" ); \
         fail( _SS_.str() ); } \
        return _OK_ ? true : false; }()
#endif
