/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/*
 * This routine performs truncation error calculations for NUMOSs in the
 * circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "numosdef.h"
#include "sperror.h"
#include "suffix.h"


int
NUMOStrunc(inModel, ckt, timeStep)
  GENmodel *inModel;
  register CKTcircuit *ckt;
  double *timeStep;
{
  register NUMOSmodel *model = (NUMOSmodel *) inModel;
  register NUMOSinstance *inst;
  double computeLTECoeff();
  double TWOtrunc();
  double deltaNew;
  double deltaNorm[7];
  double startTime;
  int i;

  for (i = 0; i <= ckt->CKTmaxOrder; i++) {
    deltaNorm[i] = ckt->CKTdeltaOld[i] / TNorm;
  }

  for (; model != NULL; model = model->NUMOSnextModel) {
    OneCarrier = model->NUMOSmethods->METHoneCarrier;
    model->NUMOSpInfo->order = ckt->CKTorder;
    model->NUMOSpInfo->delta = deltaNorm;
    model->NUMOSpInfo->lteCoeff = computeLTECoeff(model->NUMOSpInfo);
    for (inst = model->NUMOSinstances; inst != NULL;
	inst = inst->NUMOSnextInstance) {
      if (inst->NUMOSowner != ARCHme) continue;

      startTime = SPfrontEnd->IFseconds();
      deltaNew = TWOtrunc(inst->NUMOSpDevice, model->NUMOSpInfo,
	  ckt->CKTdelta);
      *timeStep = MIN(*timeStep, deltaNew);
      inst->NUMOSpDevice->pStats->totalTime[STAT_TRAN] +=
	  SPfrontEnd->IFseconds() - startTime;
    }
  }
  return (OK);
}
