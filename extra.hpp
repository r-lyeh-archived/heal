/*
 * Heal is a lightweight C++ framework to aid and debug applications.
 * Copyright (c) 2011, 2012, 2013, 2014 Mario 'rlyeh' Rodriguez
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See license copy at http://www.boost.org/LICENSE_1_0.txt)

 * Mem/CPU OS code is based on code by David Robert Nadeau (See http://goo.gl/8P5Jqv)
 * Distributed under Creative Commons Attribution 3.0 Unported License
 * http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#pragma once

#include <stddef.h>
#include <string>

namespace heal 
{
    /**
     * Returns the peak (maximum so far) resident set size (physical
     * memory use) measured in bytes, or zero if the value cannot be
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

