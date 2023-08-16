/* Data Distributor - Assign Tasks to Groups that make up a Parallel Machine
 *     In many ways, this an experimental piece of code. Caveat Emptor.
 */
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include "tasks.h"
#include "groups.h"

#define STEPS_PER_TEMP 100
#define TEMPFACTOR 0.95
#define INITTEMP  500.0
#define MINTEMP   1.0E-6

/* IPSC */
#define MACHINE_SPEED   2.0
#define MACHINE_MEMORY  32.0e6
#define MACHINE_DISK    250e6
#define MACHINE_IOBW    1e6
/* DS5000/125 */
/*
#define MACHINE_SPEED   1.0
#define MACHINE_MEMORY  16.0e6
#define MACHINE_DISK    250e6
#define MACHINE_IOBW    1e6
*/

Task *tasks;
int NumTasks;

Group *groups;
int NumGroups;

GroupLevel *levels;
int NumLevels;

int ArchSize;
int TaskSize;
int NumPasses;
int *mapping;
double **costs;
double *sizes;
double *memory;
double Average;
double MaxEfficiency;

double MachineMflops = MACHINE_SPEED;
double MachineMbytes = MACHINE_MEMORY;

double TaskTime();
double TaskMemory();
double PartCost();

main( argc, argv )
int argc;
char *argv[];
{
  int i,j;
  int limitNode, group, groupSize, NumSizes;
  double taskTime,   taskMemory;
  double totalTime, totalMemory, totalSize, newCost;
  double savedTime, savedMemory, savedSize, savedCost;
  double maxTime,     maxMemory,   maxSize;
  double levMaxTime, levTotalTime;
  double levSavedTime[50];
  char *groupFileName;
  int tasksRead = 0;

  switch (argc) {
    case 5:
      NumPasses = atoi(argv[1]);
      groupFileName = argv[2];
      ReadTasks(argv[3], &NumTasks, &tasks );
      tasksRead = 1;
      MaxEfficiency = atof(argv[4]);
      break;
    case 6:
      NumPasses = atoi(argv[1]);
      ReadMachine(argv[2], &MachineMflops, &MachineMbytes );
      groupFileName = argv[3];
      ReadTasks(argv[4], &NumTasks, &tasks );
      tasksRead = 1;
      MaxEfficiency = atof(argv[5]);
      break;
    case 8:
      NumPasses = atoi(argv[1]);
      srandom(atoi(argv[2]));
      groupFileName = argv[3];
      NumTasks = atoi(argv[4]);
      TaskSize = atoi(argv[5]);
      NumSizes = atoi(argv[6]);
      MaxEfficiency = atof(argv[7]);
      break;
    default:
      fprintf( stderr, "Usage: %s numpasses [machinefile] groupfile taskfile efficiency\n", argv[0] );
      exit(1);
      break;
  }

  /* Read the group file. */
  ReadGroups( groupFileName, &ArchSize,
      &groups, &NumGroups, &levels, &NumLevels );

  costs = (double **)calloc( (unsigned)NumLevels, sizeof(double *) );
  for ( i=0; i < NumLevels; i++ ) {
    costs[i] = (double *)calloc( (unsigned)ArchSize, sizeof(double) );
  }
  memory = (double *)calloc( (unsigned)ArchSize, sizeof(double) );
  sizes = (double *)calloc( (unsigned)ArchSize, sizeof(double) );
  if (!tasksRead) {
    tasks = (Task *)calloc( (unsigned)NumTasks, sizeof(Task) );
  }
  mapping = (int *)calloc( (unsigned)NumTasks, sizeof(int) );

  fprintf( stdout, "***** Initial Distribution *****\n" );
  for ( i=0; i < NumTasks; i++ ) {
    if (!tasksRead) {
      tasks[i].type = NUMERTWO;
      switch( tasks[i].type ) {
      case NUMERTWO:
	tasks[i].size = TaskSize * (random()%NumSizes+1);
	break;
      case COMPACT:
      case SIMPLE:
	tasks[i].size = 1;
	break;
      }
    }
    tasks[i].group = random() % (levels[0].size);
    mapping[i] = tasks[i].group;
    printf( "Task[%4d]: type %d, time %5gS, mem %5gMB, size %5d, group %s\n",
       i, tasks[i].type,
       TaskTime(tasks[i], tasks[i].group)/1.0e6,
       TaskMemory(tasks[i], tasks[i].group)/1.0e6,
       tasks[i].size,
       GroupString(&(groups[tasks[i].group])));
  }

  Average = 0.0;
  for ( i=0; i < NumTasks; i++ ) {
    group = tasks[i].group;
    groupSize = groups[group].size;
    taskTime = TaskTime( tasks[i], group );
    taskMemory = TaskMemory( tasks[i], group );
    for ( j = 0; j < groupSize; j++ ) {
      costs[groups[group].level][groups[group].members[j]]
	+= taskTime / groupSize;
      memory[groups[group].members[j]]
	+= taskMemory / groupSize;
      sizes[groups[group].members[j]] += 1.0 / groupSize;
    }
    Average += taskTime;
  }

  maxTime = totalTime = 0.0;
  maxMemory = totalMemory = 0.0;
  maxSize = totalSize = 0.0;
  for ( i=0; i < NumLevels; i++ ) {
    levMaxTime = 0.0;
    levTotalTime = 0.0;
    limitNode = 0;
    for ( j = 0; j < ArchSize; j++ ) {
      if ( costs[i][j] > levMaxTime ) {
	levMaxTime = costs[i][j];
	limitNode = j;
      }
      levTotalTime += costs[i][j];
    }
    maxTime += levMaxTime;
    totalTime += levTotalTime;
    levSavedTime[i] = levTotalTime;
  }
  for ( i=0; i < ArchSize; i++ ) {
    if ( sizes[i] > maxSize ) maxSize = sizes[i];
    totalSize += sizes[i];
    totalMemory += memory[i];
    fprintf( stdout, "CPU[ %d ]: %11g S, %11g MB, %g size\n",
      i, costs[0][i]/1e6, memory[i]/1e6, sizes[i] );
  }
  fprintf( stdout, "TOTALS: %11g S, %11g MB, %g size\n",
    totalTime/1e6, totalMemory/1e6, totalSize );
  savedTime = totalTime;
  savedSize = totalSize;
  savedMemory = totalMemory;
  savedCost = PartCost();

  for ( i=0; i < NumPasses; i++ ) {
    srandom( i+1 );
    Anneal();
    newCost = PartCost();
    if (newCost < savedCost) {
      for ( j=0; j < NumTasks; j++ ) {
	mapping[j] = tasks[j].group;
      }
      savedCost = newCost;
    }
  }
  for ( i=0; i < NumLevels; i++ ) {
    bzero( costs[i], (unsigned)ArchSize*sizeof(double) );
  }
  bzero( memory, (unsigned)ArchSize*sizeof(double) );
  bzero(  sizes, (unsigned)ArchSize*sizeof(double) );

  for ( i=0; i < NumTasks; i++ ) {
    group = tasks[i].group = mapping[i];
    groupSize = groups[group].size;
    taskTime = TaskTime( tasks[i], group );
    taskMemory = TaskMemory( tasks[i], group );
    for ( j = 0; j < groupSize; j++ ) {
      costs[groups[group].level][groups[group].members[j]]
	+= taskTime / groupSize;
      memory[groups[group].members[j]]
	+= taskMemory / groupSize;
      sizes[groups[group].members[j]] += 1.0 / groupSize;
    }
  }

  maxTime = totalTime = 0.0;
  maxMemory = totalMemory = 0.0;
  maxSize = totalSize = 0.0;
  for ( i=0; i < NumLevels; i++ ) {
    levMaxTime = levTotalTime = 0.0;
    for ( j = 0; j < ArchSize; j++ ) {
      if ( costs[i][j] > levMaxTime ) {
	levMaxTime = costs[i][j];
	limitNode = j;
      }
      levTotalTime += costs[i][j];
    }
    maxTime += levMaxTime;
    totalTime += levTotalTime;
  }
  fprintf( stdout, "***** Results *****\n" );
  for ( j=0; j < ArchSize; j++ ) {
    if ( sizes[j] > maxSize ) maxSize = sizes[j];
    if ( memory[j] > maxMemory ) maxMemory = memory[j];
    totalSize += sizes[j];
    totalMemory += memory[j];
    levMaxTime = 0.0;
    for ( i=0; i < NumLevels; i++ ) {
      levMaxTime += costs[i][j];
    }
    fprintf( stdout, "CPU[ %d ]: %11g S, %11g MB, %g size\n",
      j, levMaxTime/1e6, memory[j]/1e6, sizes[j] );
  }
  fprintf( stdout, "ORIGINAL: %11g S, %11g MB, %g size\n",
    savedTime/1e6, savedMemory/1e6, savedSize );
  fprintf( stdout, "TOTALS:   %11g S, %11g MB, %g size\n",
    totalTime/1e6, totalMemory/1e6, totalSize );
  fprintf( stdout, "MAX:      %11g S, %11g MB, %g size\n",
    maxTime/1e6,     maxMemory/1e6,   maxSize );
  fprintf( stdout, "MEASURES:  %g s, %g sa, %g rho, %g eta\n",
    savedTime/maxTime, totalTime/maxTime, savedTime/totalTime,
    savedTime/maxTime/ArchSize*100.0);

  fprintf( stdout, "***** Final Distribution *****\n" );
  for ( i=0; i < NumTasks; i++ ) {
    printf( "Task[%4d]: type %d, time %5gS, mem %5gMB, size %5d\n    Group %s\n",
       i, tasks[i].type,
       TaskTime(tasks[i], tasks[i].group)/1.0e6,
       TaskMemory(tasks[i], tasks[i].group)/1.0e6,
       tasks[i].size,
       GroupString(&(groups[tasks[i].group])));
  }
}

/* Task Time / Memory are computed using the model alpha*N^beta, where
 * the coefficients are taken from Chapter 2 of DOMLCADS by D. A. Gates.
 * The time coefficients are taken from the per-iteration times of
 * a transient analysis.  For multiprocessor groups, simple models are used
 * to predict the additional time needed for overhead/communication as well
 * as additional memory used for parallel data structures.  The models are
 * very crude, and have not been compared to actual measurements on a
 * parallel device simulator.
 */

double
TaskTime( task, groupID )
Task task;
int groupID;
{
  double cost = 0;
  double efficiency, speedup;
  Group *group = &(groups[groupID]);

  switch ( task.type ) {
  case NUMERONE:
    cost = 10 + 46.9 * pow ((double) task.size, 1.133);
    speedup = 1.0 + (group->size-1)/(1.0 + 100.0/task.size);
    efficiency = speedup/group->size;
    efficiency *= (1.0 + MaxEfficiency * (group->size-1))/group->size;
    cost *= 1.0 / efficiency;
    break;
  case NUMERTWO:
    cost = 10 + 0.87 * pow ((double) task.size, 2.201);
    speedup = 1.0 + (group->size-1)/(1.0 + 100.0/task.size);
    efficiency = speedup/group->size;
    efficiency *= (1.0 + MaxEfficiency * (group->size-1))/group->size;
    cost *= 1.0 / efficiency;
    break;
  case COMPACT:
    cost = 10 * (1+1000*(group->size-1)*(group->size-1));
    break;
  case SIMPLE:
    cost = 1 * (1+1000*(group->size-1)*(group->size-1));
    break;
  case NONE:
  default:
    break;
  }

  return( cost/MachineMflops );
}

double
TaskMemory( task, groupID )
Task task;
int groupID;
{
  double memory = 0;
  double extramem = 0.0;
  Group *group = &(groups[groupID]);

  switch ( task.type ) {
  case NUMERONE:
    memory = 100 + 3.76e3 * pow ((double) task.size, 0.830);
    extramem = 1.0 + 0.10*(group->size-1);
    memory *= extramem;
    break;
  case NUMERTWO:
    memory = 100 + 0.54e3 * pow ((double) task.size, 1.265);
    extramem = 1.0 + 0.10*(group->size-1);
    memory *= extramem;
    break;
  case COMPACT:
    memory = 100;
    break;
  case SIMPLE:
    memory = 1;
    break;
  case NONE:
  default:
    break;
  }

  return( memory );
}

int
Anneal( )
{
  double temperature;
  int numTemps = 50;
  int oldLevel, newLevel, oldGroup, newGroup, group;
  int groupSize;
  int i, j;
  int task, beginTask;
  double taskTime, taskMemory;
  double prob, lim;
  double oldTime, newTime;
  double initTime, finalTime;
  int stagnant = 0;

  initTime = PartCost();

  temperature = INITTEMP;
  while ( temperature > MINTEMP ) {
    temperature *= TEMPFACTOR;
    for ( i=0; i < (STEPS_PER_TEMP + NumTasks/10); i++ ) {

      /*
      oldLevel = random() % NumLevels;
      oldGroup = random() % levels[oldLevel].size;
      oldGroup = levels[oldLevel].groups[oldGroup].id;
      beginTask = task = random() % NumTasks;
      while (tasks[task].group != oldGroup) {
	if (task++ >= NumTasks) task = 0;
	if (task == beginTask) break;
      }
      oldGroup = tasks[task].group;
      newLevel = random() % NumLevels;
      newGroup = random() % levels[newLevel].size;
      newGroup = levels[newLevel].groups[newGroup].id;
      */
      beginTask = task = random() % NumTasks;
      oldGroup = tasks[task].group;
      newGroup = random() % NumGroups;

      if (oldGroup == newGroup) continue;

      oldTime = PartCost();

      group = newGroup;
      groupSize = groups[group].size;
      taskTime = TaskTime( tasks[task], group );
      taskMemory = TaskMemory( tasks[task], group );
      for ( j = 0; j < groupSize; j++ ) {
	costs[groups[group].level][groups[group].members[j]]
	  += taskTime / groupSize;
	memory[ groups[group].members[j] ]
	  += taskMemory / groupSize;
        sizes[ groups[group].members[j] ] += 1.0 / groupSize;
      }

      group = oldGroup;
      groupSize = groups[group].size;
      taskTime = TaskTime( tasks[task], group );
      taskMemory = TaskMemory( tasks[task], group );
      for ( j = 0; j < groupSize; j++ ) {
	costs[groups[group].level][groups[group].members[j]]
	  -= taskTime / groupSize;
	memory[ groups[group].members[j] ]
	  -= taskMemory / groupSize;
        sizes[ groups[group].members[j] ] -= 1.0 / groupSize;
      }

      newTime = PartCost();

      if ( newTime > oldTime ) {
	prob = 1.0 - exp( (oldTime - newTime) / temperature );
	lim = (double)random()/LONG_MAX;
	if ( prob > lim ) {
	  group = newGroup;
	  groupSize = groups[group].size;
	  taskTime = TaskTime( tasks[task], group );
          taskMemory = TaskMemory( tasks[task], group );
	  for ( j = 0; j < groupSize; j++ ) {
	    costs[groups[group].level][groups[group].members[j]]
	      -= taskTime / groupSize;
	    memory[ groups[group].members[j] ]
	      -= taskMemory / groupSize;
	    sizes[ groups[group].members[j] ] -= 1.0 / groupSize;
	  }

	  group = oldGroup;
	  groupSize = groups[group].size;
	  taskTime = TaskTime( tasks[task], group );
          taskMemory = TaskMemory( tasks[task], group );
	  for ( j = 0; j < groupSize; j++ ) {
	    costs[groups[group].level][groups[group].members[j]]
	      += taskTime / groupSize;
	    memory[ groups[group].members[j] ]
	      += taskMemory / groupSize;
	    sizes[ groups[group].members[j] ] += 1.0 / groupSize;
	  }
	} else {
	  tasks[task].group = newGroup;
	}
      } else {
	tasks[task].group = newGroup;
      }
    }
    finalTime = PartCost();

#ifdef DEBUG
    fprintf( stdout, "Temp[ %e ]: %g init, %g final: %g speedup\n",
      temperature, initTime, finalTime, 100/finalTime );
#endif
    if (initTime/finalTime > 1.001 || finalTime/initTime > 1.001) {
      stagnant = 0;
      initTime = finalTime;
    } else {
      stagnant++;
      initTime = finalTime;
      if (stagnant > 15) {
	numTemps = 1;
	break;
      }
    }
  }
}

double
PartCost()
{
  int i,j;
  double totalTime = 0.0;
  double levelTime, maxTime, minTime, avgTime, maxMemory;
  double partCost, penalty = 0.0;

  for ( i=0; i < NumLevels; i++ ) {
    maxTime = 0.0;
    minTime = HUGE;
    avgTime = 0.0;
    for ( j=0; j < ArchSize; j++ ) {
      if ( costs[i][j] > maxTime ) maxTime = costs[i][j];
      if ( costs[i][j] < minTime ) minTime = costs[i][j];
      avgTime += costs[i][j];
    }
    totalTime += maxTime;
    avgTime /= ArchSize;
    if (avgTime > 0.000001) {
      penalty += (1 - minTime/avgTime)*(maxTime/avgTime - 1);
    }
  }
  maxMemory = 0.0;
  for ( j=0; j < ArchSize; j++ ) {
    if ( memory[j] > maxMemory ) maxMemory = memory[j];
  }
  partCost = 100.0 * totalTime/Average;
  if (100.0*(maxMemory/MachineMbytes-1) > 10.0) {
    partCost += 10.0 * exp(10.0) + 10.0*100.0*(maxMemory/MachineMbytes-1);
  } else {
    partCost += 10.0 * exp(100.0*(maxMemory/MachineMbytes-1));
  }
  partCost += 10 * penalty;
  return( partCost );
}

#ifdef NOTDEF
double
partCostOrig()
{
  int i;
  double average = 0.0;
  double variance = 0.0;
  double deviation;

  for ( i=0; i < ArchSize; i++ ) {
    average += (double)costs[i];
    variance += ((double)costs[i])*((double)costs[i]);
  }
  average /= ArchSize;
  variance /= ArchSize;

  deviation = sqrt( variance - average*average );

  return( 100.0 * ( deviation / average + (average / Average - 1.0) ) );
}
#endif
