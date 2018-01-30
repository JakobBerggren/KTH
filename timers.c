#include <pic32mx.h> //Declare system specific adresses

void delay(int ms){
	IFS(0) = 0;
	while(ms > 0){
		if (IFS(0) & 0x100){
			ms--;
			IFS(0) = 0;
		}
	}
}