#define IVSWEEP 0
#define PBSWEEP 1
extern int ivplot(int antennaNumber, int type, int sweeps);
extern void title(int antennaNumber, int type);
extern int present(char *search,char *token);
extern int computeLimits(int type, char *file);
extern void xlabel(int type, int antenna);
extern void ylabel(int type, int sweeps);
extern void drawBorder(void);
extern void init(int antenna, int type, int sweeps);
extern void refresh(int antenna, int type, int sweeps);
extern void drawBufline(int ptr);
extern void checkAutoScale(float value);
extern void updatePlot(int i, float x, float y, int plotnumber);
