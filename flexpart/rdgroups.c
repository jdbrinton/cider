#include <stdio.h>
#include <string.h>
#include "groups.h"
#include "tasks.h"

void ReadMachine( machFileName, cpuSpeed, memory )
char *machFileName;
double *cpuSpeed;
double *memory;
{
  FILE *fpMach;
  if ( (fpMach = fopen( machFileName, "r" )) == NULL ) {
    perror( machFileName );
    exit(1);
  }
  fscanf( fpMach, "Mflops: %lf\n", cpuSpeed );
  fscanf( fpMach, "Mbytes: %lf\n", memory );
  *memory *= 1.0e6;
  fclose( fpMach );
}

void ReadTasks( taskFileName, numTasks, tasks )
char *taskFileName;
int *numTasks;
Task **tasks;
{
  FILE *fpTask;
  Task *taskPtr;
  int numNewTasks, newTaskSize, foundTasks=0, i, taskDimen;

  if ( (fpTask = fopen( taskFileName, "r" )) == NULL ) {
    perror( taskFileName );
    exit(1);
  }

  fscanf( fpTask, "NumTasks: %d\n", numTasks );
  taskPtr = *tasks = (Task *) malloc(sizeof(Task) * *numTasks);

  while (foundTasks < *numTasks) {
    fscanf( fpTask, "%d[%d]: %d\n", &numNewTasks, &taskDimen, &newTaskSize );
    for (i=0; i < numNewTasks; i++ ) {
      taskPtr->type = 2 + taskDimen;
      taskPtr->size = newTaskSize;
      taskPtr++;
    }
    foundTasks += numNewTasks;
  }
  fclose( fpTask );
}

void ReadGroups( groupFileName, numNodes, groups, numGroups, levels, numLevels )
char *groupFileName;
int *numNodes;
Group **groups;
int *numGroups;
GroupLevel **levels;
int *numLevels;
{
  FILE *fpGroup;
  Group *groupArray, *groupPtr;
  GroupLevel *groupLevels;
  int node, levelSize;
  int groupSize, groupLevel;
  int i, j;

  if ( (fpGroup = fopen( groupFileName, "r" )) == NULL ) {
    perror( groupFileName );
    exit(1);
  }

  fscanf( fpGroup, "NumNodes:   %d\n", numNodes );
  fscanf( fpGroup, "NumGroups:  %d\n", numGroups );
  fscanf( fpGroup, "NumLevels:  %d\n", numLevels );

  groupLevels = (GroupLevel *) malloc(sizeof(GroupLevel) * *numLevels);
  groupPtr = groupArray = (Group *) malloc(sizeof(Group) * *numGroups);

  fscanf( fpGroup, "LevelSizes:" );
  for ( i=0; i < *numLevels; i++ ) {
    fscanf( fpGroup, " %d", &levelSize );
    groupLevels[i].size = levelSize;
    groupLevels[i].groups = groupPtr;
    groupPtr += levelSize;
  }
  fscanf( fpGroup, "\n" );

  groupPtr = groupArray;
  for ( i=0; i < *numGroups; i++ ) {
    fscanf( fpGroup, "G[%d][%d]{", &groupLevel, &groupSize );
    groupPtr->id = i;
    groupPtr->level = groupLevel;
    groupPtr->size = groupSize;
    groupPtr->members = (int *) malloc(sizeof(int) * groupSize );
    for ( j=0; j < groupSize - 1; j++ ) {
      fscanf( fpGroup, "%d,", &node );
      groupPtr->members[j] = node;
    }
    fscanf( fpGroup, "%d}\n", &node );
    groupPtr->members[j] = node;
    groupPtr++;
  }
  fclose( fpGroup );

  *groups = groupArray;
  *levels = groupLevels;
  return;
}

char *GroupString( group )
Group *group;
{
  static char groupString[512], *p;
  int k, groupSize = group->size;

  p = groupString;

  sprintf( p++, "{" );
  for (k=0; k < groupSize - 1; k++) {
    sprintf( p, "%d,", group->members[k] );
    p = groupString + strlen(groupString);
  }
  sprintf( p, "%d}", group->members[k] );
  p = groupString + strlen(groupString);
  *++p = '\0';

  return (groupString);
}

void PrintGroups( levels, numLevels )
GroupLevel *levels;
int numLevels;
{
  int i, j, k;
  int levelSize, groupSize;
  Group group;

  for (i=0; i < numLevels; i++) {
    levelSize = levels[i].size;
    fprintf( stdout, "Level %d: %d groups\n", i, levelSize );
    for (j=0; j < levelSize; j++) {
      group = levels[i].groups[j];
      groupSize = group.size;
      fprintf( stdout, "Group %d: %d nodes\n", j, groupSize );
      for (k=0; k < groupSize; k++) {
	fprintf( stdout, " %d", group.members[k] );
      }
      putc( '\n', stdout );
    }
  }
}

#ifdef TEST
main()
{
  GroupLevel *levels;
  Group *groups;
  int numGroups, numLevels;
  ReadGroups( "default.grp", &groups, &numGroups, &levels, &numLevels );
  PrintGroups( levels, numLevels );
}
#endif
