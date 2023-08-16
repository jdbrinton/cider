#ifndef TASKS_H

typedef enum eTaskType { NONE=0, SIMPLE, COMPACT, NUMERONE, NUMERTWO } TaskType;
typedef struct sTask {
  TaskType type;
  int size;
  int group;
} Task;

extern void ReadTasks();
#endif /* TASKS_H */
