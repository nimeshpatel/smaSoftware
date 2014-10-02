/* special line types for specific parts of a plot */
/* These should be in an include file */
# define BORDERlt 200
# define TICSlt 201
# define GRID0lt 202
# define GRIDlt 203

#if __STDC__ || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif

/* tek.driver.c */
extern void xterm(void);
extern void setup P_((void));
extern void cleanup P_((void));
extern void doflush P_((void));
extern void move P_((int x, int y));
extern void line P_((int x, int y));
extern void linetype P_((register int type));
extern void text P_((char *str));
extern void center P_((char *str));
extern void clear P_((void));
extern void cursor P_((int *x, int *y, char *flag));
#undef P_
