#ifndef GROUPS_H

typedef struct sGroup {
  int id;
  int level;
  int size;
  int *members;
} Group;

typedef struct sGroupLevel {
  int size;
  Group *groups;
} GroupLevel;

extern void ReadGroups(), PrintGroups();
extern char *GroupString();
#endif /* GROUPS_H */
