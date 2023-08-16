.nr VS 12
.nr PO 1.5i
.LP
CODECS is a mixed-level circuit and device simulator that provides a 
direct link between technology parameters and circuit performance.
Detailed and accurate analyses of semiconductor 
circuits are possible by use of physical (numerical) models for
critical devices. The numerical models are based upon
solution of Poisson's equation and the current-continuity equations.
Analytical models can be used for the noncritical devices.
.LP
CODECS incorporates SPICE3 for the circuit-simulation capability
and for analytical models. Numerical devices are supported by
a new one- and two-dimensional device simulator. CODECS provides
dc, transient, small-signal ac, and pole-zero analyses 
of circuits containing one- and two-dimensional numerical models for 
diodes and bipolar transistors and two-dimensional numerical models 
for MOSFETs. The numerical models in CODECS include 
physical effects such as bandgap narrowing, Shockley-Hall-Read and
Auger recombinations, concentration and field-dependent mobilities,
concentration-dependent lifetimes, and avalanche generation. 
.LP
CODECS is written in the C language. It runs on a DEC VAX
under Ultrix. The compatibility of the
system with non-VAX systems has not been tested, but
the program is expected to be portable to other machines.
The tape is in UNIX \fItar\fP 800 or 1600 BPI format.
.LP
\fBThe following document is included with the tape:\fP
.IP 1. 9
CODECS User's Guide
.LP
\fBAdditional Documentation:\fP
.IP 1. 9
Kartikeya Mayaram,
\fICODECS: A Mixed-Level Circuit and Device Simulator\fP
(UCB/ERL M88/71, November 1988).
