# VGSDUMP
Versatec Graphics Dump Analyzer is an older utility written to analyze graphic output files
# Authors
Bill Harper - creater inventor, Mark Tamura - versatec s/w engineer, Dale Trip - versatec s/w engineer
# This is a parser that supports VDS data written in "C"
This data format is old, but the opensource of this as it can be modified to perform other tasks
as the parser itself is quite plugable for 20 year code.

This code was written by Bill Harper as a trouble shooting tool for the SE's and them adopted by the Xerox
engineering team and added to the product base line as an output analyizer, a tool much needed.  This was distributed as part of the Versatec Versaplot products, Versaplot9, Versaplot Randon, and Versaplot VRF. 

Purpose:
This tool reads graphics data (binary) an prints out an analysis in full or summary view.  It handles all VDS formats including VRF, VDS Raster (1D, 2D 2D Optimized) and blocked raster.

History:
Stated life as a Pascal program, fully function on a Xerox 820 computer running CP/M or the openversion ZCPR using a Turbo Pascal Compiler we took of the Apple IIE and changed the binary headers to run on a Xerox 820. The 820 and Apple IIE had the same CPU, just different OS. We used Kermet or X-modem to upload the files for analysis on the ZCPR (CP/M Clone) system running a BBS system for remote access.  This BBS ran in my Versatec office in Pleasanton, CA.  

Convert code to "C" - to force myself to learn it as they did not teach it to me in college.
Ported to Versate 820 controllers (Prism) to capture and anaylze data running under CP/M. 
Productized by Versatec engineering and added to Versaplot and VGS graphis pacaages, so ported to Vax, Sun, HP and Apollo OS and also PC-DOS. 
