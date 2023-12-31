/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

/*
 * This is a simple routine to dump the internal device states. It produces
 * states for .OP, .DC, & .TRAN simulations.
 */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "cktdefs.h"
#include "nbjt2def.h"
#include "suffix.h"

/* External Declarations */
extern void TWOprnSolution();
extern void TWOmemStats(), TWOcpuStats();

/* Forward Declarations */
static void NBJT2putHeader();

/* State Counter */
static int state_numOP = 0;
static int state_numDC = 0;
static int state_numTR = 0;

void
NBJT2dump(inModel, ckt)
  GENmodel *inModel;
  CKTcircuit *ckt;
{
  register NBJT2model *model = (NBJT2model *) inModel;
  register NBJT2instance *inst;
  OUTPcard *output;
  FILE *fpState;
  char fileName[BSIZE_SP];
  char description[BSIZE_SP];
  char *prefix;
  int *state_num;
  int anyOutput = 0;

  if (ckt->CKTmode & MODEDCOP) {
    prefix = "OP";
    state_num = &state_numOP;
    sprintf(description, "...");
  } else if (ckt->CKTmode & MODEDCTRANCURVE) {
    prefix = "DC";
    state_num = &state_numDC;
    sprintf(description, "sweep = % e", ckt->CKTtime);
  } else if (ckt->CKTmode & MODETRAN) {
    prefix = "TR";
    state_num = &state_numTR;
    sprintf(description, "time = % e", ckt->CKTtime);
  } else {
    /* Not a recognized CKT mode. */
    return;
  }

  for (; model != NULL; model = model->NBJT2nextModel) {
    output = model->NBJT2outputs;
    for (inst = model->NBJT2instances; inst != NULL;
	inst = inst->NBJT2nextInstance) {
      if (inst->NBJT2owner != ARCHme) continue;

      if (inst->NBJT2printGiven) {
	if ((ckt->CKTmode & MODETRAN) &&
	    ((ckt->CKTstat->STATaccepted - 1) % inst->NBJT2print != 0)) {
	  continue;
	}
	anyOutput = 1;
	sprintf(fileName, "%s%s.%d.%s", output->OUTProotFile, prefix,
	    *state_num, inst->NBJT2name);
	if (!(fpState = fopen(fileName, "w"))) {
	  perror(fileName);
	} else {
	  NBJT2putHeader(fpState, ckt, inst);
	  TWOprnSolution(fpState, inst->NBJT2pDevice,
	      model->NBJT2outputs);
	  fclose(fpState);
	  LOGmakeEntry(fileName, description);
	}
      }
    }
  }
  if (anyOutput) {
    (*state_num)++;
  }
}

#define NBJT2numOutputs 9

static
void 
NBJT2putHeader(file, ckt, inst)
  FILE *file;
  CKTcircuit *ckt;
  NBJT2instance *inst;
{
  char *reference;
  double refVal;
  int numVars = NBJT2numOutputs;

  if (ckt->CKTmode & MODEDCOP) {
    reference = NULL;
  } else if (ckt->CKTmode & MODEDCTRANCURVE) {
    reference = "sweep";
    refVal = ckt->CKTtime;
    numVars++;
  } else if (ckt->CKTmode & MODETRAN) {
    reference = "time";
    refVal = ckt->CKTtime;
    numVars++;
  } else {
    reference = NULL;
  }
  fprintf(file, "Title: Device %s external state\n", inst->NBJT2name);
  fprintf(file, "Plotname: Device Operating Point\n");
  fprintf(file, "Command: deftype v conductance S\n");
  fprintf(file, "Flags: real\n");
  fprintf(file, "No. Variables: %d\n", numVars);
  fprintf(file, "No. Points: 1\n");
  numVars = 0;
  fprintf(file, "Variables:\n");
  if (reference) {
    fprintf(file, "\t%d	%s	unknown\n", numVars++, reference);
  }
  fprintf(file, "\t%d	v13	voltage\n", numVars++);
  fprintf(file, "\t%d	v23	voltage\n", numVars++);
  fprintf(file, "\t%d	i1	current\n", numVars++);
  fprintf(file, "\t%d	i2	current\n", numVars++);
  fprintf(file, "\t%d	i3	current\n", numVars++);
  fprintf(file, "\t%d	g22	conductance\n", numVars++);
  fprintf(file, "\t%d	g21	conductance\n", numVars++);
  fprintf(file, "\t%d	g12	conductance\n", numVars++);
  fprintf(file, "\t%d	g11	conductance\n", numVars++);
  fprintf(file, "Values:\n0");
  if (reference) {
    fprintf(file, "\t% e\n", refVal);
  }
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2vce));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2vbe));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2ic));
  fprintf(file, "\t% e\n", - *(ckt->CKTstate0 + inst->NBJT2ie)
      - *(ckt->CKTstate0 + inst->NBJT2ic));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2ie));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2dIeDVbe)
      - *(ckt->CKTstate0 + inst->NBJT2dIcDVbe));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2dIeDVce)
      - *(ckt->CKTstate0 + inst->NBJT2dIcDVce));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2dIcDVbe));
  fprintf(file, "\t% e\n", *(ckt->CKTstate0 + inst->NBJT2dIcDVce));
}

int
NBJT2acct(inModel, ckt, file)
  GENmodel *inModel;
  CKTcircuit *ckt;
  FILE *file;
{
  register NBJT2model *model = (NBJT2model *) inModel;
  register NBJT2instance *inst;
  OUTPcard *output;

  for (; model != NULL; model = model->NBJT2nextModel) {
    output = model->NBJT2outputs;
    for (inst = model->NBJT2instances; inst != NULL;
	inst = inst->NBJT2nextInstance) {
      if (inst->NBJT2owner != ARCHme) continue;

      if (output->OUTPstats) {
	TWOmemStats(file, inst->NBJT2pDevice);
	TWOcpuStats(file, inst->NBJT2pDevice);
      }
    }
  }
}
