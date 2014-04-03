#include "clipboard/clipboard.hpp"
#include "environment/environment.hpp"

#if defined(NDEBUG) || defined(_NDEBUG)
const bool is_release = true;
const bool is_debug   = false;

const bool is_coder   = false;
const bool is_tester  = false;
const bool is_enduser = true;
#else
const bool is_release = false;
const bool is_debug   = true;

const bool is_coder   =  ( moon9::get_clipboard() ==  "CODER" || moon9::get_environment("USERLEVEL") ==  "CODER" );
const bool is_tester  =  ( moon9::get_clipboard() == "TESTER" || moon9::get_environment("USERLEVEL") == "TESTER" );
const bool is_enduser = !( is_coder || is_tester );
#endif
