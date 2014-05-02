#include "profile.hpp"

void test_D() {
    debug9::profile p( "test_D()" );
}

void test_C() {
    debug9::profile p( "test_C()" );
    test_D();
}

void test_B() {
    debug9::profile p( "test_B()" );
}

void test_A() {
    debug9::profile p( "test_A()" );
    test_B();
}

int main( int argc, const char **argv ) {
    debug9::profile p( "main()" );

    for( int i = 0; i < 100; ++i ) {
        if( i % 2 ) {
            test_A();
        }
        test_C();
    }

    debug9::profile p2("main2()");
}
