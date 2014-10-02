#define AC_VOLTS_IN_THRESHOLD 100
extern int printAgeStandoutN(long rightnow, long longvalue, int standout);
extern int printInterval(long age, int standoutValue);
extern int printIntervalNoTrailingSpace(long age, int standoutValue);
extern int printAgeNoStandout(long rightnow, long longvalue);
extern void printActiveAlarm(long longvalue);
extern int printAge(long rightnow, long longvalue);
extern void upspage(int count, int *rm_list);
