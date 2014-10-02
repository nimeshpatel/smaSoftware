#define C1DC_PAGE_PRINT 0
#define C1DC_PAGE_CHECK_ONLY 1
typedef struct {
  int flags1[NUMANTS+1];
  int if1power[NUMANTS+1];
  int flags2[NUMANTS+1];
  int if2power[NUMANTS+1];
} C1DC_FLAGS;
extern void c1DC(int count, int pageMode, C1DC_FLAGS *flags);

