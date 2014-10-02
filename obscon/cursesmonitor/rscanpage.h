#define IPOINT_HOUR_LIMIT 48
#define MAX_TOKENS 150 /* needs to be big enough for pointing headers */
extern int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *divider);
extern int rscanpage(int count, int *rm_list, int pageMode);
enum {RSCAN_CHECK_ONLY = 0, RSCAN_PRINT};
