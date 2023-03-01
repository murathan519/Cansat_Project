// Download library to /usr/include => git clone https://github.com/fm4dd/pi-bno055.git
// Compile => gcc bno055test.c /usr/include/pi-bno055/i2c_bno055.c -o bno055test

#include <stdio.h>
#include <pi-bno055/getbno055.h>


int main() {
	
  get_i2cbus("/dev/i2c-1", "0x28");

	opmode_t newmode = ndof;
  struct bnogyr bnod;
  struct bnoeul bnod2;
  set_mode(newmode);
    
  while(1){
		get_gyr(&bnod);
		printf("GYR %3.2f %3.2f %3.2f\n", bnod.gdata_x, bnod.gdata_y, bnod.gdata_z);
	
		get_eul(&bnod2);
		printf("EUL %3.4f %3.4f %3.4f\n", bnod2.eul_head, bnod2.eul_roll, bnod2.eul_pitc);
	}
	
	return 0;
}