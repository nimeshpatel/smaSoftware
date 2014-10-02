#define SUNSHINE_CRITERION 4
#define PWVFIELDS 36
extern float pwv[PWVFIELDS];
#define MPH_TO_METER_PER_SEC (0.44704)
#define METRIC_WEATHER_UNITS 0
#define ENGLISH_WEATHER_UNITS 1
#define WACKO_WINDDIR_MIN -99
#define WACKO_WINDDIR_MAX 400
#define HUMIDITY_MAX_STANDOUT 90
#define HUMIDITY_MIN_STANDOUT -1
#define WACKO_HUMIDITY_MIN 0
#define WACKO_HUMIDITY_MAX 110
#define WACKO_WINDSPEED_MIN -9 /* mph */
#define WACKO_WINDSPEED_MAX 150 /* mph */
#define WIND_SPEED_STANDOUT 30 /* mph */
#define WIND_SPEED_WACKO 300 /* mph */
#define WIND_DIRECTION_WACKO_PLUS 400
#define WIND_DIRECTION_WACKO_MINUS -99
#define WACKO_TEMP_MIN -20
#define WACKO_TEMP_MAX  25
#define WACKO_MBAR_MIN 500
#define WACKO_MBAR_MAX 700
/********************* added for " iced " anemometer display *****************/
#define WINDSPEED_ICED 0
#define TEMP_ICED 1
#define HUMIDITY_ICED 95
/*****************************************************************************/
float pwvToTau(float pwv);
