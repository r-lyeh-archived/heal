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

        // add a web server, with stats in ajax
        add_webmain( 8080, []( std::ostream &headers, std::ostream &content, const std::string &url ) {
            if( url.find("/echo") != std::string::npos ) {
                headers << "Content-Type: text/html;charset=UTF-8\r\n";
                content << html_template << "echo: " << url << std::endl;
                return 200;
            } else {
                headers << "Content-Type: application/json\r\n";
                content << "{\"cpu\":0, \"core\":0, \"disk\":0}" << std::endl;
                return 200;
            }
        } );

        //
        std::cout << "webserver installed at localhost:8080. try / and /echo" << std::endl;
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

    srand( time(0) );
    assert( rand() < RAND_MAX/2 && "this will fail half the times you try" );

    if( !debugger("We are about to launch debugger, if possible.") ) {
        die( "debugger() call didnt work. Exiting..." );
    }

    std::cout << "All ok." << std::endl;
    return 0;
}


std::string html_template = $quote(
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>{TITLE}</title>

<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/lib/jquery-1.8.3.min.js"></script>
<!--[if lte IE 8]><script language="javascript" type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/excanvas.min.js"></script><![endif]-->
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jquery.flot.min.js"></script>
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jquery.flot.time.js"></script>
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jshashtable-2.1.js"></script>
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jquery.numberformatter-1.2.3.min.js"></script>
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jquery.flot.symbol.js"></script>
<script type="text/javascript" src="http://www.jqueryflottutorial.com/js/flot/jquery.flot.axislabels.js"></script>
<script>
var cpu = [], cpuCore = [], disk = [];
var dataset;
var totalPoints = 100;
var updateInterval = 500;
var now = new Date().getTime();

var options = {
    series: {
        lines: {
            lineWidth: 1.2
        },
        bars: {
            align: "center",
            fillColor: { colors: [{ opacity: 1 }, { opacity: 1}] },
            barWidth: 500,
            lineWidth: 1
        }
    },
    xaxis: {
        mode: "time",
        tickSize: [60, "second"],
        tickFormatter: function (v, axis) {
            var date = new Date(v);

            if (date.getSeconds() % 20 == 0) {
                var hours = date.getHours() < 10 ? "0" + date.getHours() : date.getHours();
                var minutes = date.getMinutes() < 10 ? "0" + date.getMinutes() : date.getMinutes();
                var seconds = date.getSeconds() < 10 ? "0" + date.getSeconds() : date.getSeconds();

                return hours + ":" + minutes + ":" + seconds;
            } else {
                return "";
            }
        },
        axisLabel: "Time",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 10
    },
    yaxes: [
        {
            min: 0,
            max: 100,
            tickSize: 5,
            tickFormatter: function (v, axis) {
                if (v % 10 == 0) {
                    return v + "%";
                } else {
                    return "";
                }
            },
            axisLabel: "CPU loading",
            axisLabelUseCanvas: true,
            axisLabelFontSizePixels: 12,
            axisLabelFontFamily: 'Verdana, Arial',
            axisLabelPadding: 6
        }, {
            max: 5120,
            position: "right",
            axisLabel: "Disk",
            axisLabelUseCanvas: true,
            axisLabelFontSizePixels: 12,
            axisLabelFontFamily: 'Verdana, Arial',
            axisLabelPadding: 6
        }
    ],
    legend: {
        noColumns: 0,
        position:"nw"
    },
    grid: {
        backgroundColor: { colors: ["#ffffff", "#EDF5FF"] }
    }
};

function initData() {
    for (var i = 0; i < totalPoints; i++) {
        var temp = [now += updateInterval, 0];

        cpu.push(temp);
        cpuCore.push(temp);
        disk.push(temp);
    }
};

function GetData() {
    $.ajaxSetup({ cache: false });

    $.ajax({
        url: "stats",
        type: "GET",
        dataType: 'json',
        success: update,
        error: function () {
            setTimeout(GetData, updateInterval);
        }
        /*  type: "POST",
            data: content,
            complete: function(data) {
                $('#shop section table tbody').append(data);
            },
        */
    });
};

var temp;

function update(_data) {
    cpu.shift();
    cpuCore.shift();
    disk.shift();

    now += updateInterval;

    temp = [now, _data.cpu];
    cpu.push(temp);

    temp = [now, _data.core];
    cpuCore.push(temp);

    temp = [now, _data.disk];
    disk.push(temp);

    dataset = [
        { label: "CPU:" + _data.cpu + "%", data: cpu, lines: { fill: true, lineWidth: 1.2 }, color: "#00FF00" },
        { label: "Disk:" + _data.disk + "KB", data: disk, color: "#0044FF", bars: { show: true }, yaxis: 2 },
        { label: "CPU Core:" + _data.core + "%", data: cpuCore, lines: { lineWidth: 1.2}, color: "#FF0000" }
    ];

    $.plot($("#flot-placeholder1"), dataset, options);
    setTimeout(GetData, updateInterval);
}


$(document).ready(function () {
    initData();

    dataset = [
        { label: "CPU", data: cpu, lines:{fill:true, lineWidth:1.2}, color: "#00FF00" },
        { label: "Disk:", data: disk, color: "#0044FF", bars: { show: true }, yaxis: 2 },
        { label: "CPU Core", data: cpuCore, lines: { lineWidth: 1.2}, color: "#FF0000" }
    ];

    $.plot($("#flot-placeholder1"), dataset, options);
    setTimeout(GetData, updateInterval);
});



</script>
<!-- HTML -->
<div id="flot-placeholder1" style="width:550px;height:300px;margin:0 auto"></div>
);

