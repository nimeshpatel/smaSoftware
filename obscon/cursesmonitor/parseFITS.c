#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "parseFITS.h"

#if USE_AS_MAIN
void main(int arg, char *argv[]) {
  IMAGE_DATA imageData;
  parseFITS(argv[1],&imageData);
}
#endif

int parseFITS(IMAGE_DATA *data) {
  int fd, n, x, y, i, total, dataline;
  unsigned char buf[2880];
  char *ptr;
  int debug = 0;
  unsigned char pix[XPIX][YPIX];

  fd = open(data->fullfilename,O_RDONLY);
  if (fd == NULL) {
    printf("Could not open file = %s\n",data->fullfilename);
    return(-1);
  }
  n = read(fd,buf,sizeof(buf));
  if (debug) {
    printf("Read %d bytes from first header record\n",n);
  }
  if (n < 0) {
    printf("file = %s\n",data->fullfilename);
    return(-2);
  }
  if (n > 0) {
    ptr = strstr(buf,"EXPOSURE");
    sscanf(ptr+9,"%d",&data->exposure);
    if (debug) {
      printf("Found exposure time = %d\n",data->exposure);
    }
    ptr = strstr(ptr,"BIAS");
    sscanf(ptr+9,"%d",&data->bias);
    if (debug) {
      printf("Found bias = %d\n",data->bias);
    }
    ptr = strstr(ptr,"GAIN");
    sscanf(ptr+9,"%d",&data->gain);
    if (debug) {
      printf("Found gain = %d\n",data->gain);
    }
    ptr = strstr(ptr,"AZACT");
    sscanf(ptr+9,"%lf",&data->azim);
    if (debug) {
      printf("Found azim = %f\n",data->azim);
    }
    ptr = strstr(ptr,"ELACT");
    sscanf(ptr+9,"%lf",&data->elev);
    if (debug) {
      printf("Found elev = %f\n",data->elev);
    }
    ptr = strstr(ptr,"MV ");
    sscanf(ptr+9,"%f",&data->magnitude);
    if (debug) {
      printf("Found magnitude = %f\n",data->magnitude);
    }
  }
  if (debug) {
    for (i=0; i<n; i++) {
      printf("%c",buf[i]);
    }
    printf("\n");
  }
  total = n;

  n = read(fd,buf,sizeof(buf));
  if (debug) {
    printf("Read %d bytes from second header record\n",n);
  }
  if (n > 0) {
    ptr = buf;
    ptr = strstr(ptr,"MAXVAL");
    sscanf(ptr+9,"%hd",&data->max);
    if (debug) {
      printf("Found max = %d\n",data->max);
    }
    ptr = strstr(ptr,"MINVAL");
    sscanf(ptr+9,"%hd",&data->min);
    if (debug) {
      printf("Found min = %d\n",data->min);
    }
    ptr = strstr(ptr,"AVGVAL");
    sscanf(ptr+9,"%lf",&data->average);
    if (debug) {
      printf("Found avg = %f\n",data->average);
    }
  }
  if (debug) {
    for (i=0; i<n; i++) {
      printf("%c",buf[i]);
    }
    printf("Finished second header line\n");
  }
  total += n;

  x = 0;
  y = 0;
  dataline = 0;
  do {
    n = read(fd,buf,sizeof(buf));
#if 0
    if (1) {
#else
    if (dataline==0) {
#endif
      if (debug) {
        printf("Read %d bytes\n",n);
        for (i=0; i<n; i++) {
          printf("%c",buf[i]);
	}
      }
    }
    dataline++;
    total += n;
    if (debug) {
      printf("Read %d bytes\n",n);
    }
    for (i=0; i<n; i++) {
      pix[x][y] = buf[i];
      x++;
      if (x >= XPIX) {
	x = 0;
	y++;
      } 
    }
  } while (n > 0);
  if (debug) {
    printf("%d bytes in %d rows, %d columns\n",total,x,y);
  }
  close(fd);
  return(0);
}
