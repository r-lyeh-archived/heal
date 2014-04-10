#include <iostream>
#include <fstream>
#include "heal.hpp"

extern std::string html_template;

int main()
{
    // print this on compile time
    $warning("I *still* have to document this library");

    // print some stats
    std::cout << timestamp() << std::endl;
    std::cout << ( is_debug() ? "Debug build" : "Release build" ) << std::endl;
    std::cout << "Using " << get_mem_current_str() << '/' << get_mem_size_str() << std::endl;

    // print current stack trace
    for( auto &line : stacktrace("\1) \2") ) {
        std::cout << line << std::endl;
    }

    // measure scope
    {
        scoped_benchmark<> sb("background workers installation");

        // add a parallel worker
        add_worker( []( const std::string &text ) {
            static int i = 0;
            std::cout << ( std::string("\r") + "\\|/-"[ (++i) % 4 ] ) << std::flush;
            return true;
        } );

        // add a web server
        add_webmain( 8080, []( std::ostream &out, const std::string &url ) {
            out << html_template << "webthread echo: " << url << std::endl;
            return 200;
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
    alert( prompt("type a number") );

    if( !is_asserting() ) {
        errorbox( "Asserts are disabled. No assertions will be perfomed" );
    } else {
        alert( "Asserts are enabled. Assertions will be perfomed" );
    }

    assert1( true );
    assert2( true, "This should never fail" );
    assert3( 50, <, 100 );
    assert4( 100, ==, 100, "Why not " << 100 << '?' );

    if( !debugger("We are about to launch debugger, if possible.") ) {
        die( "debugger() call didnt work. Exiting..." );
    }

    return 0;
}

std::string html_template = $quote(
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>{TITLE}</title>
    <link href="http://www.flotcharts.org/flot/examples/examples.css" rel="stylesheet" type="text/css">
    <!--[if lte IE 8]><script language="javascript" type="text/javascript" src="http://www.flotcharts.org/flot/excanvas.min.js"></script><![endif]-->
    <script language="javascript" type="text/javascript" src="http://www.flotcharts.org/flot/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="http://www.flotcharts.org/flot/jquery.flot.js"></script>
    <script type="text/javascript">
    $(function() {
        var datasets = {
            "usa": {
                label: "sample #0",
                data: [[1988, 483994], [1989, 479060], [1990, 457648], [1991, 401949], [1992, 424705], [1993, 402375], [1994, 377867], [1995, 357382], [1996, 337946], [1997, 336185], [1998, 328611], [1999, 329421], [2000, 342172], [2001, 344932], [2002, 387303], [2003, 440813], [2004, 480451], [2005, 504638], [2006, 528692]]
            },
            "russia": {
                label: "sample #1",
                data: [[1988, 218000], [1989, 203000], [1990, 171000], [1992, 42500], [1993, 37600], [1994, 36600], [1995, 21700], [1996, 19200], [1997, 21300], [1998, 13600], [1999, 14000], [2000, 19100], [2001, 21300], [2002, 23600], [2003, 25100], [2004, 26100], [2005, 31100], [2006, 34700]]
            },
            "uk": {
                label: "sample #2",
                data: [[1988, 62982], [1989, 62027], [1990, 60696], [1991, 62348], [1992, 58560], [1993, 56393], [1994, 54579], [1995, 50818], [1996, 50554], [1997, 48276], [1998, 47691], [1999, 47529], [2000, 47778], [2001, 48760], [2002, 50949], [2003, 57452], [2004, 60234], [2005, 60076], [2006, 59213]]
            },
            "germany": {
                label: "sample #3",
                data: [[1988, 55627], [1989, 55475], [1990, 58464], [1991, 55134], [1992, 52436], [1993, 47139], [1994, 43962], [1995, 43238], [1996, 42395], [1997, 40854], [1998, 40993], [1999, 41822], [2000, 41147], [2001, 40474], [2002, 40604], [2003, 40044], [2004, 38816], [2005, 38060], [2006, 36984]]
            },
            "denmark": {
                label: "sample #4",
                data: [[1988, 3813], [1989, 3719], [1990, 3722], [1991, 3789], [1992, 3720], [1993, 3730], [1994, 3636], [1995, 3598], [1996, 3610], [1997, 3655], [1998, 3695], [1999, 3673], [2000, 3553], [2001, 3774], [2002, 3728], [2003, 3618], [2004, 3638], [2005, 3467], [2006, 3770]]
            },
            "sweden": {
                label: "sample #5",
                data: [[1988, 6402], [1989, 6474], [1990, 6605], [1991, 6209], [1992, 6035], [1993, 6020], [1994, 6000], [1995, 6018], [1996, 3958], [1997, 5780], [1998, 5954], [1999, 6178], [2000, 6411], [2001, 5993], [2002, 5833], [2003, 5791], [2004, 5450], [2005, 5521], [2006, 5271]]
            },
            "norway": {
                label: "sample #6",
                data: [[1988, 4382], [1989, 4498], [1990, 4535], [1991, 4398], [1992, 4766], [1993, 4441], [1994, 4670], [1995, 4217], [1996, 4275], [1997, 4203], [1998, 4482], [1999, 4506], [2000, 4358], [2001, 4385], [2002, 5269], [2003, 5066], [2004, 5194], [2005, 4887], [2006, 4891]]
            },
            "random" : {
                label: "sample #7",
                data: [[1988,4000]]
            }
        };
        // hard-code color indices to prevent them from shifting as
        // countries are turned on/off
        var i = 0;
        $.each(datasets, function(key, val) {
            val.color = i;
            ++i;
        });
        // insert checkboxes
        var choiceContainer = $("#choices");
        $.each(datasets, function(key, val) {
            choiceContainer.append("<br/><input type='checkbox' name='" + key +
                "' checked='checked' id='id" + key + "'></input>" +
                "<label for='id" + key + "'>"
                + val.label + "</label>");
        });
        choiceContainer.find("input").click(plotAccordingToChoices);
        var year = 1989;
        function plotAccordingToChoices() {
            var data = [];
            choiceContainer.find("input:checked").each(function () {
                var key = $(this).attr("name");
                if (key && datasets[key]) {
                    data.push(datasets[key]);
                }
            });
            if (data.length > 0) {
                $.plot("#placeholder", data, {
                    yaxis: {
                        min: 0
                    },
                    xaxis: {
                        tickDecimals: 0
                    }
                });
            }
            datasets["random"].data.push( [year++, Math.floor( Math.random() * 600000 ) ] );
            //alert(JSON.stringify(datasets));
            setTimeout(plotAccordingToChoices, 1000);
        }
        plotAccordingToChoices();
        // Initiate a recurring data update
        $("button.dataUpdate").click(function () {
            data = [];
            alreadyFetched = {};
            $.plot("#placeholder", data, options);
            var iteration = 0;
            function fetchData() {
                ++iteration;
                function onDataReceived(series) {
                    // Load all the data in one pass; if we only got partial
                    // data we could merge it with what we already have.
                    data = [ series ];
                    $.plot("#placeholder", data, options);
                }
                // Normally we call the same URL - a script connected to a
                // database - but in this case we only have static example
                // files, so we need to modify the URL.
                $.ajax({
                    url: "data-eu-gdp-growth-" + iteration + ".json",
                    type: "GET",
                    dataType: "json",
                    success: onDataReceived
                });
                if (iteration < 5) {
                    setTimeout(fetchData, 1000);
                } else {
                    data = [];
                    alreadyFetched = {};
                }
            }
        });
        // Add the Flot version string to the footer
        $("#footer").prepend("Flot " + $.plot.version + " &ndash; ");
    });
    </script>
</head>
<body>
    <div id="header">
        <h2>{TITLE}</h2>
    </div>
    <div id="content">
        <p>{USAGE}</p>
        <p>{REPORT}</p>
        <div class="demo-container">
            <div id="placeholder" class="demo-placeholder" style="float:left; width:675px;"></div>
            <p id="choices" style="float:right; width:135px;"></p>
        </div>
        <p>This is a placeholder example (source: <a href="https://github.com/r-lyeh/heal">heal</a>).</p>
        <p>
            <button class="dataUpdate">Do nothing</button>
        </p>
        <p>{SETTINGS}</p>
    </div>
    <!--div id="footer">
        Copyright &copy; 2007 - 2013 IOLA and Ole Laursen
    </div-->
</body>
</html>
);

