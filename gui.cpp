#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gui.hpp"

wingrid grid;

void OnValue( char* txt, int row, int col, int typ ) {
    if(!col) grid.insert_row(txt,row,col,typ);
    else     grid.modify_field(txt,row,col,typ);
}

void OnField( char* txt, int row, int col ,int len ) {
    grid.insert_column(txt,row,col,len);
}

int main( int argc, const char **argv )
{
    window win;

    if( win.create() )
    {
        win.set_title( (char*)"localhost" );
        grid.create( win.hWnd );

        OnField( "name", 0,0, 100 ); OnField( "surname", 0,1, 100 );

        OnValue( "hello", 0,0,0 ); OnValue( "world", 0,1,0 );
        OnValue( "john",  0,0,0 ); OnValue( "doe",   0,1,0 );

        while( win.is_open() )
        {
            win.pump();

            Sleep(1);
        }
    }

    return 0;
}
