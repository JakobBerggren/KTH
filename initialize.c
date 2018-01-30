#include <pic32mx.h> //Declare system specific adresses
#include <stdint.h>

#define	DsplVdd		PORTFbits.RF6
#define	DsplVbat	PORTFbits.RF5
#define	DsplDataCmd	PORTFbits.RF4
#define	DsplReset	PORTGbits.RG9

#define prtVdd		PORTF
#define maskVdd		0x40
#define	prtVbat		PORTF
#define	maskVbat	0x20
#define	prtDataCmd	PORTF
#define	maskDataCmd	0x10
#define	prtReset	PORTG
#define	maskReset	0x200

#define OledMaxPix 	512
#define	OledMaxCol	128
#define	OledMaxRow	32

#define TMR2PERIOD ((80000000 / 256) / 1000)

void initialize(void){
	TRISD = TRISD | 0xFE0; //Set all switches and btn 2-4
	TRISF = TRISF | 0x2; //Enable button 1

	TRISE = TRISE & ~0x0FF;
	PORTE = PORTE & ~0x0FF;
	PORTE = PORTE | 0x1;
	
	TMR2 = 0;
	T2CON = 0x70; //Turn the timer on
	PR2 = TMR2PERIOD; //Set pre scaler value
	T2CONSET = 0x8000; //Start the time
}

uint8_t spi_send_recv(uint8_t data){ //OBSERVERA! DENNA FUNKTION ÄR TAGEN FRÅN EXEMPLET PÅ GITHUB 
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
}

void display_init(){ //Initialize the display controller and turn it on
	prtDataCmd &= ~maskDataCmd; //Clear the data/cmd bit
	prtVdd &= ~maskVdd; //
	delay(1); //Wait 1 ms for the power to come up
	
	spi_send_recv(0xAE); //Turn display off
	
	//Turn reset low then high
	prtReset &= ~maskReset;
	delay(1);
	prtReset |= maskReset;
	delay(1);
	
	//Send set charge pump and pre-charge period commands
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	//Turn on VCC then wait
	prtVbat &= ~maskVbat;
	delay(100);
	
	//Send commands to invert the display and set origin un upper left corner
	spi_send_recv(0xA1); //Remap columns
	spi_send_recv(0xC8); //Remap rows
	
	spi_send_recv(0xDA); 
	spi_send_recv(0x20);
	
	//Send display on command
	spi_send_recv(0xAF);
}

void spi_init(){
	OSCCON &= ~0x180000; //Set up the bus clock
	OSCCON |= 0x080000;
	
	//Set up output pins
	AD1PCFG = 0xFFFF;
	ODCD = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	//Set output pins for the display signals
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	//Set up input pins
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	SPI2CON = 0;	//Initialize SPI port 2 and as master
	SPI2BRG = 4;	
	
	SPI2STATCLR &= ~0x40; //Clear SPIROV
	SPI2CON |= 0x60; //Set CKP and MSTEN to 1
	
	SPI2CONSET = 0x8000; //Turn on SPI
}