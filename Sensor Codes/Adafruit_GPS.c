// Download library to /usr/include => git clone https://github.com/wdalmut/libgps.git
// Compile => gcc gpsTest.c -l gps -lm -o gpsTest

#include <stdio.h>
#include <stdlib.h>
#include <gps.h>

int main(void) {
  
    gps_init();

    loc_t data;

    while (1) {
        gps_location(&data);

        printf("Latitude: %.6f   Longitude: %.6f   Altitude: %.1f\n", data.latitude, data.longitude, data.altitude);
    }

    return 0;
}