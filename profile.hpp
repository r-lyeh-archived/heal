/**
 * Simple profiler class

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

 * To do:
 * - Detailed report tree like this one http://stackoverflow.com/questions/910172/track-c-memory-allocations

 * - rlyeh ~~ listening to Team Ghost / High hopes
 */

#pragma once

#include <cassert>

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <deque>
#include <string>
#include <stdarg.h>
#include <sstream>

#include "heal.hpp"

namespace base9 {
    std::string format( const char *fmt, ... ) {
        char buf[128];
        va_list args;
        va_start( args, fmt );
        if( vsprintf( buf, fmt, args ) < 0 ) {
            buf[0] = '\0';
        }
        va_end(args);
        return buf;
    }
}

namespace debug9
{
    // corazonada: en arquitecturas monohilo mientras haya alg�n counter vivo *creo poder asegurar* que siempre es
    // hijo de otro (tree). estaria bien ordenarlo por autokey de creacion.

    class detail
    {
        std::deque< std::string > depth;

        struct info
        {
            int hits = 0;
            double time = 0;
            std::string alias;
            double created = 0;

            info( const std::string &_alias = std::string() ) : alias( _alias )
            {}
        };

        std::map< std::string /*hash*/, info > counters;
        typedef std::map< std::string /*hash*/, info >::iterator counters_it;

        detail()
        {}

        public:

        static detail *instance() {
            static detail d;
            return &d;
        }

        ~detail() {
            std::cout << report( false ) << std::endl;
        }

        void add( const std::string &id ) {
            depth.push_back( !depth.size() ? id : depth.back() + ":" + id ) ;

            if( counters.find( id ) == counters.end() )
                counters[ id ] = info( depth.back() );

            counters[ id ].created = get_time_os();
            assert( counters[ id ].created > 0 );
        }

        void del( const std::string &id ) {
            if( counters[ id ].created ) {
                counters[ id ].hits ++;
                counters[ id ].time += ( get_time_os() - counters[ id ].created );
                counters[ id ].created = 0;
            }

            depth.pop_back();
        }

        std::string report( bool reversed = false ) const {
            std::string out = std::string() + "Report: ok, " + std::to_string( counters.size() ) + " profile(s) analyzed\n";
            std::vector< std::string > sort;

            // update times

            auto starts_with = []( const std::string &a, const std::string &b ) {
                if( a.size() >= b.size() ) {
                    if( a.substr(0,b.size()) == b ) {
                        return true;
                    }
                }
                return false;
            };

            {
                // sorted tree
                struct pair {
                    std::string first;
                    info second;
                    bool operator<( const pair &p ) const {
                        return first < p.first;
                    }
                };
                std::vector< pair > az_tree;

                for( auto &it : counters ) {
                    az_tree.push_back( { it.second.alias, it.second } );
                }

                std::sort( az_tree.begin(), az_tree.end() );
                std::reverse( az_tree.begin(), az_tree.end() );

                // update time hierarchically
                for( size_t i = 0; i < az_tree.size(); ++i ) {
                    for( size_t j = i + 1; j < az_tree.size(); ++j )
                        if( starts_with( az_tree[ i ].first, az_tree[ j ].first ) )
                            az_tree[ j ].second.time -= az_tree[ i ].second.time;
                }
            }


            double total = 0;

            for( auto &it : counters ) {
                total += it.second.time;
            }

            assert( total > 0 );

            size_t i = 0;
            for( const auto &it : counters ) {
                ++i;
                double cpu = it.second.time * 100.0 / total;

                if( cpu < 10.0 )
                    sort.push_back( base9::format( "%4d. 00%3.2f%% cpu, %5d hits, %s (%fs)\n", i, cpu, it.second.hits, it.second.alias.c_str(), it.second.time ) );
                else
                if( cpu < 100.0 )
                    sort.push_back( base9::format(  "%4d. 0%3.2f%% cpu, %5d hits, %s (%fs)\n", i, cpu, it.second.hits, it.second.alias.c_str(), it.second.time ) );
                else
                    sort.push_back( base9::format(   "%4d. %3.2f%% cpu, %5d hits, %s (%fs)\n", i, cpu, it.second.hits, it.second.alias.c_str(), it.second.time ) );
            }

            std::sort( sort.begin(), sort.end() );

            if( reversed ) {
                std::reverse( sort.begin(), sort.end() );
            }

            for( auto &it : sort ) {
                out += it;
            }

            return out;
        }
    };

    class profile
    {
        std::string id;

        profile()
        {}

        public:

        explicit profile( const std::string &file, const int &line ) {
            detail::instance()->add( id = base9::format("%s:%d", file.c_str(), line ) );
        }

        explicit profile( const std::string &alias ) {
            detail::instance()->add( id = alias );
        }

        ~profile() {
            detail::instance()->del( id );
        }

        static std::string report() {
            return detail::instance()->report();
        }
    };
}
