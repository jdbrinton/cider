/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include "numglobs.h"
#include "nummacs.h"
#include "numenum.h"
#include "onemesh.h"
#include "onedev.h"

/* Forward Declarations */
void ONEgetStatePointers();
static void ONEresetEvalFlag();

void
ONEbuildMesh(pDevice, pCoord, pDomain, pMaterial)
  ONEdevice *pDevice;
  ONEcoord *pCoord;
  ONEdomain *pDomain;
  ONEmaterial *pMaterial;
{
  int index, i;
  int elemType;
  double xPos;
  ONEcoord *pC;
  ONEnode *pNode, *pNextNode;
  ONEdomain *pD;
  ONEelem *pElem;
  ONEmaterial *pM;
  int poiEqn, numEqn;
  ONEedge *pEdge;
  ONEnode **nodeArray;
  BOOLEAN error = FALSE;

  /* generate the work array for setting up nodes and elements */
  ALLOC(nodeArray, ONEnode *, 1 + pDevice->numNodes);

  for (pC = pCoord; pC ISNOT NIL(ONEcoord); pC = pC->next) {
    xPos = pC->location;
    ALLOC(pNode, ONEnode, 1);
    pNode->x = xPos;
    pNode->nodeI = pC->number;
    nodeArray[pNode->nodeI] = pNode;
  }

  /* mark the domain info on the nodes */
  if (pDomain IS NIL(ONEdomain)) {
    fprintf(stderr, "Error: domains not defined for device\n");
    exit(-1);
  }
  for (pD = pDomain; pD ISNOT NIL(ONEdomain); pD = pD->next) {
    for (pM = pMaterial; pM != NIL(ONEmaterial); pM = pM->next) {
      if (pD->material == pM->id) {
	break;
      }
    }
    elemType = pM->type;
    for (index = pD->ixLo; index <= pD->ixHi; index++) {
      pNode = nodeArray[index];
      if (NOT pNode->nodeType) {
	pNode->nodeType = elemType;
      }
    }
  }

  /*
   * check to see if a domain has been defined for all nodes. if not flag an
   * error message
   */
  for (index = 2; index < pDevice->numNodes; index++) {
    pNode = nodeArray[index];
    if (NOT pNode->nodeType) {
      printf("Error: No domain defined for node %d\n", pNode->nodeI);
      error = TRUE;
    }
  }
  if (error) {
    /* nodes with undefined domains -- exit */
    exit(-1);
  }
  /* mark the first and last nodes to be contact nodes */
  nodeArray[1]->nodeType = CONTACT;
  nodeArray[pDevice->numNodes]->nodeType = CONTACT;


  /* generate the elements and the edges */
  for (index = 1; index < pDevice->numNodes; index++) {
    ALLOC(pElem, ONEelem, 1);
    ALLOC(pEdge, ONEedge, 1);
    pElem->pEdge = pEdge;
    pElem->pLeftNode = nodeArray[index];
    pElem->pRightNode = nodeArray[index + 1];
    pDevice->elemArray[index] = pElem;
  }

  /* now setup the nodes to which an element belongs */
  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    pElem->pLeftNode->pRightElem = pElem;
    pElem->pRightNode->pLeftElem = pElem;
    if (index > 1) {
      pElem->pLeftElem = pDevice->elemArray[index - 1];
    }
    if (index < pDevice->numNodes - 1) {
      pElem->pRightElem = pDevice->elemArray[index + 1];
    }
  }

  /* mark the domain info on the elements */
  for (pD = pDomain; pD ISNOT NIL(ONEdomain); pD = pD->next) {
    for (pM = pMaterial; pM != NIL(ONEmaterial); pM = pM->next) {
      if (pD->material == pM->id) {
	break;
      }
    }
    elemType = pM->type;
    for (index = pD->ixLo; index < pD->ixHi; index++) {
      pElem = pDevice->elemArray[index];
      pElem->domain = pD->id;
      pElem->elemType = elemType;
      pElem->matlInfo = pM;
    }
  }

  /* identify the interface nodes */
  for (index = 2; index < pDevice->numNodes; index++) {
    pNode = nodeArray[index];
    if (pNode->pLeftElem->elemType ISNOT
	pNode->pRightElem->elemType) {
      /* an interface node */
      pNode->nodeType = INTERFACE;
    }
  }

  /* now mark the nodes to be evaluated */
  /* all interface nodes marked in silicon elements */

  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    pElem->dx = pElem->pRightNode->x - pElem->pLeftNode->x;
    for (i = 0; i <= 1; i++) {
      pNode = pElem->pNodes[i];
      pElem->evalNodes[i] = FALSE;
      if (pElem->elemType IS INSULATOR) {
	if (NOT pNode->evaluated AND
	    pNode->nodeType ISNOT INTERFACE) {
	  /* a non interface node in oxide domain */
	  pNode->evaluated = TRUE;
	  pElem->evalNodes[i] = TRUE;
	}
      }
      if (pElem->elemType IS SEMICON) {
	if (NOT pNode->evaluated) {
	  pNode->evaluated = TRUE;
	  pElem->evalNodes[i] = TRUE;
	}
      }
    }
  }

  /* set up the equation number for the nodes */
  poiEqn = numEqn = 1;
  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    for (i = 0; i <= 1; i++) {
      if (pElem->evalNodes[i]) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  pNode->poiEqn = poiEqn++;

	  pNode->psiEqn = numEqn;
	  if (pElem->elemType IS INSULATOR) {
	    numEqn += 1;	/* only poisson's equation */
	  } else {
	    pNode->nEqn = numEqn + 1;
	    pNode->pEqn = numEqn + 2;
	    numEqn += 3;
	  }
	} else {		/* this is a contact node */
	  pNode->poiEqn = 0;
	  pNode->psiEqn = 0;
	  pNode->nEqn = 0;
	  pNode->pEqn = 0;
	}
	/*
	 * fprintf(stdout,"NODE: %d %d\n",pNode->nodeI,pNode->poiEqn);
	 */
      }
    }
  }
  pDevice->dimEquil = poiEqn;
  pDevice->dimBias = numEqn;

  /*
   * ONEprnMesh( pDevice );
   */
}

/*
 * We have a separate function for this, so that the setup routines can reset
 * the state pointers without rebuilding the entire mesh.
 */
void
ONEgetStatePointers(pDevice, numStates)
  ONEdevice *pDevice;
  int *numStates;
{
  int eIndex, nIndex;
  ONEelem *pElem;
  ONEnode *pNode;

  for (eIndex = 1; eIndex < pDevice->numNodes; eIndex++) {
    pElem = pDevice->elemArray[eIndex];
    for (nIndex = 0; nIndex <= 1; nIndex++) {
      if (pElem->evalNodes[nIndex]) {
	pNode = pElem->pNodes[nIndex];
	pNode->nodeState = *numStates;
	*numStates += ONEnumNodeStates;
      }
    }
    pElem->pEdge->edgeState = *numStates;
    *numStates += ONEnumEdgeStates;
  }
}

/* adjust the base contact to be the position of maximum density */
/* index EB and BC are the indexes for the eb, bc junctions */
void
adjustBaseContact(pDevice, indexEB, indexBC)
  ONEdevice *pDevice;
  int indexEB, indexBC;
{
  int index, i, newBaseIndex, midPoint;
  double maxDensity;
  ONEnode *pNode;
  ONEelem *pElem;
  ONEnode *pBaseNode = pDevice->elemArray[pDevice->baseIndex]->pNodes[0];

  /* Initialize the base contact to be the center of the two junctions */
  /* This should take care of uniform dopings. */

  midPoint = 0.5 * (indexEB + indexBC);
  newBaseIndex = midPoint;

  if (pBaseNode->baseType IS P_TYPE) {
    maxDensity = pDevice->elemArray[midPoint]->pNodes[0]->pConc;
    for (index = indexEB; index < indexBC; index++) {
      pElem = pDevice->elemArray[index];
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->pConc > maxDensity) {
	  maxDensity = pNode->pConc;
	  newBaseIndex = index;
	}
      }
    }
  } else if (pBaseNode->baseType IS N_TYPE) {
    maxDensity = pDevice->elemArray[midPoint]->pNodes[0]->nConc;
    for (index = indexEB; index < indexBC; index++) {
      pElem = pDevice->elemArray[index];
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->nConc > maxDensity) {
	  maxDensity = pNode->nConc;
	  newBaseIndex = index;
	}
      }
    }
  } else {
    printf("adjustBaseContact: unknown base type %d\n", pBaseNode->baseType);
  }
  /* at the conclusion of this loop have the point of max density */
  if (pDevice->baseIndex ISNOT newBaseIndex) {
    /* so change the position */
    pNode = pDevice->elemArray[newBaseIndex]->pNodes[0];
    pNode->baseType = pBaseNode->baseType;
    pNode->vbe = pBaseNode->vbe;
    pBaseNode->baseType = FALSE;
    pBaseNode->vbe = 0.0;
    pDevice->baseIndex = newBaseIndex;
  }
}

void
NBJTjunctions(pDevice, indexEB, indexBC)
  ONEdevice *pDevice;
  int *indexEB, *indexBC;
{
  int index;
  double conc1, conc2;
  BOOLEAN findFirstJunction = TRUE;
  BOOLEAN notFound = TRUE;

  for (index = 1; index < pDevice->numNodes AND notFound; index++) {
    conc1 = pDevice->elemArray[index]->pNodes[0]->netConc;
    conc2 = pDevice->elemArray[index]->pNodes[1]->netConc;

    if (conc1 * conc2 < 0.0 AND findFirstJunction) {
      *indexEB = index;
      findFirstJunction = FALSE;
    } else if (conc1 * conc2 < 0.0 AND NOT findFirstJunction) {
      *indexBC = index;
      notFound = FALSE;
    }
  }

  if (notFound) {
    fprintf(stderr, "BJT: Device does not have two junctions!\n");
    exit(-1);
  }
}

int
ONEprnMesh(pDevice)
  ONEdevice *pDevice;
{
  int eIndex, index;
  ONEelem *pElem;
  ONEnode *pNode;
  ONEedge *pEdge;
  char *name;

  for (eIndex = 1; eIndex < pDevice->numNodes; eIndex++) {
    pElem = pDevice->elemArray[eIndex];
    fprintf(stderr, "elem %5d:\n", eIndex);
    for (index = 0; index <= 1; index++) {
      if (pElem->evalNodes[index]) {
	pNode = pElem->pNodes[index];
	switch (pNode->nodeType) {
	case SEMICON:
	  name = "semiconductor";
	  break;
	case INSULATOR:
	  name = "insulator";
	  break;
	case CONTACT:
	  name = "contact";
	  break;
	case SCHOTTKY:
	  name = "schottky";
	  break;
	case INTERFACE:
	  name = "interface";
	  break;
	default:
	  name = "unknown";
	  break;
	}
	fprintf(stderr, "node %5d: %s %5d\n", index, name,
	    pNode->nodeI);
      }
    }
  }
}

static void
ONEresetEvalFlag(pDevice)
  ONEdevice *pDevice;
{
  int index, eIndex;
  ONEelem *pElem;

  for (eIndex = 1; eIndex < pDevice->numNodes; eIndex++) {
    pElem = pDevice->elemArray[eIndex];
    for (index = 0; index <= 1; index++) {
      pElem->pNodes[index]->evaluated = FALSE;
    }
    pElem->pEdge->evaluated = FALSE;
  }
}
