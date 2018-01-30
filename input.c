#include <pic32mx.h> //Declare system specific adresses

int getButton(void){
	unsigned char count_down;
	unsigned char count_up;
	
	#define COUNT_TARGET 25u
	
	while(count_down < COUNT_TARGET && count_up < COUNT_TARGET){
		delay(1);
		
		if (((PORTD >> 4) & 0xE) | ((PORTF >> 1) & 0x1) != 0b0000){
			count_up = 0;
			count_down++;
		}
		else{
			count_down = 0;
			count_up++;
		}
	}
	
	if (count_down >= COUNT_TARGET){
		return ((PORTD >> 4) & 0xE) | ((PORTF >> 1) & 0x1);
	}
	else{
		return 0b0000;
	}
	
}

int getSwitch(void){
	return ((PORTD >> 8) & 0xF);
}