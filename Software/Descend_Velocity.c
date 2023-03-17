//Compile => sudo gcc Descent_Velocity.c -lwiringPi -lm -o Descent_Velocity

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <wiringPi.h>


int main()
{
	float pid = 0, previous_error = 0, pwm = 0, error = 0;
	float yaw = 0, oldYaw = 0, differenceYaw = 0;
	float pid_p = 0;
	float pid_i = 0;
	float pid_d = 0;
	
	float kp = 0.6;
	float ki = 0.0005;
	float kd = 5;
	
	int freq = 40;
	float throttle = 165;
	
	char telemetry[100], command1[50], command2[50];
	int lastTimeTelemetry = 0, lastTimePid = 0;
	int controlStatus = 0;
	float pressure = 0, temperature = 0, velocity = 0, altitude = 0;
	char status[20] = "";
	
	
	delay(10000);
	
	
	while(controlStatus == 0)
	{
		delay(100);
		
		FILE *statusReadFile = fopen("/home/pi/Desktop/TURKSAT/statusData.txt", "r");
		fscanf(statusReadFile, "%s", status);
		fclose(statusReadFile);
		
		
		if(strcmp(status, "Separation") == 0)
		{
			delay(1500);
			controlStatus = 1;
			break;
		}
	}


	while (controlStatus == 1)
	{
		printf("aaaaa\n");
		FILE *msFile = fopen("/home/pi/Desktop/TURKSAT/MS5611data.txt", "r");
		fscanf(msFile, "%f %f %f %f", &temperature, &pressure, &altitude, &velocity);
		fclose(msFile);
		
		error = -2 - velocity;
		
		if(millis() > lastTimePid + freq)
			{
				lastTimePid = millis();
				
			
				pid_p = kp*error;									//Proportional
				
				if((-0.6 < error && error < 0.6))					//Integral
				{
				  pid_i = pid_i + (ki*error); 
				}
				
				pid_d = kd*((error - previous_error)/freq);			//Derivative
				
				pid = (pid_p + pid_i + pid_d);						//PID sum
				
				
				if(pid < -10)										//Signal restrictions
				{
				  pid = -10;
				}
				if(pid > 10)
				{
				  pid = 10;
				}

				
				pwm = throttle + pid;								//Output signal creation with reference throttle and PID signals
			}
			
			previous_error = error;
			
			snprintf(command1, 50, "echo P1-11=%f > /dev/servoblaster", pwm);
			snprintf(command2, 50, "echo P1-13=%f > /dev/servoblaster", pwm);
			
			//printf("HIZ =%.1f\n", roc/100.0);
			//printf("ERROR =%.1f\n", error);
			//printf("%.1f ", roc/100.0);
			printf("PWM = %f\n\n", pwm);
						
			FILE *motorFile = fopen("/home/pi/Desktop/TURKSAT/motorPwm.txt", "w");
			fprintf(motorFile, "%f", pwm);
			fclose(motorFile);
	}
	
	return 0;
}
