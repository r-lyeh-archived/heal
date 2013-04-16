#include <iostream>
#include "heal.hpp"

int main()
{
    if( !is_asserting() )
        errorbox( "Asserts are disabled. No assertions will be perfomed" );

    assert3( 1, <, 2);
    assert4( 1, <, 2, "This should never happen" );

    std::cout << hexdump(3.14159f) << std::endl;
    std::cout << hexdump("hello world") << std::endl;

    std::cout << ( is_debug() ? "Debug build" : "Release build" ) << std::endl;
    std::cout << stacktrace("\1) \2\n") << std::endl;

    if( !debugger("We are about to launch debugger...") )
        die( "Something went wrong: debugger() didnt work" );

    return 0;
}
