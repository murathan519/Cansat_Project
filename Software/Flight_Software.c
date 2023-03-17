/*
Mission:
1) Get sensor data from MS5611, Adafruit Ultimate GPS, INA219 and BNO055.
2) Process the raw sensor data and convert it to telemetry format.
3) Save telemetry data to SD card.
4) Send telemetry to ground station.
5) Receive video file from ground station and save it to SD card.
6) Receive separation command from ground station if needed.
7) Broadcast live to ground station.
8) Stabilize the cansat vertically during descend with PID controller by arranging the servo motors.
9) Fix the descend velocity around 10m/s with PID controller by arranging the BLDC motors.
10)Fix the altitude around 200 meters altitude for 10 seconds.
11)Count the rotation number and detect the rotation direction.
12)Put the system in low power mode if needed. (turn off the motors and save energy for communication and component feeding)
13)Assign the satellite status with respect to taken sensor data.
14)Execute the separation by activating the MOSFETs when separation status come.
15)Ignite the detonator of the fog capsule by activating the MOSFETs when separation status come.
16)Sound the buzzer after landing.
-by Osman Özcan & Murathan Bakýr
*/

// Compile => sudo gcc Flight_Software.c /usr/include/pi-bno055/i2c_bno055.c ina219/raspberrypi4b_driver_ina219_interface.c ina219/iic.c ina219/driver_ina219_basic.c ina219/driver_ina219.c -lm -lgps -l wiringPi  -o Flight_Software

#include <wiringPi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>

//--------------------------------------------------Function Definitions----------------------------------------------------
void timeFunction(time_t now, int *year, int *month, int *day, int *hour, int *minute, int *second);
void videoPacketInfo(char videoPacket[6]);
void batteryVoltage(float *current, float *voltage, int *controlCamera);
void rotation(int *rotationNumber, float *yaw, float *differenceYaw, float *oldYaw, int *singleInput, int *numberControl, int *directionControl, int *directionControl2);
void bnoFunction(float *yaw, float *pitch, float *roll);
void msFunction(float *temperature, float *pressure, float *altitude, float *velocity, float *startAltitude, int *altitudeControl);
unsigned int PROM_read(int DA, char PROM_CMD);
long CONV_read(int DA, char CONV_CMD);
void satelliteStatus(char status[20], float *altitude, float *velocity, int *statusControl);
void task(char status[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl);
void stopCoaxiel();

//-------------------------------------------------------Main Code-----------------------------------------------------------
int main()
{
	const unsigned short teamID = 59637;
	int packetNumber = -2;
		
	//Time
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	//GPS
	float gpsLatitude = 0, gpsLongitude = 0, gpsAltitude = 0;
	//MS5611 temperature and pressure sensor
	float temperature = 0, pressure = 0, altitude = 0, velocity = 0, startAltitude = 0;
	int altitudeControl = 0;
	//BNO055 IMU sensor
	float yaw = 0, pitch = 0, roll = 0;
	//INA219 current and voltage sensor
	float current = 0, voltage = 0;
	int controlCamera = 0;
	//Status
	char status[20] = "Launchpad";
	int statusControl = 0;
	//Video file transmission status
	char videoPacket[6] = "No";
	//Rotation number
	int rotationNumber = 0;
	float differenceYaw = 0, oldYaw = 0;
	int singleInput = 1, directionControl = 0, directionControl2 = 0, numberControl = 1;
	//Date
	time_t now;
	//Ground station info
	char fromGCS[2] = "";
	//MOSFET and buzzer
	int lastTime = 0, lastTimeMOSFET=0, lastTimeMOSFETControl = 1, controlMOSFET = 1, lastTimeBUZZER=0, lastTimeBUZZERControl = 1, controlBUZZER = 1;
	//Telemetry
	char timeTelemetry[21], telemetry[200], mysqlTelemetry[300];
	
	
	//Read last data and save critial info to backup txt file OOPROM.txt
	//In case of unexpected restart of RPI board, these critical data will be used to avoid losing critical information right after reinsallation
	FILE *lastDataReadFile = fopen("/home/pi/Desktop/TURKSAT/OOPROM.txt", "r"); //Since RPI doesn't have EEPROM, we save the critical backup data to OOPROM.txt which is a reference to Osman Ozcan
	fscanf(lastDataReadFile, "%d %d %f %d %d %d", &packetNumber, &rotationNumber, &startAltitude, &altitudeControl, &statusControl, &controlMOSFET);
	fclose(lastDataReadFile);
	
	//GPIO
	wiringPiSetup();
	
	pinMode(10, OUTPUT);	//Buzzer
	pinMode(24, OUTPUT);	//Seperation MOSFET 
	pinMode(25, OUTPUT);	//Fog capsule MOSFET 
	digitalWrite(10, LOW);
	digitalWrite(24, LOW);
	digitalWrite(25, LOW);
	
	//Servo and BLDC motors
	system("sudo killall servod");
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	system("echo P1-11=100 > /dev/servoblaster");	//Bottom BLDC motor
	system("echo P1-13=100 > /dev/servoblaster");	//Top BLDC motor
	
	
	//Loop function
	while(1)
	{
		//Adafruit Ultimate GPS reading
		FILE *gpsReadFile = fopen("/home/pi/Desktop/TURKSAT/GPSdata.txt", "r");
		fscanf(gpsReadFile, "%f %f %f", &gpsLatitude, &gpsLongitude, &gpsAltitude);
		fclose(gpsReadFile);
		
		//BNO055 reading
		FILE *bno055ReadFile = fopen("/home/pi/Desktop/TURKSAT/BNO055data.txt", "r");
		fscanf(bno055ReadFile, "%f %f %f", &yaw, &pitch, &roll);
		fclose(bno055ReadFile);
		
		//INA219 reading
		FILE *ina219ReadFile = fopen("/home/pi/Desktop/TURKSAT/ina219/INA219data.txt", "r");
		fscanf(ina219ReadFile, "%f", &current);
		fclose(ina219ReadFile);
		
		//MS5611 reading
		FILE *ms5611ReadFile = fopen("/home/pi/Desktop/TURKSAT/MS5611data.txt", "r");
		fscanf(ms5611ReadFile, "%f %f %f %f", &temperature, &pressure, &altitude, &velocity);
		fclose(ms5611ReadFile);
		
		//Status writing
		FILE *statusDosya = fopen("/home/pi/Desktop/TURKSAT/statusData.txt", "w");
		fprintf(statusFile, "%s", status);
		fclose(statusFile);
		
		//Function data feeding
		timeFunction(now, &year, &month, &day, &hour, &minute, &second);
		videoPacketInfo(videoPacket);
		batteryVoltage(&current, &voltage, &controlCamera);
		msFunction(&temperature, &pressure, &altitude, &velocity, &startAltitude, &altitudeControl);
		rotation(&rotationNumber, &yaw, &differenceYaw, &oldYaw, &singleInput, &numberControl, &directionControl, &directionControl2);
		satelliteStatus(status, &altitude, &velocity, &statusControl);
		task(status, &lastTimeMOSFET, &lastTimeMOSFETControl, &controlMOSFET, &lastTimeBUZZER, &lastTimeBUZZERControl);
		
		//Telemetry transmission at 1Hz
		if(millis() > lastTime+1000)
		{
			lastTime = millis();
			packetNumber++;
			
			if(packetNumber >= 1)
			{
				//txt file creation to cumulatively save telemetry
				FILE *telemetryFile = fopen("/home/pi/Desktop/TURKSAT/telemetry.txt", "a");
				//Backup txt file creation to save last critical data
				FILE *lastDataWriteFile= fopen("/home/pi/Desktop/TURKSAT/OOPROM.txt", "w");
				//txt file creation for ground station readings
				FILE *GCSFile = fopen("/home/pi/Desktop/TURKSAT/GCS.txt", "r");
				
				//Telemetry formation
				snprintf(timeTelemetry, 21, "%d/%d/%d - %d:%d:%d", day, month, year, hour, minute, second);
				snprintf(telemetry, 200, "%d,%d,%s,%.1f,%.1f,%.2f,%.1f,%.2f,%.6f,%.6f,%.1f,%s,%.2f,%.2f,%.2f,%d,%s", teamID, packetNumber, timeTelemetry, pressure, altitude, velocity, temperature, voltage, gpsLatitude, gpsLongitude, gpsAltitude, status, pitch, roll, yaw, rotationNumber, videoPacket);
				
				//Print telemetry to the screen
				printf("%s\n", telemetry);
				
				//Save telemetry 
				fprintf(telemetryFile, "%s\n", telemetry);
				fclose(telemetryFile);
				
				//Save last critical data
				fprintf(lastDataWriteFile, "%d %d %f %d %d %d", packetNumber, rotationNumber, startAltitude, altitudeControl, statusControl, controlMOSFET);
				fclose(lastDataWriteFile);
				
				//Read ground station
				fscanf(GCSFile, "%s", fromGCS);
				fclose(GCSFile);
				
				
				//React to incoming command from ground station
				if(strcmp(fromGCS, "A") == 0)
				{
					statusControl = 2;
					
					if(controlMOSFET)
					{
						digitalWrite(24, HIGH);
						digitalWrite(25, HIGH);
							
						if(lastTimeMOSFETControl)
						{
							lastTimeMOSFET = millis();
							lastTimeMOSFETControl = 0;
						}
						if(millis() > lastTimeMOSFET + 2000)  //Activate MOSFETs for 2 seconds
						{
							digitalWrite(24, LOW);
							digitalWrite(25, LOW);
								
							controlMOSFET = 0;
						}
					}
					else
					{
						digitalWrite(24, LOW);
						digitalWrite(25, LOW);
					}	
				}
				else if(strcmp(fromGCS, "B") == 0)
				{	
					system("echo P1-16=140 > /dev/servoblaster");	//Bottom servo motor
					system("echo P1-18=133 > /dev/servoblaster");	//Top servo motor
					system("echo P1-11=110 > /dev/servoblaster");	//Bottom BLDC motor
					system("echo P1-13=110 > /dev/servoblaster");	//Top BLDC motor
				}
				else if(strcmp(fromGCS, "C") == 0)
				{
					stopCoaxiel();					
				}
			
			}
		}
	}
	
    return 0;
}

//----------------------------------------------------------Functions--------------------------------------------------------------
void timeFunction(time_t now, int *year, int *month, int *day, int *hour, int *minute, int *second)
{	
	time(&now);
	struct tm *local = localtime(&now);
	
	*year = local->tm_year;
	*month = local->tm_mon;
	*day = local->tm_mday;
	*hour = local->tm_hour;
	*minute = local->tm_min;
	*second = local->tm_sec;
}

void videoPacketInfo(char videoPacket[6])
{
	if(access("/home/pi/video.mp4", F_OK) != -1)
	{
		strcpy(videoPacket, "Yes");
	}
	else
	{
		strcpy(videoPacket, "No");
	}
}

void batteryVoltage(float *current, float *voltage, int *controlCamera)
{
	*voltage = (*current/1000.0)*23;
	
	if(*voltage < 3.55 && *voltage > 3.05)
	{
		*controlCamera++;
		
		if(*controlCamera > 100)
		{
			system("sudo killall raspivid");
		}	
	}
}

void rotation(int *rotationNumber, float *yaw, float *differenceYaw, float *oldYaw, int *singleInput, int *numberControl, int *directionControl, int *directionControl2)
{
  	*differenceYaw = *yaw - *oldYaw;
  	*oldYaw = *yaw;
	
	if(*yaw > 175 && *yaw < 185 && *singleInput)
	{
		if((*numberControl%2))
		{
	 		if(*differenceYaw > 0)
			{
				*directionControl = 1;
	      	}
	      	else
			{
				*directionControl = 0;
	      	}
	    }
	    else
		{
	    	if(*differenceYaw > 0)
			{
				*directionControl2 = 1;
	      	}
	      	else
			{
				*directionControl2 = 0;
	      	}
	    }

	    if((*directionControl && *directionControl2) || !(*directionControl || *directionControl2))
		{
			(*rotationNumber)++;
	    }

		(*numberControl)++;
	    *singleInput = 0;
	}
	
	if(*yaw < 175 || *yaw > 185)
	{
    	*singleInput = 1;
  	}
}

void msFunction(float *temperature, float *pressure, float *altitude, float *velocity, float *startAltitude, int *altitudeControl)
{
	if(*altitudeControl < 1000)
	{                  
    	*startAltitude = *altitude;         //Altitude calculation after approximately 20 seconds for high accuracy
    	(*altitudeControl)++;
  	}
	
	*altitude = *altitude - *startAltitude;
	
	if(*altitude < 0)
	{
    	*altitude = 0;
  	}
}

void satelliteStatus(char status[20], float *altitude, float *velocity, int *statusControl)
{
	if(*altitude<5 && *velocity>=0 && *velocity<1 && *statusControl==0)
	{
		strcpy(status, "Launchpad");
	}
	else if(*altitude>=5 && *velocity>=1 && *statuControl==0)
	{
		strcpy(status, "Ascend");
	}
	else if(*altitude>60 && *velocity<1 && *velocity>-1 && *statusControl==0)
	{
		strcpy(status, "Apogee");
	}
	else if(*altitude>50 && *velocity<=-2 && *statusControl<2)
	{
		strcpy(status, "First Descend");
		*statusControl = 1;
	}
	else if(*altitude<50 && *altitude>40 && *velocity<-2 && *statusControl<3)
	{
		strcpy(status, "Separation");
		*statusControl = 2;
	}
	else if(*altitude<395 && *altitude>225 && *velocity<-2)
	{
		strcpy(status, "Second Descend");
		*statusControl = 2;
	}
	else if(*altitude<225 && *altitude>180 && *velocity>-3 && *velocity<3 && *statusControl==2)
	{
		strcpy(status, "Altitude Fix");
		*statusControl = 2;
	}
	else if(*altitude<180 && *velocity <-2 && *statusControl == 2)
	{
		strcpy(status, "Third Descend");
		*statusControl = 2;
	}
	else if(*altitude<5 && *velocity<1 && *velocity>-1 && *statusControl==2)
	{
		strcpy(status, "Landing");
	}
}

void task(char status[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl)
{
	if(strcmp(status, "Separation") == 0 && *controlMOSFET)
	{
		digitalWrite(24, HIGH);		//Separation MOSFET
		digitalWrite(25, HIGH);		//Fog capsule MOSFET
		
		//MOSFET activation for 2 seconds
		if(*lastTimeMOSFETControl)
		{
			*lastTimeMOSFET = millis();
			*lastTimeMOSFETControl = 0;
		}
		
		if(millis() > *lastTimeMOSFET + 2000)
		{
			*controlMOSFET = 0;
		}
	}
	
	if(strcmp(status, "Landing") == 0)
	{
		digitalWrite(10, HIGH);
		
		//After 1 minute landing, only execute buzzer
		if(*lastTimeBUZZERControl)
		{
			*lastTimeBUZZER = millis();
			*lastTimeBUZZERControl = 0;
		}
		
		if(millis() > *lastTimeBUZZER + 3000)
		{
			system("sudo killall Vertical_Stabilization Yaw_Stabilization servod");	
		}
		
		if(millis() > *lastTimeBUZZER + 60000)
		{	
			system("sudo /home/pi/Desktop/TURKSAT/buZZers");
			system("sudo killall servod BNO055 GPS INA219 MS5611 raspivid Vertical_Stabilization Yaw_Stabilization Flight_Software");	
		}
	}
	else
	{
		digitalWrite(10, LOW);
	}
}

void stopCoaxiel()
{
	system("echo P1-11=100 > /dev/servoblaster");		//Bottom BLDC motor
	system("echo P1-13=100 > /dev/servoblaster");		//Top BLDC motor
	
	system("sudo killall Vertical_Stabilization Yaw_Stabilization servod");
}
