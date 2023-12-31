/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/*
 * This is the function called each iteration to evaluate the numerical BJTs
 * in the circuit and load them into the matrix as appropriate
 */

#include "spice.h"
#include <stdio.h>
#include <math.h>
#include "util.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "nbjtdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"

/* External Declarations */
extern int ONEdcDebug;
extern int ONEtranDebug;
extern int ONEacDebug;

extern double limitVbe(), limitVce();
extern void computeIntegCoeff(), computePredCoeff();
extern void NBJTconductance(), NBJTcurrent(), NBJTproject(), NBJTsetBCs();
extern void NBJTupdate();
extern void ONEequilSolve(), ONEbiasSolve();
extern void ONEpredict(), ONEsaveState();
extern void ONEresetJacobian(), ONEstoreInitialGuess();
extern int ONEdeviceConverged(), ONEreadState();

int
NBJTload(inModel, ckt)
  GENmodel *inModel;
  CKTcircuit *ckt;
{
  register NBJTmodel *model = (NBJTmodel *) inModel;
  register NBJTinstance *inst;
  register ONEdevice *pDevice;
  double startTime, startTime2, totalTime, totalTime2;
  double tol;
  double ic, ie;
  double iceq, ieeq;
  double ichat, iehat;
  double delVce, delVbe;
  double vce, vbe, vbc;
  double dIeDVce, dIeDVbe;
  double dIcDVce, dIcDVbe;
  double xfact;
  int icheck;
  int icheck1;
  int i;
  double deltaNorm[7];
  int devConverged;
  int numDevNonCon;
  int deviceType;
  int doInitSolve;
  int doVoltPred;
  char *initStateName;

  /* loop through all the models */
  for (; model != NULL; model = model->NBJTnextModel) {
    FieldDepMobility = model->NBJTmodels->MODLfieldDepMobility;
    Srh = model->NBJTmodels->MODLsrh;
    Auger = model->NBJTmodels->MODLauger;
    AvalancheGen = model->NBJTmodels->MODLavalancheGen;
    MobDeriv = model->NBJTmethods->METHmobDeriv;
    MaxIterations = model->NBJTmethods->METHitLim;
    ONEdcDebug = model->NBJToutputs->OUTPdcDebug;
    ONEtranDebug = model->NBJToutputs->OUTPtranDebug;
    ONEacDebug = model->NBJToutputs->OUTPacDebug;
    deviceType = model->NBJToptions->OPTNdeviceType;
    doVoltPred = model->NBJTmethods->METHvoltPred;

    if (ckt->CKTmode & MODEINITPRED) {
      /* compute normalized deltas and predictor coeff */
      if (!(ckt->CKTmode & MODEDCTRANCURVE)) {
	model->NBJTpInfo->order = ckt->CKTorder;
	model->NBJTpInfo->method = ckt->CKTintegrateMethod;
	for (i = 0; i <= ckt->CKTmaxOrder; i++) {
	  deltaNorm[i] = ckt->CKTdeltaOld[i] / TNorm;
	}
	computeIntegCoeff(ckt->CKTintegrateMethod, ckt->CKTorder,
	    model->NBJTpInfo->intCoeff, deltaNorm);
	computePredCoeff(ckt->CKTintegrateMethod, ckt->CKTorder,
	    model->NBJTpInfo->predCoeff, deltaNorm);
      }
    } else if (ckt->CKTmode & MODEINITTRAN) {
      model->NBJTpInfo->order = ckt->CKTorder;
      model->NBJTpInfo->method = ckt->CKTintegrateMethod;
      for (i = 0; i <= ckt->CKTmaxOrder; i++) {
	deltaNorm[i] = ckt->CKTdeltaOld[i] / TNorm;
      }
      computeIntegCoeff(ckt->CKTintegrateMethod, ckt->CKTorder,
	  model->NBJTpInfo->intCoeff, deltaNorm);
    }
    /* loop through all the instances of the model */
    for (inst = model->NBJTinstances; inst != NULL;
	inst = inst->NBJTnextInstance) {
      if (inst->NBJTowner != ARCHme) continue;

      pDevice = inst->NBJTpDevice;

      totalTime = 0.0;
      startTime = SPfrontEnd->IFseconds();

      /* Get Temp.-Dep. Global Parameters */
      GLOBgetGlobals(&(inst->NBJTglobals));

      /*
       * initialization
       */

      pDevice->devStates = ckt->CKTstates;
      icheck = 1;
      doInitSolve = FALSE;
      initStateName = NULL;
      if (ckt->CKTmode & MODEINITSMSIG) {
	vbe = *(ckt->CKTstate0 + inst->NBJTvbe);
	vce = *(ckt->CKTstate0 + inst->NBJTvce);
	delVbe = 0.0;
	delVce = 0.0;
	NBJTsetBCs(pDevice, vce, vbe);
      } else if (ckt->CKTmode & MODEINITTRAN) {
	*(ckt->CKTstate0 + inst->NBJTvbe) =
	    *(ckt->CKTstate1 + inst->NBJTvbe);
	*(ckt->CKTstate0 + inst->NBJTvce) =
	    *(ckt->CKTstate1 + inst->NBJTvce);
	vbe = *(ckt->CKTstate1 + inst->NBJTvbe);
	vce = *(ckt->CKTstate1 + inst->NBJTvce);
	ONEsaveState(pDevice);
	delVbe = 0.0;
	delVce = 0.0;
      } else if ((ckt->CKTmode & MODEINITJCT) &&
	  (ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)) {
	doInitSolve = TRUE;
	initStateName = inst->NBJTicFile;
	vbe = 0.0;
	vce = 0.0;
	delVbe = vbe;
	delVce = vce;
      } else if ((ckt->CKTmode & MODEINITJCT) && (inst->NBJToff == 0)) {
	doInitSolve = TRUE;
	initStateName = inst->NBJTicFile;
	vbe = inst->NBJTtype * 0.6;
	vce = inst->NBJTtype * 1.0;
	delVbe = vbe;
	delVce = vce;
      } else if (ckt->CKTmode & MODEINITJCT) {
	doInitSolve = TRUE;
	vbe = 0.0;
	vce = 0.0;
	delVbe = vbe;
	delVce = vce;
      } else if ((ckt->CKTmode & MODEINITFIX) && inst->NBJToff) {
	vbe = 0.0;
	vce = 0.0;
	delVbe = vbe;
	delVce = vce;
      } else {
	if (ckt->CKTmode & MODEINITPRED) {
	  *(ckt->CKTstate0 + inst->NBJTvbe) =
	      *(ckt->CKTstate1 + inst->NBJTvbe);
	  *(ckt->CKTstate0 + inst->NBJTvce) =
	      *(ckt->CKTstate1 + inst->NBJTvce);
	  *(ckt->CKTstate0 + inst->NBJTic) =
	      *(ckt->CKTstate1 + inst->NBJTic);
	  *(ckt->CKTstate0 + inst->NBJTie) =
	      *(ckt->CKTstate1 + inst->NBJTie);
	  *(ckt->CKTstate0 + inst->NBJTdIeDVce) =
	      *(ckt->CKTstate1 + inst->NBJTdIeDVce);
	  *(ckt->CKTstate0 + inst->NBJTdIeDVbe) =
	      *(ckt->CKTstate1 + inst->NBJTdIeDVbe);
	  *(ckt->CKTstate0 + inst->NBJTdIcDVce) =
	      *(ckt->CKTstate1 + inst->NBJTdIcDVce);
	  *(ckt->CKTstate0 + inst->NBJTdIcDVbe) =
	      *(ckt->CKTstate1 + inst->NBJTdIcDVbe);
	  if (!(ckt->CKTmode & MODEDCTRANCURVE)) {
	    /* no linear prediction on device voltages */
	    vbe = *(ckt->CKTstate1 + inst->NBJTvbe);
	    vce = *(ckt->CKTstate1 + inst->NBJTvce);
	    ONEpredict(pDevice, model->NBJTpInfo);
	  } else {
            if (doVoltPred) {
	      /* linear prediction */
	      xfact=ckt->CKTdelta/ckt->CKTdeltaOld[1];
	      vbe = (1+xfact) * (*(ckt->CKTstate1 + inst->NBJTvbe))
		  -   (xfact) * (*(ckt->CKTstate2 + inst->NBJTvbe));
	      vce = (1+xfact) * (*(ckt->CKTstate1 + inst->NBJTvce))
		  -   (xfact) * (*(ckt->CKTstate2 + inst->NBJTvce));
	    } else {
	      vbe = *(ckt->CKTstate1 + inst->NBJTvbe);
	      vce = *(ckt->CKTstate1 + inst->NBJTvce);
	    }
	  }
	} else {
	  /*
	   * compute new nonlinear branch voltages
	   */
	  vbe = *(ckt->CKTrhsOld + inst->NBJTbaseNode) -
	      *(ckt->CKTrhsOld + inst->NBJTemitNode);
	  vce = *(ckt->CKTrhsOld + inst->NBJTcolNode) -
	      *(ckt->CKTrhsOld + inst->NBJTemitNode);
	}
	delVbe = vbe - *(ckt->CKTstate0 + inst->NBJTvbe);
	delVce = vce - *(ckt->CKTstate0 + inst->NBJTvce);
	ichat = *(ckt->CKTstate0 + inst->NBJTic) -
	    *(ckt->CKTstate0 + inst->NBJTdIcDVbe) * delVbe -
	    *(ckt->CKTstate0 + inst->NBJTdIcDVce) * delVce;
	iehat = *(ckt->CKTstate0 + inst->NBJTie) -
	    *(ckt->CKTstate0 + inst->NBJTdIeDVbe) * delVbe -
	    *(ckt->CKTstate0 + inst->NBJTdIeDVce) * delVce;

#ifndef NOBYPASS
	/*
	 * bypass if solution has not changed
	 */
	/*
	 * the following collections of if's would be just one if the average
	 * compiler could handle it, but many find the expression too
	 * complicated, thus the split.
	 */
	if ((ckt->CKTbypass) && pDevice->converged &&
	    (!(ckt->CKTmode & MODEINITPRED)) &&
	    (FABS(delVbe) < (ckt->CKTreltol * MAX(FABS(vbe),
			FABS(*(ckt->CKTstate0 + inst->NBJTvbe))) +
		    ckt->CKTvoltTol)))
	  if ((FABS(delVce) < ckt->CKTreltol * MAX(FABS(vce),
		      FABS(*(ckt->CKTstate0 + inst->NBJTvce))) +
		  ckt->CKTvoltTol))
	    if ((FABS(ichat - *(ckt->CKTstate0 + inst->NBJTic)) <
		    ckt->CKTreltol * MAX(FABS(ichat),
			FABS(*(ckt->CKTstate0 + inst->NBJTic))) +
		    ckt->CKTabstol))
	      if ((FABS(iehat - *(ckt->CKTstate0 + inst->NBJTie)) <
		      ckt->CKTreltol * MAX(FABS(iehat),
			  FABS(*(ckt->CKTstate0 + inst->NBJTie))) +
		      ckt->CKTabstol)) {
		/*
		 * bypassing....
		 */
		vbe = *(ckt->CKTstate0 + inst->NBJTvbe);
		vce = *(ckt->CKTstate0 + inst->NBJTvce);
		ic = *(ckt->CKTstate0 + inst->NBJTic);
		ie = *(ckt->CKTstate0 + inst->NBJTie);
		dIeDVce = *(ckt->CKTstate0 + inst->NBJTdIeDVce);
		dIeDVbe = *(ckt->CKTstate0 + inst->NBJTdIeDVbe);
		dIcDVce = *(ckt->CKTstate0 + inst->NBJTdIcDVce);
		dIcDVbe = *(ckt->CKTstate0 + inst->NBJTdIcDVbe);
		goto load;
	      }
#endif				/* NOBYPASS */
	/*
	 * limit nonlinear branch voltages
	 */
	icheck1 = 1;
	vbe = inst->NBJTtype * limitVbe(inst->NBJTtype * vbe,
	    inst->NBJTtype * *(ckt->CKTstate0 + inst->NBJTvbe), &icheck);
	/*
	vbc = vbe - vce;
	vbc = inst->NBJTtype * limitVbe(inst->NBJTtype * vbc,
	    inst->NBJTtype * (*(ckt->CKTstate0 + inst->NBJTvbe)
	    - *(ckt->CKTstate0 + inst->NBJTvce)), &icheck1);
	vce = vbe - vbc;
	    */
	vce = inst->NBJTtype * limitVce(inst->NBJTtype * vce,
	    inst->NBJTtype * *(ckt->CKTstate0 + inst->NBJTvce), &icheck1);
	if (icheck1 == 1)
	  icheck = 1;
	delVbe = vbe - *(ckt->CKTstate0 + inst->NBJTvbe);
	delVce = vce - *(ckt->CKTstate0 + inst->NBJTvce);
      }
      if (doInitSolve) {
	if (ONEdcDebug) {
	  printVoltages(stdout,
	      model->NBJTmodName, inst->NBJTname,
	      deviceType, 2, 0.0, 0.0, 0.0, 0.0);
	}
	startTime2 = SPfrontEnd->IFseconds();
	ONEequilSolve(pDevice);
	totalTime2 = SPfrontEnd->IFseconds() - startTime2;
	pDevice->pStats->totalTime[STAT_SETUP] += totalTime2;
	pDevice->pStats->totalTime[STAT_DC] -= totalTime2;

	ONEbiasSolve(pDevice, MaxIterations, FALSE, NULL);

	*(ckt->CKTstate0 + inst->NBJTvbe) = 0.0;
	*(ckt->CKTstate0 + inst->NBJTvce) = 0.0;

	if (initStateName != NULL) {
	  if (ONEreadState(pDevice, initStateName, 2, &vce, &vbe ) < 0) {
	    fprintf(stderr,
		"NBJTload: trouble reading state-file %s\n", initStateName);
	  } else {
	    NBJTsetBCs(pDevice, vce, vbe);
	    delVce = delVbe = 0.0;
	  }
	}
      }
      /*
       * determine dc current and derivatives using the numerical routines
       */
      if (ckt->CKTmode & (MODEDCOP | MODETRANOP | MODEDCTRANCURVE | MODEINITSMSIG)) {

	numDevNonCon = 0;
	inst->NBJTc11 = inst->NBJTy11r = inst->NBJTy11i = 0.0;
	inst->NBJTc12 = inst->NBJTy12r = inst->NBJTy12i = 0.0;
	inst->NBJTc21 = inst->NBJTy21r = inst->NBJTy21i = 0.0;
	inst->NBJTc22 = inst->NBJTy22r = inst->NBJTy22i = 0.0;
	inst->NBJTsmSigAvail = FALSE;
    devNonCon:
	NBJTproject(pDevice, delVce, delVbe, vbe);
	if (ONEdcDebug) {
	  printVoltages(stdout,
	      model->NBJTmodName, inst->NBJTname,
	      deviceType, 2, vce, delVce, vbe, delVbe);
	}
	ONEbiasSolve(pDevice, MaxIterations, FALSE, model->NBJTpInfo);

	devConverged = pDevice->converged;
	if (devConverged && finite(pDevice->rhsNorm)) {
	  /* compute the currents */
	  NBJTcurrent(pDevice, FALSE, (double *) NULL,
	      &ie, &ic);
	  NBJTconductance(pDevice, FALSE, (double *) NULL,
	      &dIeDVce, &dIcDVce, &dIeDVbe, &dIcDVbe);
	  /*
	   * Add Gmin terms to everything in case we are operating at
	   * abnormally low current levels
	   */
	  ie += ckt->CKTgmin * (vce + vbe);
	  dIeDVce += ckt->CKTgmin;
	  dIeDVbe += ckt->CKTgmin;
	  ic += ckt->CKTgmin * (2.0 * vce - vbe);
	  dIcDVce += 2.0 * ckt->CKTgmin;
	  dIcDVbe -= ckt->CKTgmin;

	} else {
	  /* reduce the voltage step until converged */
	  /* restore the boundary potential to previous value */
	  NBJTsetBCs(pDevice, vce - delVce, vbe - delVbe);
	  ONEstoreInitialGuess(pDevice);
	  ONEresetJacobian(pDevice);
	  delVbe *= 0.5;
	  delVce *= 0.5;
	  vbe = delVbe + *(ckt->CKTstate0 + inst->NBJTvbe);
	  vce = delVce + *(ckt->CKTstate0 + inst->NBJTvce);
	  numDevNonCon++;
	  icheck = 1;
	  if (numDevNonCon > 10) {
	    printVoltages(stderr,
		model->NBJTmodName, inst->NBJTname,
		deviceType, 2, vce, delVce, vbe, delVbe);
	    fprintf(stderr,
		"*** Non-convergence during load ***\n");
	    totalTime += SPfrontEnd->IFseconds() - startTime;
	    pDevice->pStats->totalTime[STAT_DC] += totalTime;
	    ckt->CKTtroubleElt = (GENinstance *) inst;
	    return (E_BADMATRIX);
	  } else {
	    goto devNonCon;
	  }
	}

      }
      if ((ckt->CKTmode & (MODETRAN | MODEAC)) ||
	  ((ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)) ||
	  (ckt->CKTmode & MODEINITSMSIG)) {
	/*
	 * store small-signal parameters
	 */
	if ((!(ckt->CKTmode & MODETRANOP)) ||
	    (!(ckt->CKTmode & MODEUIC))) {
	  if (ckt->CKTmode & MODEINITSMSIG) {
	    totalTime += SPfrontEnd->IFseconds() - startTime;
	    pDevice->pStats->totalTime[STAT_DC] += totalTime;
	    startTime2 = SPfrontEnd->IFseconds();
	    NBJTinitSmSig(inst);
	    pDevice->pStats->totalTime[STAT_AC] +=
		SPfrontEnd->IFseconds() - startTime2;
	    continue;
	  } else {
	    inst->NBJTsmSigAvail = FALSE;
	  }
	  /*
	   * transient analysis
	   */
	  if (ckt->CKTmode & MODEINITPRED) {
	    NBJTsetBCs(pDevice, vce, vbe);
	    ONEstoreInitialGuess(pDevice);
	  } else {
	    NBJTupdate(pDevice, delVce, delVbe, vbe, TRUE);
	  }
	  if (ONEtranDebug) {
	    printVoltages(stdout,
		model->NBJTmodName, inst->NBJTname,
		deviceType, 2, vce, delVce, vbe, delVbe);
	  }
	  ONEbiasSolve(pDevice, 0, TRUE, model->NBJTpInfo);
	  if (!finite(pDevice->rhsNorm)) {
	    /* Timestep took us to never-never land. */
	    totalTime += SPfrontEnd->IFseconds() - startTime;
	    pDevice->pStats->totalTime[STAT_TRAN] += totalTime;
	    ckt->CKTtroubleElt = (GENinstance *) inst;
	    return (E_BADMATRIX);
	  }
	  devConverged = ONEdeviceConverged(pDevice);
	  pDevice->converged = devConverged;

	  /* compute the currents */
	  NBJTcurrent(pDevice, TRUE,
	      model->NBJTpInfo->intCoeff, &ie, &ic);
	  NBJTconductance(pDevice, TRUE,
	      model->NBJTpInfo->intCoeff,
	      &dIeDVce, &dIcDVce, &dIeDVbe, &dIcDVbe);

	  /*
	   * Add Gmin terms to everything in case we are operating at
	   * abnormally low current levels
	   */
	  ie += ckt->CKTgmin * (vce + vbe);
	  dIeDVce += ckt->CKTgmin;
	  dIeDVbe += ckt->CKTgmin;
	  ic += ckt->CKTgmin * (2.0 * vce - vbe);
	  dIcDVce += 2.0 * ckt->CKTgmin;
	  dIcDVbe -= ckt->CKTgmin;
	}
      }
      /*
       * check convergence
       */
      if ((!(ckt->CKTmode & MODEINITFIX)) || (!(inst->NBJToff))) {
	if (icheck == 1 || !devConverged) {
	  ckt->CKTnoncon++;
	  ckt->CKTtroubleElt = (GENinstance *) inst;
	} else {
	  tol = ckt->CKTreltol * MAX(FABS(ichat), FABS(ic)) + ckt->CKTabstol;
	  if (FABS(ichat - ic) > tol) {
	    ckt->CKTnoncon++;
	    ckt->CKTtroubleElt = (GENinstance *) inst;
	  } else {
	    tol = ckt->CKTreltol * MAX(FABS(iehat), FABS(ie)) +
		ckt->CKTabstol;
	    if (FABS(iehat - ie) > tol) {
	      ckt->CKTnoncon++;
	      ckt->CKTtroubleElt = (GENinstance *) inst;
	    }
	  }
	}
      }
      *(ckt->CKTstate0 + inst->NBJTvbe) = vbe;
      *(ckt->CKTstate0 + inst->NBJTvce) = vce;
      *(ckt->CKTstate0 + inst->NBJTic) = ic;
      *(ckt->CKTstate0 + inst->NBJTie) = ie;
      *(ckt->CKTstate0 + inst->NBJTdIeDVce) = dIeDVce;
      *(ckt->CKTstate0 + inst->NBJTdIeDVbe) = dIeDVbe;
      *(ckt->CKTstate0 + inst->NBJTdIcDVce) = dIcDVce;
      *(ckt->CKTstate0 + inst->NBJTdIcDVbe) = dIcDVbe;

  load:
      /*
       * load current excitation vector
       */
      iceq = ic - dIcDVce * vce - dIcDVbe * vbe;
      ieeq = ie - dIeDVce * vce - dIeDVbe * vbe;
      *(ckt->CKTrhs + inst->NBJTcolNode) -= iceq;
      *(ckt->CKTrhs + inst->NBJTbaseNode) -= ieeq - iceq;
      *(ckt->CKTrhs + inst->NBJTemitNode) += ieeq;

      /*
       * load y matrix
       */
      *(inst->NBJTcolColPtr) += dIcDVce;
      *(inst->NBJTcolBasePtr) += dIcDVbe;
      *(inst->NBJTcolEmitPtr) -= dIcDVbe + dIcDVce;
      *(inst->NBJTbaseColPtr) -= dIcDVce - dIeDVce;
      *(inst->NBJTbaseBasePtr) -= dIcDVbe - dIeDVbe;
      *(inst->NBJTbaseEmitPtr) += dIcDVbe + dIcDVce - dIeDVbe - dIeDVce;
      *(inst->NBJTemitColPtr) -= dIeDVce;
      *(inst->NBJTemitBasePtr) -= dIeDVbe;
      *(inst->NBJTemitEmitPtr) += dIeDVbe + dIeDVce;

      totalTime += SPfrontEnd->IFseconds() - startTime;
      if (ckt->CKTmode & MODETRAN) {
	pDevice->pStats->totalTime[STAT_TRAN] += totalTime;
      } else {
	pDevice->pStats->totalTime[STAT_DC] += totalTime;
      }
    }
  }
  return (OK);
}

int
NBJTinitSmSig(inst)
  NBJTinstance *inst;
{
  SPcomplex yIeVce, yIeVbe;
  SPcomplex yIcVce, yIcVbe;
  double omega = inst->NBJTmodPtr->NBJTmethods->METHomega;

  AcAnalysisMethod = SOR_ONLY;
  (void) NBJTadmittance(inst->NBJTpDevice, omega,
      &yIeVce, &yIcVce, &yIeVbe, &yIcVbe);
  inst->NBJTc11 = yIcVce.imag / omega;
  inst->NBJTc12 = yIcVbe.imag / omega;
  inst->NBJTc21 = (yIeVce.imag - yIcVce.imag) / omega;
  inst->NBJTc22 = (yIeVbe.imag - yIcVbe.imag) / omega;
  inst->NBJTy11r = yIcVce.real;
  inst->NBJTy11i = yIcVce.imag;
  inst->NBJTy12r = yIcVbe.real;
  inst->NBJTy12i = yIcVbe.imag;
  inst->NBJTy21r = yIeVce.real - yIcVce.real;
  inst->NBJTy21i = yIeVce.imag - yIcVce.imag;
  inst->NBJTy22r = yIeVbe.real - yIcVbe.real;
  inst->NBJTy22i = yIeVbe.imag - yIcVbe.imag;
  inst->NBJTsmSigAvail = TRUE;
  return (OK);
}
