// Heal is a lightweight C++ framework to aid and debug applications.
// - rlyeh, zlib/libpng licensed

// Mem/CPU OS code is based on code by David Robert Nadeau (See http://goo.gl/8P5Jqv)
// Distributed under Creative Commons Attribution 3.0 Unported License
// http://creativecommons.org/licenses/by/3.0/deed.en_US

#include "heal.hpp"
#include "extra.hpp"

namespace {

    template<typename T>
    std::string to_string( const T &t, int digits = 20 ) {
        std::stringstream ss;
        ss.precision( digits );
        ss << std::fixed << t;
        return ss.str();
    }

    template<>
    std::string to_string( const bool &boolean, int digits ) {
        return boolean ? "true" : "false";
    }

    template<>
    std::string to_string( const std::istream &is, int digits ) {
        std::stringstream ss;
        std::streamsize at = is.rdbuf()->pubseekoff(0,is.cur);
        ss << is.rdbuf();
        is.rdbuf()->pubseekpos(at);
        return ss.str();
    }
}

namespace heal
{
    std::string human_size( size_t bytes ) {
        /**/ if( bytes >= 1024 * 1024 * 1024 ) return to_string( bytes / (1024 * 1024 * 1024)) + " GiB";
        else if( bytes >=   16 * 1024 * 1024 ) return to_string( bytes / (       1024 * 1024)) + " MiB";
        else if( bytes >=          16 * 1024 ) return to_string( bytes / (              1024)) + " KiB";
        else                                   return to_string( bytes ) + " bytes";
    }
#if 0
    std::string human_time( double time ) {
        /**/ if( time >   48 * 3600 ) return to_string( time / (24*3600), 0 ) + " days";
        else if( time >    120 * 60 ) return to_string( time / (60*60), 0 ) + " hours";
        else if( time >=        120 ) return to_string( time / 60, 0 ) + " mins";
        else if( time >= 1 || time <= 0 ) return to_string( time, 2 ) + " s";
        else if( time <= 1 / 1000.f )
            return to_string( time * 1000000, 0 ) + " ns";
        else
            return to_string( time * 1000, 0 ) + " ms";
    }
#endif
    std::string human_time( double time ) {
        std::uint64_t time64 = std::uint64_t( time );
        std::uint64_t secs64 = ( time64 % 60 );
        std::uint64_t mins64 = ( time64 / 60 ) % 60;
        std::uint64_t hour64 = ( time64 / 3600 ) % 24;
        std::uint64_t days64 = ( time64 / 86400 );
        std::string csecs = to_string( int( time * 100 ) % 100 );
        std::string sign = std::string() + (time < 0 ? '-' : '+');
        return sign +
            to_string(days64) + (hour64 < 10 ? "d:0" : "d:") +
            to_string(hour64) + (mins64 < 10 ? "h:0" : "h:") +
            to_string(mins64) + (secs64 < 10 ? "m:0" : "m:") +
            to_string(secs64) + "." + csecs + "s";
    }

    std::string get_mem_peak_str() {
        return human_size( get_mem_peak() );
    }
    std::string get_mem_current_str() {
        return human_size( get_mem_current() );
    }
    std::string get_mem_size_str() {
        return human_size( get_mem_size() );
    }
    std::string get_time_cpu_str() {
        return human_time( get_time_cpu() );
    }
    static double start = -get_time_clock();
    std::string get_time_clock_str() {
        return human_time( get_time_clock() + start );
    }

}



#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define get_mem_peak() or get_mem_current() for an unknown OS."
#endif



#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#if defined(BSD)
#include <sys/sysctl.h>
#endif

#else
#error "Unable to define get_mem_size() for an unknown OS."
#endif



#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>

#else
#error "Unable to define get_time_cpu() for an unknown OS."
#endif





#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h> /* POSIX flags */
#include <time.h>   /* clock_gettime(), time() */
#include <sys/time.h>   /* gethrtime(), gettimeofday() */

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#else
#error "Unable to define get_time_clock() for an unknown OS."
#endif

namespace heal {

    /**
     * Returns the peak (maximum so far) resident set size (physical
     * memory use) measured in bytes, or zero if the value cannot be
     * determined on this OS.
     */
    size_t get_mem_peak( )
    {
    #if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        info.cb = sizeof(info);
        GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
        return (size_t)info.PeakWorkingSetSize;

    #elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
        /* AIX and Solaris ------------------------------------------ */
        struct psinfo psinfo;
        int fd = -1;
        if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
            return (size_t)0L;      /* Can't open? */
        if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
        {
            close( fd );
            return (size_t)0L;      /* Can't read? */
        }
        close( fd );
        return (size_t)(psinfo.pr_rssize * 1024L);

    #elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        /* BSD, Linux, and OSX -------------------------------------- */
        struct rusage rusage;
        getrusage( RUSAGE_SELF, &rusage );
    #if defined(__APPLE__) && defined(__MACH__)
        return (size_t)rusage.ru_maxrss;
    #else
        return (size_t)(rusage.ru_maxrss * 1024L);
    #endif

    #else
        /* Unknown OS ----------------------------------------------- */
        return (size_t)0L;          /* Unsupported. */
    #endif
    }


    /**
     * Returns the current resident set size (physical memory use) measured
     * in bytes, or zero if the value cannot be determined on this OS.
     */
    size_t get_mem_current( )
    {
    #if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        info.cb = sizeof(info);
        GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
        return (size_t)info.WorkingSetSize;

    #elif defined(__APPLE__) && defined(__MACH__)
        /* OSX ------------------------------------------------------ */
        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
            (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
            return (size_t)0L;      /* Can't access? */
        return (size_t)info.resident_size;

    #elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
        /* Linux ---------------------------------------------------- */
        long rss = 0L;
        FILE* fp = NULL;
        if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
            return (size_t)0L;      /* Can't open? */
        if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
        {
            fclose( fp );
            return (size_t)0L;      /* Can't read? */
        }
        fclose( fp );
        return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

    #else
        /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
        return (size_t)0L;          /* Unsupported. */
    #endif
    }


    /**
     * Returns the size of physical memory (RAM) in bytes.
     */
    size_t get_mem_size( )
    {
    #if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
        /* Cygwin under Windows. ------------------------------------ */
        /* New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit */
        MEMORYSTATUS status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatus( &status );
        return (size_t)status.dwTotalPhys;

    #elif defined(_WIN32)
        /* Windows. ------------------------------------------------- */
        /* Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS */
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx( &status );
        return (size_t)status.ullTotalPhys;

    #elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        /* UNIX variants. ------------------------------------------- */
        /* Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM */

    #if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
        int mib[2];
        mib[0] = CTL_HW;
    #if defined(HW_MEMSIZE)
        mib[1] = HW_MEMSIZE;            /* OSX. --------------------- */
    #elif defined(HW_PHYSMEM64)
        mib[1] = HW_PHYSMEM64;          /* NetBSD, OpenBSD. --------- */
    #endif
        std::int64_t size = 0;               /* 64-bit */
        size_t len = sizeof( size );
        if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
            return (size_t)size;
        return 0L;          /* Failed? */

    #elif defined(_SC_AIX_REALMEM)
        /* AIX. ----------------------------------------------------- */
        return (size_t)sysconf( _SC_AIX_REALMEM ) * (size_t)1024L;

    #elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
        /* FreeBSD, Linux, OpenBSD, and Solaris. -------------------- */
        return (size_t)sysconf( _SC_PHYS_PAGES ) *
            (size_t)sysconf( _SC_PAGESIZE );

    #elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
        /* Legacy. -------------------------------------------------- */
        return (size_t)sysconf( _SC_PHYS_PAGES ) *
            (size_t)sysconf( _SC_PAGE_SIZE );

    #elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
        /* DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. -------- */
        int mib[2];
        mib[0] = CTL_HW;
    #if defined(HW_REALMEM)
        mib[1] = HW_REALMEM;        /* FreeBSD. ----------------- */
    #elif defined(HW_PYSMEM)
        mib[1] = HW_PHYSMEM;        /* Others. ------------------ */
    #endif
        unsigned int size = 0;      /* 32-bit */
        size_t len = sizeof( size );
        if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
            return (size_t)size;
        return 0L;          /* Failed? */
    #endif /* sysctl and sysconf variants */

    #else
        return 0L;          /* Unknown OS. */
    #endif
    }


    /**
     * Returns the amount of CPU time used by the current process,
     * in seconds, or -1.0 if an error occurred.
     */
    double get_time_cpu( )
    {
    #if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        FILETIME createTime;
        FILETIME exitTime;
        FILETIME kernelTime;
        FILETIME userTime;
        if ( GetProcessTimes( GetCurrentProcess( ),
            &createTime, &exitTime, &kernelTime, &userTime ) != -1 )
        {
            SYSTEMTIME userSystemTime;
            if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
                return (double)userSystemTime.wHour * 3600.0 +
                    (double)userSystemTime.wMinute * 60.0 +
                    (double)userSystemTime.wSecond +
                    (double)userSystemTime.wMilliseconds / 1000.0;
        }

    #elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        /* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and Solaris --------- */

    #if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
        /* Prefer high-res POSIX timers, when available. */
        {
            clockid_t id;
            struct timespec ts;
    #if _POSIX_CPUTIME > 0
            /* Clock ids vary by OS.  Query the id, if possible. */
            if ( clock_getcpuclockid( 0, &id ) == -1 )
    #endif
    #if defined(CLOCK_PROCESS_CPUTIME_ID)
                /* Use known clock id for AIX, Linux, or Solaris. */
                id = CLOCK_PROCESS_CPUTIME_ID;
    #elif defined(CLOCK_VIRTUAL)
                /* Use known clock id for BSD or HP-UX. */
                id = CLOCK_VIRTUAL;
    #else
                id = (clockid_t)-1;
    #endif
            if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
                return (double)ts.tv_sec +
                    (double)ts.tv_nsec / 1000000000.0;
        }
    #endif

    #if defined(RUSAGE_SELF)
        {
            struct rusage rusage;
            if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
                return (double)rusage.ru_utime.tv_sec +
                    (double)rusage.ru_utime.tv_usec / 1000000.0;
        }
    #endif

    #if defined(_SC_CLK_TCK)
        {
            const double ticks = (double)sysconf( _SC_CLK_TCK );
            struct tms tms;
            if ( times( &tms ) != (clock_t)-1 )
                return (double)tms.tms_utime / ticks;
        }
    #endif

    #if defined(CLOCKS_PER_SEC)
        {
            clock_t cl = clock( );
            if ( cl != (clock_t)-1 )
                return (double)cl / (double)CLOCKS_PER_SEC;
        }
    #endif

    #endif

        return -1;      /* Failed. */
    }


    /**
     * Returns the real time, in seconds, or -1.0 if an error occurred.
     *
     * Time is measured since an arbitrary and OS-dependent start time.
     * The returned real time is only useful for computing an elapsed time
     * between two calls to this function.
     */
    double get_time_clock()
    {
    #if defined(_WIN32)
        FILETIME tm;
        ULONGLONG t;
    #if 0 //defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
        /* Windows 8, Windows Server 2012 and later. ---------------- */
    //  GetSystemTimePreciseAsFileTime( &tm );
    #else
        /* Windows 2000 and later. ---------------------------------- */
        GetSystemTimeAsFileTime( &tm );
    #endif
        t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
        return (double)t / 10000000.0;

    #elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
        /* HP-UX, Solaris. ------------------------------------------ */
        return (double)gethrtime( ) / 1000000000.0;

    #elif defined(__MACH__) && defined(__APPLE__)
        /* OSX. ----------------------------------------------------- */
        static double timeConvert = 0.0;
        if ( timeConvert == 0.0 )
        {
            mach_timebase_info_data_t timeBase;
            (void)mach_timebase_info( &timeBase );
            timeConvert = (double)timeBase.numer /
                (double)timeBase.denom /
                1000000000.0;
        }
        return (double)mach_absolute_time( ) * timeConvert;

    #elif defined(_POSIX_VERSION)
        /* POSIX. --------------------------------------------------- */
    #if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
        {
            struct timespec ts;
    #if defined(CLOCK_MONOTONIC_PRECISE)
            /* BSD. --------------------------------------------- */
            const clockid_t id = CLOCK_MONOTONIC_PRECISE;
    #elif defined(CLOCK_MONOTONIC_RAW)
            /* Linux. ------------------------------------------- */
            const clockid_t id = CLOCK_MONOTONIC_RAW;
    #elif defined(CLOCK_HIGHRES)
            /* Solaris. ----------------------------------------- */
            const clockid_t id = CLOCK_HIGHRES;
    #elif defined(CLOCK_MONOTONIC)
            /* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
            const clockid_t id = CLOCK_MONOTONIC;
    #elif defined(CLOCK_REALTIME)
            /* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
            const clockid_t id = CLOCK_REALTIME;
    #else
            const clockid_t id = (clockid_t)-1; /* Unknown. */
    #endif /* CLOCK_* */
            if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
                return (double)ts.tv_sec +
                    (double)ts.tv_nsec / 1000000000.0;
            /* Fall thru. */
        }
    #endif /* _POSIX_TIMERS */

        /* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
        struct timeval tm;
        gettimeofday( &tm, NULL );
        return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
    #else
        return -1.0;        /* Failed. */
    #endif
    }
}
