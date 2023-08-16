\s+4\fBCIDER\fP\s0
.sp 1.5
.PP
CIDER is a mixed-level circuit and device simulator that provides a
direct link between technology parameters and circuit performance.
In cases where compact semiconductor device models are inaccurate or
nonexistent, a mixed-level circuit and device simulator can provide
greater accuracy by numerically modeling the critical devices in
a circuit.  Compact models can be used for the noncritical devices.
A tool such as CIDER can be useful in the development of new process
technologies and device designs, improved compact models, ASIC standard
cell libraries, and critical subcircuits of full-custom ICs.

.PP
CIDER
couples the latest version of SPICE3 (version 3F.2) to an internal C-based
device simulator, DSIM.  SPICE3 provides circuit analyses,
compact models for semiconductor devices, and an interactive
user interface.  DSIM provides accurate, one- and two-dimensional
numerical device models based on the solution of Poisson's equation, and
the electron and hole current-continuity equations.  DSIM incorporates
many of the same basic physical models found in the
the Stanford two-dimensional device simulator PISCES.
Input to CIDER consists of a SPICE-like description of the circuit
and its compact models, and PISCES-like descriptions of the
structures of numerically modeled devices.  As a result, CIDER should
seem familiar to designers already accustomed to these two tools.
For example, SPICE3F.2 input files should run without modification,
producing identical results.

.PP
CIDER is based on the mixed-level circuit and device simulator CODECS,
and is a replacement for this program.  The basic algorithms of the two
programs are the same.  Some of the differences
between CIDER and CODECS are described below.  The CIDER input format
has greater flexibility and allows increased access to physical
model parameters.  New physical models
have been added to allow simulation of state-of-the-art devices.
These include transverse field mobility degradation, important in
scaled-down MOSFETs, and a polysilicon model for poly-emitter
bipolar transistors.  Temperature dependence has been included
over the range from -55C to 125C.
The numerical models can be used to simulate all the basic types
of semiconductor devices: resistors, MOS capacitors, diodes, BJTs, JFETs
and MOSFETs.  BJTs and JFETs can be modeled with or without a substrate
contact.  Support has been added for the capture of device internal states.
Post-processing of device states can be performed using
the NUTMEG user interface of SPICE3.
Previously computed states can be loaded into the program to provide accurate
initial guesses for subsequent analyses.  Finally, numerous small bugs
have been discovered and fixed, and the program has been ported to a wider
variety of computing platforms.

.PP
Berkeley tradition calls for the naming
of new versions of programs by affixing a (number, letter, number) triplet
to the end of the program name.  Under this scheme, CIDER should instead
be named CODECS2A.1.  However, tradition has been broken in this case because
major incompatibilities exist between the two programs and because it was
observed that the acronym CODECS is already used in the analog design community
to refer to coder-decoder circuits.

.sp 1.5
\fBHardware/Operating System Requirements:\fp
The program has been run successfully under:
(Ultrix 4, RISC),
(SunOS 4, Sun4),
(AIX 3, RS/6000),
(UNIX SVR3, iPSC/860 node).
Compatibility with other computer systems has not been tested.
However, the program is expected to compile on most machines with
UNIX, a C compiler and IEEE-754 floating point arithmetic.  At least
8MB physical memory is recommended.
.sp .3
.ne 3
.LP
\fBVersions Available:\fP
UNIX only, available Q1 1993
.sp .3
.ne 3
.LP
\fBDistribution Media:\fP
QIC-120, QIC-150, QIC-320, 8mm (2.2 gig), TK50 (DEC tape format),
9-track 1600 bpi, 9-track 6250 bpi
.sp .3
.ne 3
.LP
\fBSource Code:\fP
Yes
.sp .3
.ne 3
.LP
\fBObject Code:\fP
No
.sp .3
.ne 3
.LP
\fBProgramming Language:\fP
C
.sp .3
.ne 3
.LP
\fBMaterials/Handling Fee:\fP
$150.00
.sp .3
.ne 3
.LP
\fBDocumentation included with the tape:\fP
.IP 1.
CIDER User's Guide.
Available separately for $5.00.
.sp .3
.ne 3
.LP
\fBAdditional Documentation:\fP
.IP 1.
K. Mayaram,
\fICODECS: A Mixed-Level Circuit and Device Simulator,\fP
UCB/ERL M88/71, November 1988.
Available for $15.00.
.IP 2.
D. Gates,
\fI An Inversion-Layer Mobility Model for CODECS,\fP
UCB/ERL M90/96, October 1990.
Available for $10.00.
.IP 2.
D. Gates,
\fIDesign-Oriented Mixed-Level Circuit and Device Simulation,\fP
Forthcoming in late Spring 1993.
.sp .3
.ne 3
.LP
\fBForeign Distribution:\fP
Contact Software Distribution Office
.bp
