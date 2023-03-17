// Compile => sudo gcc BNO055.c /usr/include/pi-bno055/i2c_bno055.c -o BNO055

#include <stdio.h>
#include <pi-bno055/getbno055.h>

int main() 
{	
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
	struct bnoeul bnod;
	set_mode(newmode);
	
	while(1)
	{
		get_eul(&bnod);
		
		FILE *bno055WriteFile = fopen("/home/pi/Desktop/TURKSAT/BNO055data.txt", "w");
		fprintf(bno055WriteFile, "%f %f %f", bnod.eul_head, bnod.eul_pitc, bnod.eul_roll);
		fclose(bno055WriteFile);
	}
	
	return 0;
}
