// Compile => sudo gcc GPS.c -lgps -lm -o GPS

#include <stdio.h>
#include <stdlib.h>
#include <gps.h>

int main() 
{
    gps_init();
    loc_t data;
    
    float latitude = 0, longitude = 0, altitude = 0;

    while (1) 
	{
        gps_location(&data);
	
		latitude = data.latitude;
		longitude = data.longitude;
		altitude = data.altitude;
		
		FILE *gpsWriteFile = fopen("/home/pi/Desktop/TURKSAT/GPSdata.txt", "w");	
		fprintf(gpsWriteFile, "%f %f %f", latitude, longitude, altitude);
		fclose(gpsWriteFile);	
    }

    return 0;
}
