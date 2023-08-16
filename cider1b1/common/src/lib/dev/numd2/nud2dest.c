/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/*
 * This routine deletes all NUMD2s from the circuit and frees all storage
 * they were using.  The current implementation has memory leaks.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "numd2def.h"
#include "suffix.h"

void
NUMD2destroy(inModel)
  GENmodel **inModel;
{

  NUMD2model **model = (NUMD2model **) inModel;
  NUMD2model *mod, *nextMod;
  NUMD2instance *inst, *nextInst;

  void TWOdestroy();

  for (mod = *model; mod;) {
    for (inst = mod->NUMD2instances; inst;) {
      TWOdestroy(inst->NUMD2pDevice);
      nextInst = inst->NUMD2nextInstance;
      FREE(inst);
      inst = nextInst;
    }
    nextMod = mod->NUMD2nextModel;
    FREE(mod);
    mod = nextMod;
  }
  *model = NULL;
}
