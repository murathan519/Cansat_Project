// Compile => sudo gcc INA219.c ina219/raspberrypi4b_driver_ina219_interface.c ina219/iic.c ina219/driver_ina219_basic.c ina219/driver_ina219.c -lm -o INA219

#include "ina219/driver_ina219_basic.h"
#include "ina219/iic.h"
#include <stdio.h>

int main()
{
	float mA;
		
    ina219_basic_init(INA219_ADDRESS_0, 0.1);
    
    while(1)
	{
        ina219_basic_read(&mA);
        
        printf("%f\n", mA);
        
		FILE *ina219WriteFile = fopen("/home/pi/Desktop/TURKSAT/INA219data.txt", "w");
		fprintf(ina219WriteFile, "%f", mA);
		fclose(ina219WriteFile);
	}
	
	ina219_basic_deinit();
	
	return 0;
}
