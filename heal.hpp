// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

/* Smart assert replacement for LHS/RHS values
 * Copyright (c) 2014 Mario 'rlyeh' Rodriguez

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

 * - rlyeh ~~ listening to Tuber / Desert Overcrowded
 */

/* Private API */

#ifndef ASSERTPP_HEADER
#define ASSERTPP_HEADER

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <deque>
#include <string>
#include <sstream>

namespace assertpp {
    class check {
        int ok;
        std::deque< std::string > xpr;
        template<typename T> static std::string to_str( const T &t ) { std::stringstream ss; return ss << t ? ss.str() : "??"; }
    public:
        check( const char *const text, const char *const file, int line, bool result )
        :   xpr( {std::string(file) + ':' + to_str(line), "", text, "" } ), ok(result)
        {}
        ~check() {
            if( xpr.empty() ) return;
            operator bool();
            xpr[0] = xpr[0] + xpr[1];
            xpr.erase( xpr.begin() + 1 );
            if( !ok ) {
                xpr[2] = xpr[2].substr( xpr[2][2] == ' ' ? 3 : 4 );
                xpr[1].resize( (xpr[1] != xpr[2]) * xpr[1].size() );
                std::string buf;
                buf = "<heal/heal.hpp> says: expression failed! (" + xpr[1] + ") -> (" + xpr[2] + ") -> (unexpected) at " + xpr[0] + "\n";
                fprintf(stderr, "%s", buf.c_str() );
                // assert fallback here
                fclose( stderr );
                fclose( stdout );
                assert( !"<heal/heal.hpp> says: expression failed!" );
                // user defined fallbacks here
                for(;;) {}
            };
        }
#       define assert$impl(OP) \
        template<typename T> check &operator OP( const T &rhs         ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; } \
        template<unsigned N> check &operator OP( const char (&rhs)[N] ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; }
        operator bool() {
            return xpr.size() >= 3 && xpr[3].size() >= 6 && [&]() -> bool {
                char sign = xpr[3].at(xpr[3].size()/2+1);
                bool equal = xpr[3].substr( 4 + xpr[3].size()/2 ) == xpr[3].substr( 3, xpr[3].size()/2 - 3 );
                return ok = ( sign == '=' ? equal : ( sign == '!' ? !equal : ok ) );
            }(), ok;
        }
        assert$impl( <); assert$impl(<=); assert$impl( >); assert$impl(>=); assert$impl(!=); assert$impl(==); assert$impl(^=);
        assert$impl(&&); assert$impl(&=); assert$impl(& ); assert$impl(||); assert$impl(|=); assert$impl(| ); assert$impl(^ );
#       undef assert$impl
    };
}

#endif

/* Public API */

#include <cassert>

#if !(defined(NDEBUG) || defined(_NDEBUG))
#undef  assert
#define assert(...) ( bool(__VA_ARGS__) ? \
        ( assertpp::check(#__VA_ARGS__,__FILE__,__LINE__,1) < __VA_ARGS__ ) : \
        ( assertpp::check(#__VA_ARGS__,__FILE__,__LINE__,0) < __VA_ARGS__ ) )
#endif

// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

#ifndef __HEALHPP__
#define __HEALHPP__

#include <stdio.h>

#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// OS utils. Here is where the fun starts... good luck

#define $quote(...)     #__VA_ARGS__
#define $comment(...)   $no
#define $uncomment(...) $yes

#define $yes(...)     __VA_ARGS__
#define $no(...)

#if defined(_WIN32)
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
// usage: $warning("this is shown at compile time")
#define $heal$todo$stringize$impl(X) #X
#define $heal$todo$stringize(X) $heal$todo$stringize$impl(X)
#if $on($msvc)
#   define $heal$todo$message(msg) __FILE__ "(" $heal$todo$stringize(__LINE__)") : $warning - " msg " - [ "__FUNCTION__ " ]"
#   define $warning(msg) __pragma( message( $heal$todo$message(msg) ) )
#elif $on($gnuc) || $on($clang)
#   define $heal$todo$message(msg) __FILE__ "(" $heal$todo$stringize(__LINE__)") : $warning - " msg " - [ "__func__ " ]"
#   define $heal$todo$message$impl(msg) _Pragma(#msg)
#   define $warning(msg) $heal$todo$message$impl( message( $heal$todo$message(msg) ) )
#else
#   define $warning(msg)
#endif

/* public API */

namespace heal {

typedef std::function< int( const std::string &in ) > heal_callback_in;
typedef std::function< int( std::ostream &headers, std::ostream &content, const std::string &in ) > heal_http_callback;

extern std::vector< heal_callback_in > warns;
extern std::vector< heal_callback_in > fails;

void warn( const std::string &error );
void fail( const std::string &error );

void add_worker( heal_callback_in fn );
void add_webmain( int port, heal_http_callback fn );

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

struct callstack /* : public std::vector<const void*> */ {
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

}

#endif
