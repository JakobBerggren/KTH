#include <pic32mx.h> //Declare system specific adresses


int mode;
int first_line;
int first_circle;
int shape;

int radius;
int x_point;
int y_point;

void check_pos(int* x, int* y){
	if(*x > 127){ //Check borders and loop around if one is crossed
		*x = 0;
	}
	if(*x < 0){ //Check borders and loop around if one is crossed
		*x = 127;
			}
	if(*y > 31){ //Check borders and loop around if one is crossed
		*y = 0;
	}
	if(*y < 0){ //Check borders and loop around if one is crossed
		*y = 31;
	}
}
void move_draw(int* x, int* y, int dirx, int diry){
	(*x) += dirx;
	(*y) += diry;
	check_pos(x,y);
	oled_set(*x,*y);
}

void move_nodraw(int* x, int* y, int dirx, int diry){
	oled_invert(*x,*y);
	(*x) += dirx;
	(*y) += diry;
	check_pos(x,y);
	oled_invert(*x,*y);
}

void move_nocursor(int* x, int* y, int dirx, int diry){
	(*x) += dirx;
	(*y) += diry;
	check_pos(x,y);
}

void move_radius(int dirx){
	if(first_circle || radius < 0){
		radius = 0;
	}
	
	radius += dirx;
}

void move(int* x, int* y, int dirx, int diry){
	if(mode==1){
		move_draw(x,y,dirx,diry);
	}
	else if(mode==2){
		move_radius(dirx);
	}
	else if(mode==3){
		move_nocursor(x,y,dirx,diry);
	}
	else{
		move_nodraw(x,y,dirx,diry);
	}
}

void draw_shape(int x, int y){
	if((first_line && shape) || (first_circle && !shape)){
		x_point = x;
		y_point = y;
	}
	if(shape){
		oled_line(x_point,y_point,x,y,first_line);
		first_line = 0;
	}
	else{
		oled_circle(x_point,y_point,radius,first_circle);
		first_circle = 0;
	}
	
}

//Funktion som hanterar knapparna och kallar på dess funktioner.
void buttons(int* x, int* y){
	switch(getButton()){
		case(0b0001):
			move(x,y,1,0);
			break;
		
		case(0b0010):
			move(x,y,0,1);
			break;
		
		case(0b0100):
			move(x,y,0,-1);
			break;
		
		case(0b1000):
			move(x,y,-1,0);
			break;
		case(0):
			move(x,y,0,0);
			break;
			
			
	}
	delay(100);
}

void switches(int* x, int* y){
	switch(getSwitch()){
		case(0b0000): //Walk mode
			mode = 0;
			first_circle = 1;
			first_line = 1;
			radius=0;
			buttons(x,y);
			break;

		case(0b1000): //Draw mode
			mode = 1;
			first_circle = 1;
			first_line = 1;
			radius=0;
			buttons(x,y);
			break;
		
		case(0b0100): //Draw line mode
			mode = 3;
			shape = 1;
			first_circle = 1;
			radius = 0;
			buttons(x,y);
			draw_shape(*x,*y);	
			break;
		
		case(0b0010): //Circle mode 
			mode = 2;
			shape = 0;
			first_line =1;
			buttons(x,y);
			draw_shape(*x,*y);
			break;
		case(0b0001): //Reset mode
			first_circle = 1;
			first_line = 1;
			radius=0;
			oled_reset(*x ,*y); //Clear the entire screen
		break;
	}
}


int main() {
	first_line = 1; //Used for shape logic
	first_circle =1;
	mode = 0; //0 for non-draw, 1 for draw
	
	spi_init(); //Initialize SPI
	initialize(); //Initialize input ports
	display_init(); //Initialize the display
	oled_update(); //Draw the display
	
	int x = 0; //Current cursor x and y location
	int y = 0;
	
	while (1){	
		switches(&x,&y);
		oled_update(); //Draw the display with the changed data
	}
	return 0;
}

