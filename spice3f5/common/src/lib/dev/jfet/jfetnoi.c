/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Gary W. Ng
**********/

#include "spice.h"
#include <stdio.h>
#include "jfetdefs.h"
#include "cktdefs.h"
#include "fteconst.h"
#include "iferrmsg.h"
#include "noisedef.h"
#include "util.h"
#include "suffix.h"

/*
 * JFETnoise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with JFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the JFET's is summed with the variable "OnDens".
 */

extern void   NevalSrc();
extern double Nintegrate();

int
JFETnoise (mode, operation, genmodel, ckt, data, OnDens)
    int mode;
    int operation;
    GENmodel *genmodel;
    CKTcircuit *ckt;
    register Ndata *data;
    double *OnDens;
{
    JFETmodel *firstModel = (JFETmodel *) genmodel;
    register JFETmodel *model;
    register JFETinstance *inst;
    char name[N_MXVLNTH];
    double tempOnoise;
    double tempInoise;
    double noizDens[JFETNSRCS];
    double lnNdens[JFETNSRCS];
    int error;
    int i;

    /* define the names of the noise sources */

    static char *JFETnNames[JFETNSRCS] = {       /* Note that we have to keep the order */
	"_rd",              /* noise due to rd */        /* consistent with the index definitions */
	"_rs",              /* noise due to rs */        /* in JFETdefs.h */
	"_id",              /* noise due to id */
	"_1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (model=firstModel; model != NULL; model=model->JFETnextModel) {
	for (inst=model->JFETinstances; inst != NULL; inst=inst->JFETnextInstance) {
	    if (inst->JFETowner != ARCHme) continue;

	    switch (operation) {

	    case N_OPEN:

		/* see if we have to to produce a summary report */
		/* if so, name all the noise generators */

		if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
		    switch (mode) {

		    case N_DENS:
			for (i=0; i < JFETNSRCS; i++) {
			    (void)sprintf(name,"onoise_%s%s",inst->JFETname,JFETnNames[i]);


data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(GENERIC **)NULL);
				/* we've added one more plot */


			}
			break;

		    case INT_NOIZ:
			for (i=0; i < JFETNSRCS; i++) {
			    (void)sprintf(name,"onoise_total_%s%s",inst->JFETname,JFETnNames[i]);


data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(GENERIC **)NULL);
				/* we've added one more plot */


			    (void)sprintf(name,"inoise_total_%s%s",inst->JFETname,JFETnNames[i]);


data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(GENERIC **)NULL);
				/* we've added one more plot */

			}
			break;
		    }
		}
		break;

	    case N_CALC:
		switch (mode) {

		case N_DENS:
		    NevalSrc(&noizDens[JFETRDNOIZ],&lnNdens[JFETRDNOIZ],
				 ckt,THERMNOISE,inst->JFETdrainPrimeNode,inst->JFETdrainNode,
				 model->JFETdrainConduct * inst->JFETarea);

		    NevalSrc(&noizDens[JFETRSNOIZ],&lnNdens[JFETRSNOIZ],
				 ckt,THERMNOISE,inst->JFETsourcePrimeNode,
				 inst->JFETsourceNode,model->JFETsourceConduct*inst->JFETarea);

		    NevalSrc(&noizDens[JFETIDNOIZ],&lnNdens[JFETIDNOIZ],
				 ckt,THERMNOISE,inst->JFETdrainPrimeNode,
				 inst->JFETsourcePrimeNode,
				 (2.0/3.0 * FABS(*(ckt->CKTstate0 + inst->JFETgm))));

		    NevalSrc(&noizDens[JFETFLNOIZ],(double*)NULL,ckt,
				 N_GAIN,inst->JFETdrainPrimeNode,
				 inst->JFETsourcePrimeNode, (double)0.0);
		    noizDens[JFETFLNOIZ] *= model->JFETfNcoef * 
				 exp(model->JFETfNexp *
				 log(MAX(FABS(*(ckt->CKTstate0 + inst->JFETcd)),N_MINLOG))) /
				 data->freq;
		    lnNdens[JFETFLNOIZ] = 
				 log(MAX(noizDens[JFETFLNOIZ],N_MINLOG));

		    noizDens[JFETTOTNOIZ] = noizDens[JFETRDNOIZ] +
						     noizDens[JFETRSNOIZ] +
						     noizDens[JFETIDNOIZ] +
						     noizDens[JFETFLNOIZ];
		    lnNdens[JFETTOTNOIZ] = 
				 log(MAX(noizDens[JFETTOTNOIZ], N_MINLOG));

		    *OnDens += noizDens[JFETTOTNOIZ];

		    if (data->delFreq == 0.0) { 

			/* if we haven't done any previous integration, we need to */
			/* initialize our "history" variables                      */

			for (i=0; i < JFETNSRCS; i++) {
			    inst->JFETnVar[LNLSTDENS][i] = lnNdens[i];
			}

			/* clear out our integration variables if it's the first pass */

			if (data->freq == ((NOISEAN*)ckt->CKTcurJob)->NstartFreq) {
			    for (i=0; i < JFETNSRCS; i++) {
				inst->JFETnVar[OUTNOIZ][i] = 0.0;
				inst->JFETnVar[INNOIZ][i] = 0.0;
			    }
			}
		    } else {   /* data->delFreq != 0.0 (we have to integrate) */
			for (i=0; i < JFETNSRCS; i++) {
			    if (i != JFETTOTNOIZ) {
				tempOnoise = Nintegrate(noizDens[i], lnNdens[i],
				      inst->JFETnVar[LNLSTDENS][i], data);
				tempInoise = Nintegrate(noizDens[i] * data->GainSqInv ,
				      lnNdens[i] + data->lnGainInv,
				      inst->JFETnVar[LNLSTDENS][i] + data->lnGainInv,
				      data);
				inst->JFETnVar[LNLSTDENS][i] = lnNdens[i];
				data->outNoiz += tempOnoise;
				data->inNoise += tempInoise;
				if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
				    inst->JFETnVar[OUTNOIZ][i] += tempOnoise;
				    inst->JFETnVar[OUTNOIZ][JFETTOTNOIZ] += tempOnoise;
				    inst->JFETnVar[INNOIZ][i] += tempInoise;
				    inst->JFETnVar[INNOIZ][JFETTOTNOIZ] += tempInoise;
                                }
			    }
			}
		    }
		    if (data->prtSummary) {
			for (i=0; i < JFETNSRCS; i++) {     /* print a summary report */
			    data->outpVector[data->outNumber++] = noizDens[i];
			}
		    }
		    break;

		case INT_NOIZ:        /* already calculated, just output */
		    if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
			for (i=0; i < JFETNSRCS; i++) {
			    data->outpVector[data->outNumber++] = inst->JFETnVar[OUTNOIZ][i];
			    data->outpVector[data->outNumber++] = inst->JFETnVar[INNOIZ][i];
			}
		    }    /* if */
		    break;
		}    /* switch (mode) */
		break;

	    case N_CLOSE:
		return (OK);         /* do nothing, the main calling routine will close */
		break;               /* the plots */
	    }    /* switch (operation) */
	}    /* for inst */
    }    /* for model */

return(OK);
}
            

