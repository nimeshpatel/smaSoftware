#include <stdio.h>
#include <math.h>

void main(void) {
  FILE *fp, *output;
  char line[80];
  char *ptr;
#define MAX  11000
  float freq[MAX], f, inc;
  float trans[MAX];
  int i = 0, closestpoint,pts;
  double ratio,tau,tau225,distance,closest;

  fp = fopen("atplot.out","r");
  closestpoint = 0;
  closest = 1000.0;
  do {
    ptr = fgets(line,sizeof(line),fp);
    if (ptr != NULL) {
      sscanf(line,"%f %f",&freq[i],&trans[i]);
      /*      printf("%f %f\n",freq[i],trans[i]);*/
      distance = 225.0-freq[i];
      if (fabs(distance) < closest) {
	closest = fabs(distance);
	closestpoint = i;
      }
      i++;
    }
  } while (ptr != NULL);
  pts = i;
  printf("Read %d lines\n",i);
  tau225 = -log10(trans[closestpoint]*0.01);
  printf("%d: trans=%f  tau225 = %f\n",closestpoint,trans[closestpoint],tau225);
  inc = 1.0;
#define TOPFREQ 900
  output = fopen("taufactor.h","w");
  fprintf(output,"static float taufactor[TOPFREQ] = {\n  ");
  for (f=0; f<TOPFREQ; f+=inc) {
    closest = 1000.0;
    for (i=0; i<pts; i++) {
      distance = f-freq[i];
      if (fabs(distance) < closest) {
	closest = fabs(distance);
	closestpoint = i;
      }
    }
    printf("%f %f\n",f,ratio);
    tau = -log10(trans[closestpoint]*0.01);
    ratio = tau/tau225;
    fprintf(output,"%.2e,",ratio);
    if ((int)f % 9 == 0) {
      fprintf(output,"\n  ");
    }
  }
  fprintf(output,"};\n");
  fclose(fp);
  fclose(output);
}
