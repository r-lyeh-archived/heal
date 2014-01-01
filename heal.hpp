#ifndef __HEALHPP__
#define __HEALHPP__

#include <iostream>
#include <sstream>
#include <string>

/* public API */

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

class callstack
{
    public:
    /* save */
    callstack();
    /* print */
    std::string str( const char *format12 = "#\1 \2\n", size_t skip_initial = 0 );

    private:
    size_t num_frames;
    enum { max_frames = 32 };
    void *frames[ max_frames ];
};

std::string demangle( const std::string &name );
std::string stacktrace( const char *format12 = "#\1 \2\n", size_t skip_initial = 0 );
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

void setup_assert( void (*custom_assert_cb)( const std::string &assertion, const char *at_file, int at_line ) );

/* private API */
namespace cb
{
    extern void (*custom_assert_cb)( const std::string &assertion, const char *at_file, int at_line );
}

#endif

#include <cassert>

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
#   define assert2(A,description)
#   define assert3(A,op,B)
#   define assert4(A,op,B,description)
#else
#   define assert1(A)                       assert2(A,"")
#   define assert2(A,description) do { \
        auto _A_ = (A); if( _A_ == decltype(_A_)(0) && cb::custom_assert_cb ) { \
            std::stringstream ss; ss \
                << "Assertion failed at " __FILE__ ":" << __LINE__ \
                << "\n\t" #A \
                << "\n\t" << _A_ \
                << "\n\t(false)" \
                << "\n\t" << description; \
            (*cb::custom_assert_cb)( ss.str(), __FILE__, __LINE__ ); \
    } } while(0)
#   define assert3(A,op,B)                  assert4(A,op,B,"")
#   define assert4(A,op,B,description) do { \
        auto _A_ = (A); auto _B_ = (B); if( (!(_A_ op _B_)) && cb::custom_assert_cb ) { \
            std::stringstream ss; ss \
                << "Assertion failed at " __FILE__ ":" << __LINE__ \
                << "\n\t" #A " " #op " " #B \
                << "\n\t" << _A_ << ' ' << #op << ' ' << _B_ \
                << "\n\t(false)" \
                << "\n\t" << description; \
            (*cb::custom_assert_cb)( ss.str(), __FILE__, __LINE__ ); \
    } } while(0)
#endif
