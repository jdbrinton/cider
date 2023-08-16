# parallel executable location added above this line
set program = `basename "$0"`

# process arguments
set pgroup=0
if ($#argv == 4) then
    if ( x$1 != x-p ) then
	echo "Usage: $program [-p num-nodes] circuit-file output-suffix" 
	exit 1
    else
	set pgroup=1
	set nnodes=$2
	shift; shift
    endif
else if ($#argv != 2) then
    echo "Usage: $program [-p num-nodes] circuit-file output-suffix" 
    exit 1
endif

# Location of file containing machine names formatted one name per line.
if ($?CPUSERVERS) then
    set servers = $CPUSERVERS
else
    set servers = "cpuservers"
endif
if ( ! -f $servers ) then
    echo "${program}: cpuservers file '$servers' not found"  
    exit 1
endif

# setup names
if ( ! -f $1 ) then
    if ( ! -f $1.cir ) then
	echo "${program}: input file '$1' not found" 
	exit 1
    else
	set circuit = $1.cir
	set circuitroot = $1
    endif
else
    set circuit = $1
    set circuitroot = `basename $1 '\.[^.]*'`
endif
set prefix = ${circuitroot}$2

# make the process group file
if ($pgroup == 1) then
    makep $executable $nnodes $servers >! /tmp/${prefix}$$.p
    if ($status == 1) then
      echo ${program}: error creating process group.
      exit 1
    endif
    cp /tmp/${prefix}$$.p  ${prefix}.p
else
    cp ${prefix}.p  /tmp/${prefix}$$.p
endif
if ($status == 1) then
 exit 1
endif

# run in parallel
echo "" 
echo "${program}: parallel job for '${circuit}' started `date`" 
set runtime = `time parallel /tmp/${prefix}$$ -r $prefix.raw -o $prefix.sum -c $circuit >& $prefix.err`
set runstatus = $status
echo "${program}: parallel job for '${circuit}' ended `date`" 
echo "Resource Usage: $runtime"

# clean up
rm -f /tmp/${prefix}$$.p >& /dev/null
rm -f events* >& /dev/null

# combine summary files
if ($runstatus == 0) then 
    rm -f $prefix.sum
    cat /dev/null > $prefix.sum
    if ( -f ${prefix}.sum000 ) then
	foreach file ( ${prefix}.sum??? )
	    echo ${file}: >> $prefix.sum
	    cat $file >> $prefix.sum
	    rm -f $file
	end
    endif
endif
