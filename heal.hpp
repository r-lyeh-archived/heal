// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

// std 11 <-> 03/w-boost bridge compatiblity layer, plus a few macro utils.
// - rlyeh, zlib/libpng licensed.

#ifndef __BRIDGE_HPP__
#define __BRIDGE_HPP__

#ifdef __SSE__
#   define BOOST_HAS_INT128 1
#   include <xmmintrin.h>
#endif

#if (__cplusplus < 201103L && !defined(_MSC_VER)) || (defined(_MSC_VER) && (_MSC_VER < 1700)) || (defined(__GLIBCXX__) && __GLIBCXX__ < 20130322L)
#   define BRIDGE_VERSION 2003
#   include <boost/functional.hpp> // if old libstdc++ or msc libs are found, use boost::function
#   include <boost/function.hpp>   //
#   include <boost/thread.hpp>     // and boost::thread
#   include <boost/cstdint.hpp>
#   include <boost/type_traits.hpp>
#   include <boost/bind.hpp>
#   include <boost/bind/placeholders.hpp>
namespace std {
    namespace placeholders {
        //...
    }
    using namespace boost;
}
#else
#   define BRIDGE_VERSION 2011
#   include <functional>       // else assume modern c++11 and use std::function<> instead
#   include <mutex>            // and std::mutex
#   include <thread>           // and std::thread
#   include <cstdint>
#endif

#if BRIDGE_VERSION >= 2011
#define $cpp11          $yes
#define $cpp03          $no
#else
#define $cpp11          $no
#define $cpp03          $yes
#endif

// Thread Local Storage

#if defined(__MINGW32__) || defined(__SUNPRO_C) || defined(__xlc__) || defined(__GNUC__) || defined(__clang__) || defined(__GNUC__) // __INTEL_COMPILER on linux
//   MingW, Solaris Studio C/C++, IBM XL C/C++,[3] GNU C,[4] Clang[5] and Intel C++ Compiler (Linux systems)
#    define $tls(x) __thread x
#else
//   Visual C++,[7] Intel C/C++ (Windows systems),[8] C++Builder, and Digital Mars C++
#    define $tls(x) __declspec(thread) x
#endif

// OS utils. Here is where the fun starts... good luck

#define $quote(...)     #__VA_ARGS__
#define $comment(...)   $no
#define $uncomment(...) $yes

#define $yes(...)     __VA_ARGS__
#define $no(...)

#define $on(v)        (0 v(+1))  // usage: #if $on($msvc)
#define $is           $on        // usage: #if $is($debug)
#define $has(...)     $clang(__has_feature(__VA_ARGS__)) $celse(__VA_ARGS__) // usage: #if $has(cxx_exceptions)

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

#if ULONG_MAX == 4294967295
#   define $bits64    $yes
#   define $bits32    $no
#else
#   define $bits64    $no
#   define $bits32    $yes
#endif

#if defined(NDEBUG) || defined(_NDEBUG) || defined(RELEASE)
#   define $release   $yes
#   define $debug     $no
#else
#   define $release   $no
#   define $debug     $yes
#endif

#if defined(NDEVEL) || defined(_NDEVEL) || defined(PUBLIC)
#   define $public    $yes
#   define $devel     $no
#else
#   define $public    $no
#   define $devel     $yes
#endif

#if defined(__GNUC__) || defined(__MINGW32__)
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

#if $on($msvc) || $on($gnuc) || $on($clang)
#   define $undefined_compiler $no
#else
#   define $undefined_compiler $yes
#endif

#if $on($windows) || $on($linux) || $on($apple)
#   define $undefined_os $no
#else
#   define $undefined_os $yes
#endif

#if $on($gnuc) || $on($clang)
#   define $likely(expr)    (__builtin_expect(!!(expr), 1))
#   define $unlikely(expr)  (__builtin_expect(!!(expr), 0))
#else
#   define $likely(expr)    ((expr))
#   define $unlikely(expr)  ((expr))
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
#if $on($msvc)
#   define $warning(msg) __pragma( message( msg ) )
#elif $on($gnuc) || $on($clang)
#   define $$warning$impl(msg) _Pragma(#msg)
#   define $warning(msg) $$warning$impl( message( msg ) )
#else
#   define $warning(msg)
#endif

// create a $warning(...) macro
// usage: $warning("this is shown at compile time")
#define $$todo$stringize$impl(X) #X
#define $$todo$stringize(X) $$todo$stringize$impl(X)
#define $todo(...) $warning( __FILE__ "(" $$todo$stringize(__LINE__)") : $todo - " #__VA_ARGS__ " - [ "__func__ " ]" )

#endif // __BRIDGE_HPP__

// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

// Heal is a lightweight C++ framework to aid and debug applications.
// - rlyeh, zlib/libpng licensed // ~listening to Kalas - Monuments to Ruins

// Callstack code is based on code by Magnus Norddahl (See http://goo.gl/LM5JB)
// Mem/CPU OS code is based on code by David Robert Nadeau (See http://goo.gl/8P5Jqv)
// Distributed under Creative Commons Attribution 3.0 Unported License
// http://creativecommons.org/licenses/by/3.0/deed.en_US

#ifndef __HEALHPP__
#define __HEALHPP__

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <deque>

#define HEAL_VERSION "1.0.0" // (2015/10/01) Semantic versioning adherence and clean ups

/* public API */

namespace heal {

    typedef std::function< int( const std::string &in ) > heal_callback_in;

    extern std::vector< heal_callback_in > warns;
    extern std::vector< heal_callback_in > fails;

    void warn( const std::string &error );
    void fail( const std::string &error );

    void add_worker( heal_callback_in fn );

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
    std::string prompt( const std::string &current_value = std::string(), const std::string &title = std::string(), const std::string &caption = std::string() );

    bool is_debug();
    bool is_release();
    bool is_asserting();

    bool is_devel();
    bool is_public();

    #ifndef HEAL_MAX_TRACES
    #define HEAL_MAX_TRACES 128
    #endif

    struct callstack /* : public std::vector<const void*> */ {
        enum { max_frames = HEAL_MAX_TRACES };
        std::vector<void *> frames;
        callstack( bool autosave = false );
        size_t space() const;
        void save( unsigned frames_to_skip = 0 );
        std::vector<std::string> unwind( unsigned from = 0, unsigned to = ~0 ) const;
        std::vector<std::string> str( const char *format12 = "#\1 \2\n", size_t skip_begin = 0 ) const;
        std::string flat( const char *format12 = "#\1 \2\n", size_t skip_begin = 0 ) const;
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
    $cpp11(
    template<> inline std::string hexdump( const std::nullptr_t &obj ) {
        return hexdump( 0,0,0 );
    }
    )
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

#endif // __HEALHPP__
