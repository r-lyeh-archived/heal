// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

/* Smart assert replacement for LHS/RHS values. BOOST licensed.
 * - rlyeh ~~ listening to Tuber / Desert Overcrowded
 */

/* Public API */

#include <cassert>

#if !(defined(NDEBUG) || defined(_NDEBUG))
#undef  assert
#define assert(...) ( bool(__VA_ARGS__) ? \
        ( assertpp::check(#__VA_ARGS__,__FILE__,__LINE__,1) < __VA_ARGS__ ) : \
        ( assertpp::check(#__VA_ARGS__,__FILE__,__LINE__,0) < __VA_ARGS__ ) )
#endif

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
        bool ok;
        std::deque< std::string > xpr;
        template<typename T> static std::string to_str( const T &t ) { std::stringstream ss; return (ss << t) ? ss.str() : "??"; }
    public:
        check( const char *const text, const char *const file, int line, bool result )
        :   xpr(4), ok(result)
        { xpr[0] = std::string(file) + ':' + to_str(line); xpr[2] = text; }

        ~check() {
            if( xpr.empty() ) return;
            operator bool();
            xpr[0] = xpr[0] + xpr[1];
            xpr.erase( xpr.begin() + 1 );
            if( !ok ) {
                xpr[2] = xpr[2].substr( xpr[2][2] == ' ' ? 3 : 4 );
                xpr[1].resize( (xpr[1] != xpr[2]) * xpr[1].size() );
                std::string buf;
                buf = "<assert++> says: expression failed! (" + xpr[1] + ") -> (" + xpr[2] + ") -> (unexpected) at " + xpr[0] + "\n";
                fprintf(stderr, "%s", buf.c_str() );
                // assert fallback here
                fclose( stderr );
                fclose( stdout );
                assert( !"<assert++> says: expression failed!" );
                // user defined fallbacks here
                for(;;) {}
            };
        }
#       define assert$impl(OP) \
        template<typename T> check &operator OP( const T &rhs         ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; } \
        template<unsigned N> check &operator OP( const char (&rhs)[N] ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; }
        operator bool() {
            if( xpr.size() >= 3 && xpr[3].size() >= 6 ) {
                char sign = xpr[3].at(xpr[3].size()/2+1);
                bool equal = xpr[3].substr( 4 + xpr[3].size()/2 ) == xpr[3].substr( 3, xpr[3].size()/2 - 3 );
                ok = ( sign == '=' ? equal : ( sign == '!' ? !equal : ok ) );
            } return ok;
        }
        assert$impl( <); assert$impl(<=); assert$impl( >); assert$impl(>=); assert$impl(!=); assert$impl(==); assert$impl(^=);
        assert$impl(&&); assert$impl(&=); assert$impl(& ); assert$impl(||); assert$impl(|=); assert$impl(| ); assert$impl(^ );
#       undef assert$impl
    };
}

#endif // ASSERTPP_HEADER

// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

// std 11 <-> 03/w-boost bridge compatiblity layer, plus a few macro utils.
// - rlyeh, boost licensed.

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

#if defined(NDEBUG) || defined(_NDEBUG)
#   define $release   $yes
#   define $debug     $no
#else
#   define $release   $no
#   define $debug     $yes
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

/*
 * Heal is a lightweight C++ framework to aid and debug applications.
 * Copyright (c) 2011, 2012, 2013, 2014 Mario 'rlyeh' Rodriguez

 * Callstack code is based on code by Magnus Norddahl (See http://goo.gl/LM5JB)
 * Mem/CPU OS code is based on code by David Robert Nadeau (See http://goo.gl/8P5Jqv)

 * Distributed under the Boost Software License, Version 1.0.
 * (See license copy at http://www.boost.org/LICENSE_1_0.txt)

 * - rlyeh // ~listening to Kalas - Monuments to Ruins
 */
 
#ifndef __HEALHPP__
#define __HEALHPP__

#include <stdio.h>

#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <deque>

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

    // sfstring is a safe string replacement that does not rely on stringstream
    // this is actually safer on corner cases, like crashes, exception unwinding and in exit conditions
    class sfstring : public std::string
    {
        public:

        // basic constructors

        sfstring() : std::string()
        {}

        template<size_t N>
        sfstring( const char (&cstr)[N] ) : std::string( cstr )
        {}

        // constructor sugars

#if 0
        // version that may crash on /MT on destructors
        // (just because it depends on std::locale which may be deinitialized before crashing code)
        template <typename T>
        /* explicit */ sfstring( const T &t ) : std::string()
        {
            std::stringstream ss;
            ss.precision( std::numeric_limits< long double >::digits10 + 1 );
            if( ss << t )
                this->assign( ss.str() );
        }
#else
        template<typename T>
        sfstring( const T &t ) : std::string( std::string(t) ) 
        {}
        sfstring( const std::string &t ) : std::string( t ) 
        {}

        sfstring( const int &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%d", t ) > 0 ) this->assign(buf);
        }
        sfstring( const unsigned long &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%lu", t ) > 0 ) this->assign(buf);
        }
        sfstring( const unsigned long long &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%llu", t ) > 0 ) this->assign(buf);
        }
$msvc(
        sfstring( const size_t &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%lu", t ) > 0 ) this->assign(buf);
        }
)

        sfstring( const float &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%f", t ) > 0 ) this->assign(buf);
        }
        sfstring( const double &t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%f", t ) > 0 ) this->assign(buf);
        }

        sfstring( const char *t ) : std::string( t ? t : "" ) 
        {}
        sfstring( void *t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%p", t ) > 0 ) this->assign(buf);
        }

        sfstring( const void *t ) : std::string() {
            char buf[128];
            if( sprintf(buf, "%p", t ) > 0 ) this->assign(buf);
        }
        sfstring( char *t ) : std::string( t ? t : "" ) 
        {}
#endif

        // extended constructors; safe formatting

        private:
        template<unsigned N>
        std::string &safefmt( const std::string &fmt, std::string (&t)[N] ) {
            for( std::string::const_iterator it = fmt.begin(), end = fmt.end(); it != end; ++it ) {
                unsigned index(*it);
                if( index <= N ) t[0] += t[index];
                else t[0] += *it;
            }
            return t[0];
        }
        public:

        template< typename T1 >
        sfstring( const std::string &fmt, const T1 &t1 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2), sfstring(t3) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2), sfstring(t3), sfstring(t4) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2), sfstring(t3), sfstring(t4), sfstring(t5) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2), sfstring(t3), sfstring(t4), sfstring(t5), sfstring(t6) };
            assign( safefmt( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
        sfstring( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7 ) : std::string() {
            std::string t[] = { std::string(), sfstring(t1), sfstring(t2), sfstring(t3), sfstring(t4), sfstring(t5), sfstring(t6), sfstring(t7) };
            assign( safefmt( fmt, t ) );
        }

        // chaining operators

        template< typename T >
        sfstring &operator +=( const T &t ) {
            return append( sfstring(t) ), *this;
        }

        template< typename T >
        sfstring &operator <<( const T &t ) {
            return append( sfstring(t) ), *this;
        }

        sfstring &operator <<( std::ostream &( *pf )(std::ostream &) ) {
            return *pf == static_cast<std::ostream& ( * )(std::ostream&)>( std::endl ) ? (*this) += "\n", *this : *this;
        }

        // assignment sugars

        template< typename T >
        sfstring& operator=( const T &t ) {
            if( &t != this ) {
                *this = sfstring(t);
            }
            return *this;
        }

        sfstring &operator=( const char *t ) {
            return assign( t ? t : "" ), *this;
        }

        std::string str() const {
            return *this;
        }

        size_t count( const std::string &substr ) const {
            size_t n = 0;
            std::string::size_type pos = 0;
            while( (pos = this->find( substr, pos )) != std::string::npos ) {
                n++;
                pos += substr.size();
            }
            return n;
        }

        sfstring replace( const std::string &target, const std::string &replacement ) const {
            size_t found = 0;
            sfstring s = *this;
            while( ( found = s.find( target, found ) ) != std::string::npos ) {
                s.std::string::replace( found, target.length(), replacement );
                found += replacement.length();
            }
            return s;
        }
    };   

    class sfstrings : public std::deque< sfstring >
    {
        public:

        sfstrings( unsigned size = 0 ) : std::deque< sfstring >( size )
        {}

        template <typename CONTAINER>
        sfstrings( const CONTAINER &c ) : std::deque< sfstring >( c.begin(), c.end() ) 
        {}

        template <typename CONTAINER>
        sfstrings &operator =( const CONTAINER &c ) {
            if( &c != this ) {
                *this = sfstrings( c );
            }
            return *this;
        }

        sfstring str( const char *format1 = "\1\n" ) const {
            if( this->size() == 1 )
                return *this->begin();

            sfstring out;

            for( const_iterator it = this->begin(); it != this->end(); ++it )
                out += sfstring( format1, (*it) );

            return out;
        }

        sfstring flat() const {
            return str( "\1" );
        }
    };    


    /*
     * determined on this OS.
     */
    size_t get_mem_peak();

    /**
     * Returns the current resident set size (physical memory use) measured
     * in bytes, or zero if the value cannot be determined on this OS.
     */
    size_t get_mem_current();

    /**
     * Returns the size of physical memory (RAM) in bytes.
     */
    size_t get_mem_size();

    /**
     * Returns the amount of CPU time used by the current process,
     * in seconds, or -1.0 if an error occurred.
     */
    double get_time_cpu();

    /**
     * Returns the real time, in seconds, or -1.0 if an error occurred.
     *
     * Time is measured since an arbitrary and OS-dependent start time.
     * The returned real time is only useful for computing an elapsed time
     * between two calls to this function.
     */
    double get_time_clock();

    std::string human_size( size_t bytes );
    std::string human_time( double time );

    std::string get_mem_peak_str();
    std::string get_mem_current_str();
    std::string get_mem_size_str();
    std::string get_time_cpu_str();
    std::string get_time_clock_str();
}

#endif // __HEALHPP__
