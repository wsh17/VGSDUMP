# VGSDUMP
Versatec Graphics Dump Analyzer is an older utility written to analyze graphic output files
# Authors
Bill Harper - creater inventor and creator of the Utility started this tool in Pascal, turbo Pascal. I then converted to "C" to learn this new technology, well new at the time to me.  I then was asked if we could include this with our product (Versaplot) as it was a great debuging tool.  I was then helped by Mark Tamura - versatec s/w engineer, Dale Trip - versatec s/w engineer as to built in toO Versaplot as a utility and released with the product. Versaplot and Versaplot had an analyzer, but only for the imtermediate data file, and not for the final output file that would be sent to and rendered by the Versatec controller. 
# This is a parser that supports VDS (Versatec Data Standards) and is written in "C" but converted from a Pascal base. 
This data format is old, but the opensource of this as it can be modified to perform other tasks
as the parser itself is quite plugable for 30 year code.

This code was written by Bill Harper as a trouble shooting tool for the SE's and then adopted by the Xerox
engineering team and added to the Versaplot product base as an output analyizer, a tool much needed.  This was distributed as part of the Versatec Versaplot products, Versaplot9, Versaplot Randon, and Versaplot VRF. 

Purpose:
This tool reads graphics data (binary) an prints out an analysis in full or summary view.  It handles all VDS formats including VRF, VDS Raster (1D, 2D 2D Optimized) and blocked raster.  It is build around a state machine, as the headers and data require it based on headers and graphic commands that can span headers.

History:
Stated life as a Pascal program, fully function on a Xerox 820 computer running CP/M or the openversion ZCPR using a Turbo Pascal Compiler we took of the Apple IIE and changed the binary headers to run on a Xerox 820. The 820 and Apple IIE had the same CPU, just different OS. We used Kermet or X-modem to upload the files for analysis on the ZCPR (CP/M Clone) system running a BBS system for remote access.  This BBS ran in my Versatec office in Pleasanton, CA.  

Convert code to "C" - to force myself to learn it as they did not teach it to me in college.
Ported to Versate 820 controllers (Prism) to capture and anaylze data running under CP/M. 
Productized by Versatec engineering and added to Versaplot and VGS graphis pacaages, so ported to Vax, Sun, HP and Apollo OS and also PC-DOS. 
