/*
 * Compiler/target directives and related macros

 * Copyright (c) 2010 Mario 'rlyeh' Rodriguez
 *
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

 * Notes:
 * - Directives taken from http://predef.sourceforge.net/preos.html#sec25 and
 *   http://www.boost.org/doc/libs/1_35_0/boost/detail/endian.hpp (thanks!)

 * To do:
 * - Add:
 *   DEBUG                     = UNOPTIMIZED +  DEBUG SYMBOL
 *   DEBUG_OPT                 =   OPTIMIZED +  DEBUG SYMBOL
 *   RELEASE_UNOPT             = UNOPTIMIZED + NDEBUG SYMBOL
 *   RELEASE                   =   OPTIMIZED + NDEBUG SYMBOL
 *   (code should rely only on DEBUG/NDEBUG symbols)

 * - rlyeh
 */

#pragma once

#include <string>
#include "heal.hpp"

// Endian detection

#if !defined( $little_endian ) && !defined( $middle_endian ) && !defined( $big_endian )
#   if defined (__GLIBC__)
#     include <endian.h>
#     if    (__BYTE_ORDER == __BIG_ENDIAN)
#         define $big_endian
#     elif  (__BYTE_ORDER == __LITTLE_ENDIAN)
#         define $little_endian
#     elif  (__BYTE_ORDER == __PDP_ENDIAN)
#         define $middle_endian
#     endif
#   else
#     if defined(_BIG_ENDIAN) \
        || defined(__sparc) || defined(__sparc__) \
        || defined(_POWER) || defined(__powerpc__) \
        || defined(__ppc__) || defined(__hpux) \
        || defined(_MIPSEB) || defined(_POWER) \
        || defined(__s390__)
#          define $big_endian
#     elif defined(_LITTLE_ENDIAN) \
        || defined(__i386__) || defined(__alpha__) \
        || defined(__ia64) || defined(__ia64__) \
        || defined(_M_IX86) || defined(_M_IA64) \
        || defined(_M_ALPHA) || defined(__amd64) \
        || defined(__amd64__) || defined(_M_AMD64) \
        || defined(__x86_64) || defined(__x86_64__) \
        || defined(_M_X64)
#          define $little_endian
#     elif (__BYTE_ORDER == __PDP_ENDIAN)
#          define $middle_endian
#     endif
#   endif
#endif

#if !defined( $little_endian ) && !defined( $middle_endian ) && !defined( $big_endian )
#   error Cannot determinate target endianness
#endif

#ifdef $little_endian
#   define $endian_order 1234
#endif

#ifdef $middle_endian   //If this is middle endian, how do i store nuxi endianness?
#   define $endian_order 2143
#endif

#ifdef $big_endian
#   define $endian_order 4321
#endif

// Compiler detection

#ifdef _MSC_VER
#   if _MSC_VER >= 1800
#      define $CompilerMsvc 2013
#      define $CompilerCxx11   1
#   elif _MSC_VER >= 1700
#      define $CompilerMsvc 2012
#      define $CompilerCxx11   1
#   elif _MSC_VER >= 1600
#      define $CompilerMsvc 2010
#      define $CompilerCxx11   0
#   elif _MSC_VER >= 1500
#      define $CompilerMsvc 2008
#      define $CompilerCxx11   0
#   elif _MSC_VER >= 1400
#      define $CompilerMsvc 2005
#      define $CompilerCxx11   0
#   else
#      define $CompilerMsvc 2003 //or older...
#      define $CompilerCxx11   0
#   endif
#endif

#ifdef __GNUC__
#   define $CompilerGcc     (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#   if $CompilerGcc >= 40600 // >= 4.6.0
#       define $CompilerCxx11  1
#   else
#       define $CompilerCxx11  0
#   endif
#endif

#if defined($CompilerCxx11) && $CompilerCxx11 == 0
#   undef $CompilerCxx11
#endif

#if !defined($CompilerCxx11)
#   if __cplusplus > 199711
#       define $CompilerCxx11  1
#   else
#       define $CompilerCxx11  0
#   endif
#endif

// Macro to force inlining

#ifndef $force_inline
#   if defined( $CompilerMsvc )
#       define $force_inline( ... ) __forceinline __VA_ARGS__
#   endif
#   if defined( $CompilerGcc ) || defined( $CompilerMw )
#       define $force_inline( ... ) __inline__ __VA_ARGS__
#   endif
#   if !defined( $force_inline )
#       define $force_inline( ... ) inline __VA_ARGS__
#   endif
#endif

// Macro to prevent inlining

#ifndef $dont_inline
#   if defined( $CompilerMsvc )
#       define $dont_inline( ... ) __declspec(noinline) __VA_ARGS__
#   endif
#   if defined( $CompilerGcc )
#       define $dont_inline( ... ) __VA_ARGS__ __attribute__ ((noinline))
#   endif
#   if !defined( $dont_inline )
#       define $dont_inline( ... ) __VA_ARGS__
#   endif
#endif


// Weak symbols. Usage:
// $weak_symbol( void custom_function( int elems, char *str ) )
// { /*...*/ }

#ifndef $weak_symbol
#   if defined( $CompilerMsvc )
#       define $weak_symbol( ... ) __declspec(noinline) __inline __VA_ARGS__
#   endif
#   if defined( $CompilerGcc )
#       define $weak_symbol( ... ) __VA_ARGS__ __attribute__ ((weak))
#   endif
#   if !defined( $weak_symbol )
#       define $weak_symbol( ... ) __VA_ARGS__
#   endif
#endif

// Fix stupid vc++ warnings

#ifdef $CompilerMsvc
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS 1
#       pragma warning(disable:4996)
#       pragma warning(disable:4244)    // remove me!
#   endif
#endif

// Workaround to fix missing __func__ / __FUNCTION__ / __PRETTY_FUNCTION__ directives in a few compilers

#if defined( $CompilerMsvc )
#   define  __PRETTY_FUNCTION__ __FUNCTION__
#   define  __func__ __FUNCTION__
#endif

#if defined( $CompilerGcc )
#endif

#if !defined( $CompilerMsvc ) && !defined( $CompilerGcc )
#   define  __PRETTY_FUNCTION__ __func__
#   define  __FUNCTION__ __func__
#endif

// if non C99 compiler, redefine'em as "??"
//#   define  __PRETTY_FUNCTION__ "??"
//#   define  __FUNCTION__ "??"
//#   define  __func__ "??"

// Workaround to fix missing __COUNTER__ directive in a few compilers

#if ( __COUNTER__ == __COUNTER__ )
#   define __COUNTER__ __LINE__
#endif


bool confirm( const std::string &question, const std::string &title = std::string() );

// Both functions return an empty string if dialog is cancelled
std::string load_dialog( const char *filter = "All Files (*.*)\0*.*\0" );
std::string save_dialog( const char *filter = "All Files (*.*)\0*.*\0" );

std::string get_clipboard();
void set_clipboard( const std::string &_msg );

void browse( const std::string &url );

std::string get_app_name();
std::string get_home_dir(); // home path
std::string get_temp_dir(); // temp path
bool make_dir( const std::string &fullpath );

void attach_console();
void detach_console();
void set_title_console( const std::string &title );

bool is_app_instanced();
bool is_app_running();
bool is_app_closing();

void sleep_app( double seconds );

void spawn_app( const std::string &appname );
void close_app();
void respawn_app();

void restart_app();

#if $on($release) && ( defined(MASTER) || defined(GOLD) )
#   define $gold      $yes
#else
#   define $gold      $no
#endif

bool is_gold();
bool is_coder();
bool is_tester();
bool is_enduser();
