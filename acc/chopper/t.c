#include <math.h>
double d;

main() {
    d = _NAN;
    if(d > 10 && d < 90) {
        printf("d tests ok\n");
    } else {
        printf("Bad d\n");
    }
}
