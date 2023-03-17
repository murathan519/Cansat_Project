//This code is for pre-flight security and memory reset
//All of the motors turning off, text files erasing and previous videos deleting

#include <stdlib.h>

int main()
{
	system("sudo killall servod Flight_Software buZZers BNO055 GPS INA219 MS5611 raspivid Vertical_Stabilization Yaw_Stabilization Descent_Velocity");
	system("sudo truncate -s 0 telemetry.txt");
	system("sudo truncate -s 0 OOPROM.txt");
	system("sudo truncate -s 0 GCS.txt");
	system("sudo truncate -s 0 GPSdata.txt");
	system("sudo truncate -s 0 BNO055data.txt");
	system("sudo truncate -s 0 INA219data.txt");
	system("sudo truncate -s 0 MS5611data.txt");
	system("sudo truncate -s 0 statusData.txt");
	system("sudo truncate -s 0 motorPwm.txt");
	system("sudo truncate -s 0 ina219/INA219data.txt");
	system("sudo rm /home/pi/video.mp4");
	system("sudo rm /home/pi/live_stream.h264");
	
	return 0;
}
