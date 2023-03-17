//sudo gcc INA219_.c raspberrypi4b_driver_ina219_interface.c iic.c driver_ina219_basic.c driver_ina219.c -lm -lwiringPi -o INA219_

#include "driver_ina219_basic.h"
#include "iic.h"
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

int main()
{ 
    float mA;
    
    ina219_basic_init(INA219_ADDRESS_0, 0.1);
                                    
    while(1)
	{                   
      ina219_basic_read(&mA);
                                    
      FILE *ina219WriteFile = fopen("/home/pi/Desktop/TURKSAT/INA219_/INA219data.txt", "w");
      fprintf(ina219WriteFile, "%f", mA);
      fclose(ina219WriteFile);
                            
      delay(900);
    }
                        
    ina219_basic_deinit();

    return 0;
}
