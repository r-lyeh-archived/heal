#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <iostream>

#include "heal.hpp"

using namespace heal;

// print this on compile time
$warning("I *still* have to document this library");

// html template for web server
extern std::string html_template;

int main()
{
    // print some stats
    std::cout << timestamp() << std::endl;
    std::cout << ( is_debug() ? "Debug build" : "Release build" ) << std::endl;

    // print current stack trace
    for( auto &line : stacktrace("\1) \2") ) {
        std::cout << line << std::endl;
    }

    // measure scope
    {
        // add a parallel worker
        add_worker( []( const std::string &text ) {
            static int i = 0;
            std::cout << ( std::string("\r") + "\\|/-"[ (++i) % 4 ] ) << std::flush;
            return true;
        } );
    }

    // initialize chain of warns and fails
    // these prototypes return !=0 if they handle issue, or return 0 to delegate issue to inner ring
    warns.push_back( []( const std::string &text ) {
        alert( text, "this is our custom assert title" );
        return true;
    });

    fails.push_back( []( const std::string &error ) {
        errorbox( error + "\n\n" + stackstring("\1) \2\n", 7) );
        // die();
        return true;
    });

    alert( "this is a test" );
    warn("this is a warning");
    fail("this is a fail");

    alert( 3.14159f );
    alert( -100 );
    alert( std::ifstream(__FILE__), "current source code" );
    alert( hexdump(3.14159f) );
    alert( hexdump("hello world") );
    alert( prompt("0", "type a number") );

    if( !is_asserting() ) {
        errorbox( "Asserts are disabled. No assertions will be perfomed" );
    } else {
        alert( "Asserts are enabled. Assertions will be perfomed" );
    }

    srand( time(0) );
    assert( rand() < RAND_MAX/2 && "this will fail half the times you try" );

    if( !debugger("We are about to launch debugger, if possible.") ) {
        die( "debugger() call didnt work. Exiting..." );
    }

    std::cout << "All ok." << std::endl;
    return 0;
}

