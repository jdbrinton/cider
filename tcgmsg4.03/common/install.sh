#! /bin/csh -f

# Install header files and library in area accessible to SPICE3 and CIDER.
set MACHINE = $1

if (! -d ../${MACHINE}) then
  mkdir ../${MACHINE}
  mkdir ../${MACHINE}/include
  mkdir ../${MACHINE}/lib
  mkdir ../${MACHINE}/bin
endif

cp ipcv4.0/*.h ../${MACHINE}/include
cp ipcv4.0/libtcgmsg.a ../${MACHINE}/lib
cp ipcv4.0/parallel makep ../${MACHINE}/bin
