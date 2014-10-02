#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
/* #include <resource.h> */
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define START_SAMPLES 1024
char fmt[128] = "%f";
struct avresult {
    float t;
    float v;
} av[20];

double c0 = -.21, c1 = .81, c2 = .731;

int allanvar(float *samp, int nSamp, float totTime, struct avresult *av) {
    double ssq, expvar, sTime, d;
    int stride, depth = 0, i;

    sTime = totTime / (nSamp - 1);
    expvar = 1 / sqrt(sTime * 2e9);
    depth = 0;
    ssq = 0;
    for(i = 0; i < nSamp; i++) {
	ssq += samp[i];
    }
    ssq /= nSamp;
    for(i = 0; i < nSamp; i++) {
	samp[i] = samp[i] / ssq - 1;
    }
    for(stride = 1; stride < nSamp; stride *= 2) {
	ssq = 0;
	nSamp -= stride;
	for(i = 0; i < nSamp; i++) {
	    d = samp[i] - samp[i + stride];
	    ssq +=  d * d;
	    samp[i] = (samp[i] + samp[i + stride])/2;
	}
	av[depth].v = sqrt(ssq / (nSamp * 2)) / expvar;
	av[depth++].t = stride * sTime;
    }
    return(depth);
}

int main(int argc, char *argv[]) {
    FILE *fp;
    float *samp, t1, t2;
    double fact, d;
    int i, col;
    int nSamp, bufSize;
    int depth;

    if(argc < 2) {
	fprintf(stderr, "Usage: allanvar [-cn] fname\n");
	exit(1);
    }
    if(argc > 2 && argv[1][0] == '-' && argv[1][1] == 'c') {
	col = atoi(&argv[1][2]);
	argv++;
    } else {
	col = 4;
    }
printf("Col = %d\n", col);
    for(i = 2; i < col; i++) {
	strcat(fmt, "%*f");
    }
    strcat(fmt, "%f");
    if((fp = fopen(argv[1], "r")) == NULL) {
	fprintf(stderr, "Could not open %s.  ", argv[1]);
	perror("");
	exit(1);
    }
    bufSize = START_SAMPLES;
    if((samp = (float *)malloc(bufSize * sizeof(*samp))) == NULL) {
	perror("Allocating sample memory");
	exit(1);
    }
    for(nSamp = 0;;nSamp++) {
	if(nSamp >= bufSize) {
	    bufSize += START_SAMPLES;
	    if((samp = (float *)realloc((void *)samp,
		    bufSize * sizeof(*samp))) == NULL) {
		perror("Reallocating sample memory");
		exit(1);
	    }
	}
	if(fscanf(fp, fmt, &t2, &samp[nSamp]) != 2)
	    break;
	if(nSamp == 0) t1 = t2;
	while(getc(fp) != '\n') ;
    }
    fact = 1e-6 /((t2 - t1) / (nSamp - 1));
    for(i = 0; i < nSamp; i++) {
	d = samp[i] * fact;			/* Scale to MHz */
	samp[i] = (c2*d + c1)*d + c0;	/* Convert to microwatts */
    }
    depth = allanvar(samp, nSamp, t2 - t1, av);
    for(i = 0; i < depth; i++) {
	printf("%8.3f  %8.3f\n", av[i].t, av[i].v);
    }
    return(0);
}
