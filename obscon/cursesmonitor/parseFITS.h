#define XPIX 654
#define YPIX 496
#define DIR_NAME_LENGTH 100
#define FILE_NAME_LENGTH 100
#define SOURCE_NAME_LENGTH 40
typedef struct {
  double date;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  char fullfilename[FILE_NAME_LENGTH+DIR_NAME_LENGTH];
  char filename[FILE_NAME_LENGTH];
  char source[SOURCE_NAME_LENGTH];
  short max;
  short min;
  int exposure;
  int bias;
  int gain;
  double average;
  float magnitude;
  double azim;
  double elev;
#define PIXELS 23
  unsigned char pix[PIXELS][PIXELS];
} IMAGE_DATA;

int parseFITS(IMAGE_DATA *);
