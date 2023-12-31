/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

#include "numglobs.h"
#include "numconst.h"
#include "numenum.h"
#include "nummacs.h"
#include "twomesh.h"
#include "twodev.h"

/* Use list-sorting header file to create electrode sorter */
#define TYPE		TWOelectrode
#define NEXT		next
#define SORT		TWOssortElectrodes
#define SORT1		TWOsortElectrodes
#include "lsort.h"

#define ARG_MIN(a,b,c) ((a) > (b) ? 1 : ((a) < (b) ? -1 : (c)))

int TWOcmpElectrode(a, b)
TWOelectrode *a, *b;
{
    return ARG_MIN(a->id, b->id, 0);
}

/*
 * checkElectrodes
 *   debug list of electrodes for errors, exit on any error:
 *   1. list doesn't have only contiguous id's from 1 to idHigh
 *   2. electrodes with box shapes
 *   3. order of node numbers is correct
 */
void
checkElectrodes( pElectrode, idHigh )
TWOelectrode *pElectrode;
int idHigh;
{
  TWOelectrode *pE;
  int id;
  BOOLEAN error = FALSE;

  /*
   * order the electrodes
   * assign electrode numbers to uninitialized electrodes ( id == -1 )
   * uninit electrodes are in reverse order from order in input file
   * resort electrodes
   */
  pElectrode = TWOsortElectrodes( pElectrode, TWOcmpElectrode );
  id = 1;
  for (pE = pElectrode; pE != NIL(TWOelectrode); pE = pE->next) {
    if (pE->id == -1) pE->id = id++;
  }
  pElectrode = TWOsortElectrodes( pElectrode, TWOcmpElectrode );

  for (pE = pElectrode, id = 1; pE != NIL(TWOelectrode); pE = pE->next) {
  /* Check id's */
    if ( pE->id < 1 || pE->id > idHigh) {
      fprintf(stderr, "Error: electrode %d out of range\n",pE->id);
      error = TRUE;
    } else if ( pE->id != id ) {
      if ( pE->id != ++id ) {
	fprintf(stderr, "Error: electrode(s) %d to %d missing\n",id,pE->id-1);
	id = pE->id;
	error = TRUE;
      }
    }
  }

  /* Make sure total number of distinct electrodes is correct */
  if ( id != idHigh ) {
    fprintf(stderr, "Error: %d electrode%s not equal to %d required\n",
	id, (id != 1) ? "s are" : " is", idHigh);
    error = TRUE;
  }
    
  if (error) {
    exit(-1);
  }
}

/*
 * setupContacts
 *   go through each set of electrode segments, find its size and find nodes
 */
#define MAXTERMINALS 5  /* One more than max number of terminals */
#define ELCT_ID poiEqn
void
setupContacts( pDevice, pElectrode, nodeArray )
TWOdevice *pDevice;
TWOelectrode *pElectrode;
TWOnode ***nodeArray;
{
  TWOelectrode *pE;
  TWOcontact *pNew, *pTail;
  TWOnode *pNode;
  int xIndex, yIndex, nIndex;
  int id = 0, electrodeSize[MAXTERMINALS], electrodeType;
  int error = FALSE;

  /* INITIALIZATION
   * 0. Assume ELCT_ID's are initialized to 0 before setup is called
   * 1. Add a node only once to only one electrode
   * 2. Compute number of nodes in each electrode
   * 3. Overwrite SEMICON or INSULATOR nodeType at electrodes
   */
  for ( pE = pElectrode; pE != NIL(TWOelectrode); pE = pE->next ) {
    if (pE->id != id) { /* Found the next electrode */
      id = pE->id;
      electrodeSize[id] = 0;
      electrodeType = NULL;
    }
    for ( xIndex = pE->ixLo; xIndex <= pE->ixHi; xIndex++ ) {
      for ( yIndex = pE->iyLo; yIndex <= pE->iyHi; yIndex++ ) {
        pNode = nodeArray[ xIndex ][ yIndex ];
        if ( pNode != NIL( TWOnode ) ) {
          pNode->nodeType = CONTACT;

	  /* Assign each node to an electrode only once */
	  if (pNode->ELCT_ID == 0) { /* Is this a new node? */
	    pNode->ELCT_ID = id;     /* Mark electrode ownership */
	    electrodeSize[id]++;    /* Increase electrode size */
	  } else if (pNode->ELCT_ID != id) {
				   /* Node already owned by another electrode */
	    fprintf(stderr, "Error: electrodes %d and %d overlap at (%d,%d)\n",
		pNode->ELCT_ID, id, xIndex, yIndex);
	    error = TRUE;
	  }
        }
      }
    }
  }
  if (error) {
    exit(-1);
  }

  /* ALLOCATION AND NODE GRABBING
   * 1. For each electrode, create a new contact structure for the electrode.
   * 2. Fill in entries of contact structure
   * 3. Update First and Last Contact pointers of device
   */
/*
  printElectrodes( pDevice->pFirstContact );
*/
  id = 0;
  pDevice->pFirstContact = pTail = NIL(TWOcontact);
  for ( pE = pElectrode; pE != NIL(TWOelectrode); pE = pE->next ) {
    if (pE->id != id) { /* Found the next electrode */
      if ( pDevice->pFirstContact == NIL(TWOcontact) ) {
	ALLOC( pNew, TWOcontact, 1 );
	pDevice->pFirstContact = pTail = pNew;
      } else {
	ALLOC( pNew->next, TWOcontact, 1 );
	pTail = pNew = pNew->next;
      }
      pNew->next = NIL(TWOcontact);
      id = pNew->id = pE->id;
      pNew->workf = pE->workf;
      pNew->numNodes = electrodeSize[id];
      nIndex = 0;
      ALLOC( pNew->pNodes, TWOnode *, electrodeSize[id] );
    }
    for ( xIndex = pE->ixLo; xIndex <= pE->ixHi; xIndex++ ) {
      for ( yIndex = pE->iyLo; yIndex <= pE->iyHi; yIndex++ ) {
        pNode = nodeArray[ xIndex ][ yIndex ];
        if ( pNode != NIL( TWOnode ) ) {
	  /* Make sure node belongs to this electrode, then
	   * clear ELCT_ID so that it is not grabbed again.
	   */
	  if (pNode->ELCT_ID == id) {
	    pNew->pNodes[ nIndex++ ] = pNode;
	    pNode->ELCT_ID = 0;
	  }
	}
      }
    }
  }
  pDevice->pLastContact = pTail;
}
