#include <pic32mx.h> //Declare sign_ystem specific adresses
#include <stdint.h>


#define	prtDataCmd	PORTF
#define	maskDataCmd	0x10

#define OledMaxPix 	512 //All the pixels on the display. Each element is a column of 8 pixels. 128*32/8=512

#define	OledMaxCol	128 //Number of columns
#define	OledMaxRow	32 //Number of Rows
#define	OledMaxPag	4 //Number of pages. Data is stored in 4 8*32 rows

#define OledMaxLineLenght 132 //Pythagoras theorem for the longest possible line on the display

char oledBmp[OledMaxPix] = {1}; //Array that the display reads from
//Arrays for x and y coordinates that the oled_line function use
char oledLineX[OledMaxLineLenght];
char oledLineY[OledMaxLineLenght];

char oledCircleX[OledMaxPix];
char oledCircleY[OledMaxPix];

//char oledCircleX[OledMaxPix];
//char OledCircleY[OledMaxPix];

//Returns 2^(input)
int iPow(int y){
	int i;
	int value = 1;
	for(i = 0; i < y; i++){
		value *= 2;
	}
	return value;
}

//Returns absolute value of the input
int iAbs(int x){
	if(x < 0){
		return -x;
	}
	return x;
}

//Gives an approximation of sqrt(input)    //Is not used
int iSqrt(double t){
	int i;
	double r = 1;
	
	for(i = 0; i<50; i++){
		r = (r+(t/r))/2;
	}
	return((int) r);
}

int on_screen(int x,int y){
	if((x < 128 && x >= 0) && (y < 32 && y >= 0))
		return 1;
	return 0;
}

//Function for putting array in buffer for SPI
void oled_put_buffer(int cb, char* rgbTx){
	int ib;
	char bTmp;
	
	for(ib = 0; ib < cb; ib++){
		while(!(SPI2STAT & 0x08));
		SPI2BUF = *rgbTx++;
		while(!(SPI2STAT & 0x01));
		bTmp = SPI2BUF;
	}
}

//Sends the oledBmp to display. Draws the array of data
void oled_update(){
	int ipag;
	int icol;
	char* pb;
	
	pb = oledBmp;
	
	for(ipag = 0; ipag < OledMaxPag; ipag++){
		prtDataCmd &= ~maskDataCmd;
		
		spi_send_recv(0x22);
		spi_send_recv(ipag);
		
		spi_send_recv(0x00);
		spi_send_recv(0x10);
		
		prtDataCmd |= maskDataCmd;
		
		oled_put_buffer(OledMaxCol, pb);
		pb += OledMaxCol;
	}
}


//Set the input coordinates pixel to white
void oled_set(int x, int y){
	int chunkOffset;
	
	//Find column that the value is located in
	if(y < 8){
		chunkOffset = 0;
	}
	else if(y < 16){
		chunkOffset = 128;
		y -= 8;
	}
	else if(y < 24){
		chunkOffset = 256;
		y -= 16;
	}
	else if(y < 32){
		chunkOffset = 384;
		y -= 24;
	}
	//Change the correct x and y value in data array
	oledBmp[x + chunkOffset] = oledBmp[x + chunkOffset] | iPow(y);
}

//Set the input coordinates pixel to black
void oled_clr(int x, int y){
	int chunkOffset;
	//Find column that the value is located in
	if(y < 8){
		chunkOffset = 0;
	}
	else if(y < 16){
		chunkOffset = 128;
		y -= 8;
	}
	else if(y < 24){
		chunkOffset = 256;
		y -= 16;
	}
	else if(y < 32){
		chunkOffset = 384;
		y -= 24;
	}
	//Change the correct x and y value in data array
	oledBmp[x + chunkOffset] = oledBmp[x + chunkOffset] &= ~iPow(y);
}

//Checks if the specified pixel is black or white
int oled_check(int x, int y){
	int chunkOffset;
	//Find column that the value is located in
	if(y < 8){
		chunkOffset = 0;
	}
	else if(y < 16){
		chunkOffset = 128;
		y -= 8;
	}
	else if(y < 24){
		chunkOffset = 256;
		y -= 16;
	}
	else if(y < 32){
		chunkOffset = 384;
		y -= 24;
	}
	return ((oledBmp[x + chunkOffset] & iPow(y)) >> y);
}

//Inverts the specified pixel
void oled_invert(int x, int y){
	if(oled_check(x, y)){
		oled_clr(x, y); //Set the input coordinates pixel to black
	}
	else if(oled_check(x, y) == 0){
		oled_set(x, y); //Set the input coordinates pixel to white
	}
}

void oled_line(int x_point, int y_point, int x, int y, int boolean){
	int k;
	
	//If boolean is true it means that the mode is run for the first time and clears the arrays of last line pixel positions
	if (boolean){
		//-1 is used as a breakpoint
		oledLineX[0] = -1;
		oledLineY[0] = -1;
	}
	
	//Loop is used for clearing the previous line if the cursor has moved
	for(k = 0; k < OledMaxLineLenght; k++){
		//if breakpoint is detected breaks loop
		if(oledLineX[k] == -1) break;
		//Clears the points that is in the arrays
		oled_clr(oledLineX[k], oledLineY[k]);
	}
	
	//bresenham's line algorithm https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
	int change_x = iAbs(x - x_point);
	int sign_x = x_point < x ? 1 : -1;
	int change_y = iAbs(y - y_point);
	int sign_y = y_point<y ? 1 : -1;
	int err = (change_x>change_y ? change_x : -change_y)/2;
	int e2;
	int i;
	
	for(i = 0;;i++){
		//If current pixel is black, change to white and save the x and y of that point
		if(!oled_check(x_point, y_point)){
			oled_set(x_point, y_point);
			oledLineX[i] = x_point;
			oledLineY[i] = y_point;
		}
		//else do not save the value
		else{
			oledLineX[i] = 0;
			oledLineY[i] = 0;
		}
		//If the points has reached the end destination e.g the line is complete
		if(x_point==x && y_point== y) break;
		//Determine the next pixel location
		e2 = err;
		if(e2 > -change_x){
			err -= change_y;
			x_point += sign_x;
		}
		if(e2 < change_y){
			err += change_x;
			y_point += sign_y;
		}
	}
	//Set breakpoint at the end of the line arrays 
	oledLineX[i + 1] = -1;
	oledLineY[i + 1] = -1;
}

oled_circle(int x_point, int y_point, int radius,int boolean){
	int k;
	int n = 0;
	int err = 0;
	int r =radius;
	
	if(boolean){
		oledCircleX[0] = -1;
		oledCircleY[0] = -1;
	}
	
	for(k = 0; k<OledMaxPix;k++){
		if(oledCircleX[k] == -1 || oledCircleY[k] == -1){
			break;
		}
		oled_clr(oledCircleX[k], oledCircleY[k]);
	}
	
	k=0;
	while (r >= n){
		if (!oled_check(x_point + r, y_point + n) && on_screen(x_point + r, y_point + n)){
			oled_set(x_point + r, y_point + n);
			oledCircleX[k] =  x_point + r;
			oledCircleY[k++] = y_point + n;
		}
		
		if (!oled_check(x_point + n, y_point + r) && on_screen(x_point + n, y_point + r)){
			oled_set(x_point + n, y_point + r);
			oledCircleX[k] =  x_point + n;
			oledCircleY[k++] = y_point + r;
		}
		
		if (!oled_check(x_point - n, y_point + r) && on_screen(x_point - n, y_point + r)){
			oled_set(x_point - n, y_point + r);
			oledCircleX[k] =  x_point - n;
			oledCircleY[k++] = y_point + r;
		}
		
		if (!oled_check(x_point - r, y_point + n) && on_screen(x_point - r, y_point + n)){
			oled_set(x_point - r, y_point + n);
			oledCircleX[k] =  x_point - r;
			oledCircleY[k++] = y_point + n;
		}
		
		if (!oled_check(x_point - r, y_point - n) && on_screen(x_point - r, y_point - n)){
			oled_set(x_point - r, y_point - n);
			oledCircleX[k] =  x_point - r;
			oledCircleY[k++] = y_point - n;
		}
		
		if (!oled_check(x_point - n, y_point - r) && on_screen(x_point - n, y_point - r)){
			oled_set(x_point - n, y_point - r);
			oledCircleX[k] =  x_point - n;
			oledCircleY[k++] = y_point - r;
		}
		
		if (!oled_check(x_point + n, y_point - r) && on_screen(x_point + n, y_point - r)){
			oled_set(x_point + n, y_point - r);
			oledCircleX[k] =  x_point + n;
			oledCircleY[k++] = y_point - r;
		}
		
		if (!oled_check(x_point + r, y_point - n) && on_screen(x_point + r, y_point - n)){
			oled_set(x_point + r, y_point - n);
			oledCircleX[k] =  x_point + r;
			oledCircleY[k++] = y_point - n;
		}
		
		if (err <= 0){
			n += 1;
			err += 2*n + 1;
		}
		if (err > 0){
			r -= 1;
			err -= 2*r + 1;
		}
	}
	
	//Set breakpoint at the end of the current circle
	oledCircleX[k] = -1;
	oledCircleY[k] = -1;
}

//reset the screen to all black
void oled_reset(x, y){
	int i;
	//Set data array to all 0
	for (i = 0; i < OledMaxPix; i++){
		oledBmp[i] = 0;
	}
	oled_set(x,y); //Set the cursor to white
}
