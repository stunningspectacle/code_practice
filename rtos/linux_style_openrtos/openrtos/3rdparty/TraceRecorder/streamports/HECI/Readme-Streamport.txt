Tracealyzer Stream Port for HECI
-------------------------------------------------

This directory contains a "stream port" for the Tracealyzer recorder library,
i.e., the specific code needed to use a particular interface for streaming a
Tracealyzer RTOS trace. The stream port is defined by a set of macros in
trcStreamingPort.h, found in the "include" directory.

This particular stream port targets HECI.

To use this stream port, make sure that include/trcStreamingPort.h is found
by the compiler (i.e., add this folder to your project's include paths) and
add all included source files to your build. Make sure no other versions of
trcStreamingPort.h are included by mistake!

