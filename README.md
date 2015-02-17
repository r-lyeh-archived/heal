heal <a href="https://travis-ci.org/r-lyeh/heal"><img src="https://api.travis-ci.org/r-lyeh/heal.svg?branch=master" align="right" /></a>
====

- Heal is a lightweight C++ library to aid and debug applications. Requires C++11 (or C++03 with boost at least).
- Heal is stand-alone. OS dependencies only.
- Heal is cross-platform. Full API is only supported under Windows, for now. Feel free to tweak heal.cpp and add more OSes :)
- Heal is BOOST licensed.

### Sample
```c++
#include <iostream>
#include "heal.hpp"

int main()
{
    if( !is_asserting() )
        errorbox( "Asserts are disabled. No assertions will be perfomed" );

    assert( 1 < 2 && "Try to tweak this to see the detailed info" );

    std::cout << hexdump(3.14159f) << std::endl;
    std::cout << hexdump("hello world") << std::endl;

    std::cout << ( is_debug() ? "Debug build" : "Release build" ) << std::endl;
    std::cout << stacktrace("\1) \2\n") << std::endl;

    if( !debugger("We are about to launch debugger...") )
        die( "Something went wrong: debugger() didnt work" );

    return 0;
}
```

### Possible output
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

### API
- `assert(expr)`. Same behavior, extra information.
- `warn()` @todoc.
- `fail()` @todoc.
- `warns(cb)` @todoc.
- `fails(cb)` @todoc.
- `bool debugger(string reason)` tries to invoke debugger, if possible.
- `void breakpoint()` breaks execution, if possible.
- `string demangle(string symbol)` returns an human-readable mangled-symbol, if possible.
- `alert(type value,string title)` shows a modal window.
- `errorbox(string body,string title)` shows an error window.
- `prompt(string value, string title, string caption)` shows a prompt window.
- `bool is_asserting()` returns true if assertions are enabled.
- `bool is_release()` returns true if either `NDEBUG` or `_NDEBUG` preprocessor directives are defined.
- `bool is_debug()` returns negation of `is_release()`
- `safestring class` @todoc.
- `callstack()` @todoc.
- `stackstring()` @todoc.
- `sfstring()` class @todoc.
- `sfstrings()` class @todoc.
- `stacktrace()` returns a vector of strings that contains full current callstack. Output can be formatted. Possible output for `std::cout << stacktrace()`:
```
#0 stacktrace (heal.cpp, line 592)
#1 main (sample.cc, line 14)
#2 __tmainCRTStartup (f:\dd\vctools\crt_bld\self_x86\crt\src\crt0.c, line 240)
#3 BaseThreadInitThunk
#4 RtlInitializeExceptionChain
#5 RtlInitializeExceptionChain
```
- `hexdump()` returns an std::string hexdump of given memory pointers. Possible output for `std::cout << hexdump(memory)`:
```
offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F [ptr=0014F844 sz=10]
0014F844  H  e  l  l  o  W  o  r  l  d  .  .  .  .  .  . asc
0014F844 48 65 6c 6c 6f 57 6f 72 6c 64 ?? ?? ?? ?? ?? ?? hex
```

### API:
@todoc, chain of callbacks
