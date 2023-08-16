/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

#include "nummacs.h"
#include "gendev.h"

void printCoordInfo( pFirstCoord )
     CoordInfo *pFirstCoord;
{
  CoordInfo *pCoord;
  
  for ( pCoord = pFirstCoord; pCoord ISNOT NIL(CoordInfo);
      pCoord = pCoord->next ) {
    fprintf(stderr, "mesh number=%4d  location=%11.4e\n",
	    pCoord->number, pCoord->location );
  }
}

void killCoordInfo( pFirstCoord )
     CoordInfo *pFirstCoord;
{
  CoordInfo *pCoord, *pKill;
  
  for ( pCoord = pFirstCoord; pCoord ISNOT NIL(CoordInfo); ) {
    pKill = pCoord;
    pCoord = pCoord->next;
    FREE( pKill );
  }
}

void ONEprintDomainInfo( pFirstDomain )
     DomainInfo *pFirstDomain;
{
  DomainInfo *pDomain;
  
  for ( pDomain = pFirstDomain; pDomain ISNOT NIL(DomainInfo);
       pDomain = pDomain->next ) {
    fprintf( stderr, "domain id=%4d  mat=%4d  ixLo=%4d  ixHi=%4d\n",
	    pDomain->id, pDomain->material, pDomain->ixLo, pDomain->ixHi );
  }
}

void TWOprintDomainInfo( pFirstDomain )
     DomainInfo *pFirstDomain;
{
  DomainInfo *pDomain;
  
  for ( pDomain = pFirstDomain; pDomain ISNOT NIL(DomainInfo);
       pDomain = pDomain->next ) {
    fprintf( stderr,
	    "domain id=%4d  mat=%4d  ixLo=%4d  ixHi=%4d  iyLo=%4d  iyHi=%4d\n",
	    pDomain->id, pDomain->material,
	    pDomain->ixLo, pDomain->ixHi,
	    pDomain->iyLo, pDomain->iyHi);
  }
}

void killDomainInfo( pFirstDomain )
     DomainInfo *pFirstDomain;
{
  DomainInfo *pDomain, *pKill;
  
  for ( pDomain = pFirstDomain; pDomain ISNOT NIL(DomainInfo); ) {
    pKill = pDomain;
    pDomain = pDomain->next;
    FREE( pKill );
  }
}

void ONEprintBoundaryInfo( pFirstBoundary )
     BoundaryInfo *pFirstBoundary;
{
  BoundaryInfo *pBoundary;
  
  for ( pBoundary = pFirstBoundary; pBoundary ISNOT NIL(BoundaryInfo);
       pBoundary = pBoundary->next ) {
    fprintf( stderr,
	    "boundary dom=%4d  nbr=%4d  ixLo=%4d  ixHi=%4d\n",
	    pBoundary->domain, pBoundary->neighbor,
	    pBoundary->ixLo, pBoundary->ixHi );
  }
}

void TWOprintBoundaryInfo( pFirstBoundary )
     BoundaryInfo *pFirstBoundary;
{
  BoundaryInfo *pBoundary;
  
  for ( pBoundary = pFirstBoundary; pBoundary ISNOT NIL(BoundaryInfo);
       pBoundary = pBoundary->next ) {
    fprintf( stderr,
	    "boundary dom=%4d  nbr=%4d  ixLo=%4d  ixHi=%4d  iyLo=%4d  iyHi=%4d\n",
	    pBoundary->domain, pBoundary->neighbor,
	    pBoundary->ixLo, pBoundary->ixHi,
	    pBoundary->iyLo, pBoundary->iyHi);
  }
}

void killBoundaryInfo( pFirstBoundary )
     BoundaryInfo *pFirstBoundary;
{
  BoundaryInfo *pBoundary, *pKill;
  
  for ( pBoundary = pFirstBoundary; pBoundary ISNOT NIL(BoundaryInfo); ) {
    pKill = pBoundary;
    pBoundary = pBoundary->next;
    FREE( pKill );
  }
}

void TWOprintElectrodeInfo( pFirstElectrode )
     ElectrodeInfo *pFirstElectrode;
{
  ElectrodeInfo *pElectrode;
  
  for ( pElectrode = pFirstElectrode; pElectrode ISNOT NIL(ElectrodeInfo);
       pElectrode = pElectrode->next ) {
    fprintf( stderr,
	"electrode id=%4d  ixLo=%4d  ixHi=%4d  iyLo=%4d  iyHi=%4d\n",
	pElectrode->id, pElectrode->ixLo, pElectrode->ixHi,
	pElectrode->iyLo, pElectrode->iyHi );
  }
}

void killElectrodeInfo( pFirstElectrode )
     ElectrodeInfo *pFirstElectrode;
{
  ElectrodeInfo *pElectrode, *pKill;
  
  for ( pElectrode = pFirstElectrode; pElectrode ISNOT NIL(ElectrodeInfo); ) {
    pKill = pElectrode;
    pElectrode = pElectrode->next;
    FREE( pKill );
  }
}
