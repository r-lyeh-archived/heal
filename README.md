heal <a href="https://travis-ci.org/r-lyeh/heal"><img src="https://api.travis-ci.org/r-lyeh/heal.svg?branch=master" align="right" /></a>
====

Heal is a lightweight C++ library to aid and debug applications.

## Features
- [x] C++11 and OS dependencies only.
- [x] C++03 with boost is supported as well.
- [x] Handy, compact, stand-alone and cross-platform.
- [x] ZLIB/libPNG licensed.

## Cons
- [ ] only Win32 target has 100% full API coverage, for now :( PRs welcome :)

## Demo
```c++
#include <iostream>
#include "heal.hpp"

int main() {
    if( !is_asserting() )
        errorbox( "Asserts are disabled. No assertions will be perfomed" );

    std::cout << hexdump(3.14159f) << std::endl;
    std::cout << hexdump("hello world") << std::endl;

    std::cout << ( is_debug() ? "Debug build" : "Release build" ) << std::endl;
    std::cout << stacktrace("\1) \2\n") << std::endl;

    if( !debugger("We are about to launch debugger...") )
        die( "Something went wrong: debugger() didnt work" );
}
```

## Possible output
```
D:\prj\heal>cl sample.cc heal.cpp /Zi
D:\prj\heal>sample.exe
offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F [ptr=0032F69C sz=4]
0032F69C  .  .  I  @  .  .  .  .  .  .  .  .  .  .  .  . asc
0032F69C d0 0f 49 40 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? hex

offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F [ptr=0032F4F4 sz=11]
0032F4F4  h  e  l  l  o     w  o  r  l  d  .  .  .  .  . asc
0032F4F4 68 65 6c 6c 6f 20 77 6f 72 6c 64 ?? ?? ?? ?? ?? hex

Debug build
0) stacktrace (d:\prj\heal\heal.cpp:577)
1) main (d:\prj\heal\sample.cc:16)
2) __tmainCRTStartup (f:\dd\vctools\crt_bld\self_x86\crt\src\crt0.c:240)
3) BaseThreadInitThunk
4) RtlInitializeExceptionChain
5) RtlInitializeExceptionChain

Something went wrong: debugger() didnt work
#0 stacktrace (d:\prj\heal\heal.cpp:577)
#1 die (d:\prj\heal\heal.cpp:586)
#2 main (d:\prj\heal\sample.cc:19)
#3 __tmainCRTStartup (f:\dd\vctools\crt_bld\self_x86\crt\src\crt0.c:240)
#4 BaseThreadInitThunk
#5 RtlInitializeExceptionChain
#6 RtlInitializeExceptionChain
D:\prj\heal>
```

## API
```c++
namespace heal {
  vec<cb> warns; // chain of callbacks when warn() is invoked.
  vec<cb> fails; // chain of callbacks when fail() is invoked.
  warn("error"); // generate a warning. see callbacks above.
  fail("error"); // generate an error. see callbacks above.

  die( "reason", code = 0 );  // exit app
  die( code, "reason" = "" ); // exit app

  void breakpoint();       // break execution, if possible.
  bool debugger("reason"); // invoke debugger, if possible.

  void alert(T value = T(), "title" = "");                   // modal dialog
  void errorbox("body" = "","title" = "");                   // error dialog
  string prompt("value" = "", "title" = "", "caption" = ""); // prompt dialog

  bool is_asserting(); // true if assertions are enabled.
  bool is_release();   // true if either any of `NDEBUG`, `_NDEBUG` or `RELEASE` are defined.
  bool is_debug();     // negation of above
  bool is_public();    // true if either any of `NDEVEL`, `_NDEVEL` or `PUBLIC` are defined.
  bool is_devel();     // negation of above

  string demangle("mangled_symbol"); // return human-readable demangled-symbol, if possible.

  struct callstack;      // save stack on construction. normally used for later usage
  vec<str> stacktrace(); // returns full current callstack (that can be formatted). Like,
  string stackstring();  // returns full current callstack (that can be formatted). Like,
  // #0 stacktrace (heal.cpp, line 592)
  // #1 main (sample.cc, line 14)
  // #2 __tmainCRTStartup (f:\dd\vctools\crt_bld\self_x86\crt\src\crt0.c, line 240)
  // #3 BaseThreadInitThunk
  // #4 RtlInitializeExceptionChain
  // #5 RtlInitializeExceptionChain

  string hexdump(*ptr, len); // returns hexdump of memory pointer. Like,
  string hexdump(T); // returns hexdump of object. Like,
  // offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F [ptr=0014F844 sz=10]
  // 0014F844  H  e  l  l  o  W  o  r  l  d  .  .  .  .  .  . asc
  // 0014F844 48 65 6c 6c 6f 57 6f 72 6c 64 ?? ?? ?? ?? ?? ?? hex
}
```

## API:
@todoc, chain of callbacks

## Notes:
- Visual Studio users must use `/Zi` compiler flag for optimal symbol retrieving.
- G++/clang users must use `/g` compiler flag for optimal symbol retrieving.

## Changelog
- v1.0.0 (2015/10/01)
  - Semantic versioning adherence and clean ups
