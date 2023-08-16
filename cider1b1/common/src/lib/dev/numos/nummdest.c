/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/*
 * This routine deletes all NUMOSs from the circuit and frees all storage
 * they were using.  The current implementation has memory leaks.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "numosdef.h"
#include "suffix.h"

void
NUMOSdestroy(inModel)
  GENmodel **inModel;

{

  NUMOSmodel **model = (NUMOSmodel **) inModel;
  NUMOSmodel *mod, *nextMod;
  NUMOSinstance *inst, *nextInst;

  void TWOdestroy();

  for (mod = *model; mod;) {
    for (inst = mod->NUMOSinstances; inst;) {
      TWOdestroy(inst->NUMOSpDevice);
      nextInst = inst->NUMOSnextInstance;
      FREE(inst);
      inst = nextInst;
    }
    nextMod = mod->NUMOSnextModel;
    FREE(mod);
    mod = nextMod;
  }
  *model = NULL;
}
