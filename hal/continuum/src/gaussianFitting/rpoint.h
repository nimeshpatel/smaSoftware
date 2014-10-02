#define TARGET_SOURCE 3
#define RX_LABEL_HIGH 91
#define RX_LABEL_LOW 92
#define SUMMARY_FILE_NAME_LEN 80
extern char summary_file_name[SUMMARY_FILE_NAME_LEN];
extern char rxString[10];  /* either 'lowfreq' or 'highfreq' */
extern int lowfreqflag;
#define MAX_TOKENS 150
extern int tokenize(char *input, char *tokenArray[MAX_TOKENS]);
