//Compile => sudo gcc Vertical_Stabilization.c /usr/include/pi-bno055/i2c_bno055.c -lwiringPi -o Vertical_Stabilization

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <pi-bno055/getbno055.h>
#include <string.h>

int main()
{
	float pid_pitch = 0, pid_roll = 0, error_pitch = 0, error_roll = 0, previous_pitch_error = 0, previous_roll_error = 0, pwm_pitch = 0, pwm_roll = 0;
	float pitch = 0, roll = 0, oldPitch = 0, oldRoll = 0;
	float pid_pitch_p = 0;
	float pid_pitch_i = 0;
	float pid_pitch_d = 0;

	float pid_roll_p = 0;
	float pid_roll_i = 0;
	float pid_roll_d = 0;

	//Top plate (pitch)
	float kp1 = 0.25;
	float ki1 = 0.001;   
	float kd1 = 0.5;		
	
	int control = 0;

	//Bottom plate (roll)
	float kp2 = 0.25;
	float ki2 = 0.001;
	float kd2 = 0.45;

	int freq = 40;

	//Reference PWM values
	float throttle1 = 192;
	float throttle2 = 129;

	float desired_pitch_angle = 0;
	float desired_roll_angle = 0;

	char telemetry[100], pitchCommand[100], rollCommand[100];
	char statu[20] = "";
	int lastTimeTelemetry = 0, lastTimePid = 0;
	int controlStatu = 0;
	
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
    struct bnoeul bnod;
    struct bnogyr bnod2;
    set_mode(newmode);
	
	/* For non-automatical motor check test applications 
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	snprintf(pitchCommand, 100, "echo P1-16=%f > /dev/servoblaster", throttle2);
	snprintf(rollCommand, 100, "echo P1-18=%f > /dev/servoblaster", throttle1);
	
	system(pitchCommand);
	system(rollCommand);
	*/
	
	while(controlStatus == 0)
	{
		delay(100);
		
		FILE *statusReadFile = fopen("/home/pi/Desktop/TURKSAT/statusData.txt", "r");
		fscanf(statusReadFile, "%s", status);
		fclose(statusReadFile);
		
		if(strcmp(status,"Separation") == 0)
		{
			delay(1000);
			controlStatus = 1;
		}
	}
	
	while(controlStatus == 1)
	{
		get_eul(&bnod);
			
		roll = 0 - bnod.eul_roll;
		pitch =  0 - bnod.eul_pitc;
			
		if(control > 20)
		{
				
			if(oldPitch + 6 < pitch || oldPitch -6 > pitch)
			{
				pitch = oldPitch;
			}
			if(oldRoll + 6 < roll || oldRoll -6 > roll)
			{
				roll = oldRoll;
			}
		}
			
		control++;
			
			
		if(pitch > 180 || pitch < -180)
		{
			pitch = oldPitch;
		}
		if(roll > 180 || roll < -180)
		{
			roll = oldRoll;
		}
			
			
		oldPitch = pitch;
		oldRoll = roll;
				
		if(millis() > lastTimePid + freq)
		{
			lastTimePid = millis();
				
			pid_pitch_p = kp1*pitch;									//Proportionals
			pid_roll_p = kp2*roll;
				
				
			if((-2 < pitch && pitch < 2))								//Integrals
			{
				pid_pitch_i = pid_pitch_i + (ki1*pitch); 
			}
			else
			{
				pid_pitch_i = 0;
			}
				
			if((-2 < roll && roll < 2))
			{
				pid_roll_i = pid_roll_i + (ki2*roll); 
			}
			else
			{
				pid_roll_i = 0;
			}
			  
			pid_pitch_d = kd1*((pitch - previous_pitch_error)/freq);	//Derivatives
			pid_roll_d = kd2*((roll - previous_roll_error)/freq);
			
			pid_pitch = (pid_pitch_p + pid_pitch_i + pid_pitch_d);		//PID sums
			pid_roll = (pid_roll_p + pid_roll_i + pid_roll_d);
				
			
			if(pid_pitch < -8)											//Signal restrictions
			{
			  pid_pitch = -8;
			}
			else if(pid_pitch > 8)
			{
				pid_pitch = 8;
			}
				
			if(pid_roll < -8)
			{
			  pid_roll = -8;
			}
			else if(pid_roll > 8)
			{
			  pid_roll = 8;
			}

			pwm_pitch = throttle1 - pid_pitch;							//Output signal creations with reference and PID signals
			pwm_roll = throttle2 - pid_roll;		
		}
			
			previous_pitch_error = pitch;								//Error detections
			previous_roll_error = roll;
			
			snprintf(pitchCommand, 100, "echo P1-16=%f > /dev/servoblaster && echo P1-18=%f > /dev/servoblaster", pwm_roll, pwm_pitch);		//System execution wrt to output signal
			system(pitchCommand);
			
			/* For non-automatical motor check test applications
			system("echo P1-11=125 > /dev/servoblaster");
			system("echo P1-13=125 > /dev/servoblaster");
			*/
			
			printf("PWM PITCH= %f\n", pwm_pitch);
			printf("PWM ROLL= %.1f\n", pwm_roll);
			printf("ROLL= %.1f\n", roll);
			printf("PITCH= %.1f\n\n", pitch);
			//printf("%.1f,", pitch);
			//printf("%d\n", millis());
		}
		
    return 0;
}
