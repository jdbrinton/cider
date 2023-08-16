/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Authors: 1985 Thomas L. Quarles, 1991 David A. Gates
**********/

    /* CKTdump(ckt)
     * this is a simple program to dump the rhs vector to stdout
     */

#include "spice.h"
#include <stdio.h>
#include "strext.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "gendefs.h"
#include "devdefs.h"
#include "suffix.h"

extern SPICEdev *DEVices[];

#ifdef __STDC__
extern 	void NDEVdump(CKTcircuit *ckt);
extern	void NUMDdump(GENmodel *mod, CKTcircuit *ckt);
extern	void NBJTdump(GENmodel *mod, CKTcircuit *ckt);
extern	void NUMD2dump(GENmodel *mod, CKTcircuit *ckt);
extern	void NBJT2dump(GENmodel *mod, CKTcircuit *ckt);
extern	void NUMOSdump(GENmodel *mod, CKTcircuit *ckt);
#else
extern	void NDEVdump();
extern	void NUMDdump();
extern	void NBJTdump();
extern	void NUMD2dump();
extern	void NBJT2dump();
extern	void NUMOSdump();
#endif /*__STDC__*/

void
CKTdump(ckt,ref,plot)
    register CKTcircuit *ckt;
    double ref;
    GENERIC *plot;
{
    IFvalue refData;
    IFvalue valData;

    refData.rValue = ref;
    valData.v.numValue = ckt->CKTmaxEqNum-1;
    valData.v.vec.rVec = ckt->CKTrhsOld+1;
    (*(SPfrontEnd->OUTpData))(plot,&refData,&valData);

    /* Now dump whatever each device is interested in dumping */
    NDEVdump(ckt);
}

/*
 * Routine to dump internal values of numerical devices
 * This is inefficient, because we have to look up the indices
 * of the devices in the device table.  Would be simpler if
 * DEVices had an entry for a dumping function, so that indirection
 * could be used.
 */

void
NDEVdump(ckt)
    register CKTcircuit *ckt;
{
    register int i;

    for (i=0;i<DEVmaxnum;i++) {
        if ( (strcmp("NUMD",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMDdump( ckt->CKThead[i], ckt );
	} else if ( (strcmp("NBJT",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NBJTdump( ckt->CKThead[i], ckt );
	} else if ( (strcmp("NUMD2",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMD2dump( ckt->CKThead[i], ckt );
	} else if ( (strcmp("NBJT2",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NBJT2dump( ckt->CKThead[i], ckt );
	} else if ( (strcmp("NUMOS",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMOSdump( ckt->CKThead[i], ckt );
        }
    }
    return;
}

/*
 * Routine to dump statistics about numerical devices
 * This is inefficient, because we have to look up the indices
 * of the devices in the device table.  Would be simpler if
 * DEVices had an entry for an accounting function, so that indirection
 * could be used.
 */

void
NDEVacct(ckt, file)
    register CKTcircuit *ckt;
    FILE *file;
{
    register int i;

    if ( !ckt->CKTisSetup ) {
      return;
    }

    for (i=0;i<DEVmaxnum;i++) {
        if ( (strcmp("NUMD",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMDacct( ckt->CKThead[i], ckt, file );
	} else if ( (strcmp("NBJT",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NBJTacct( ckt->CKThead[i], ckt, file );
	} else if ( (strcmp("NUMD2",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMD2acct( ckt->CKThead[i], ckt, file );
	} else if ( (strcmp("NBJT2",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NBJT2acct( ckt->CKThead[i], ckt, file );
	} else if ( (strcmp("NUMOS",(*DEVices[i]).DEVpublic.name) == 0) &&
                (ckt->CKThead[i] != NULL) ) {
	    NUMOSacct( ckt->CKThead[i], ckt, file );
        }
    }
    return;
}
