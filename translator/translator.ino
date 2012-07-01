//Environment
#define LIGHT_DARK_THRESHOLD 500
#define BOUNDARY_IS_WHITE true

#define SPEED 150
#define RAMMING_SPEED 255
#define DELAY_180_DEGREES (500)
#define START_DELAY 4800
//Motor Controller Pin Maps
#define STBY	2
#define APWM1	3
#define AIN11	4
#define AIN12	7
#define BPWM1	5
#define BIN11	8
#define BIN12	10

#define APWM2	6
#define AIN21	11
#define AIN22	12
#define BPWM2	9
#define BIN21	13
#define BIN22	A0

//motorcontroller to motor pwm maps.
#define FRONT_RIGHT_PWM BPWM2
#define FRONT_LEFT_PWM  APWM2
#define BACK_RIGHT_PWM  APWM1
#define BACK_LEFT_PWM   BPWM1

//Line sensor pin maps
#define line_left	A6 //analog in
#define line_right	A5 //analog in
#define line_back	A4 //analog in
//IR proximity sensor pin maps
#define obj_left	A3 //digital in
#define obj_center	A2 //digital in
#define obj_right	A1 //digital in

//line sensor codes
#define LEFT_LINE	1
#define RIGHT_LINE	2
#define BACK_LINE	4
//object sensor codes
#define LEFT_OBJ	1
#define RIGHT_OBJ	2
#define CENTER_OBJ	4
//direction codes
#define FWD 0
#define BACK 1

//function prototypes
//motor direction change functions.
static void back_right(unsigned char dir);
static void back_left(unsigned char dir);
static void front_left(unsigned char dir);
static void front_right(unsigned char dir);
//movement functions
static void forward(unsigned char speed);
static void backward(unsigned char speed);
static void left(unsigned char speed);
static void right(unsigned char speed);
static void fwd_turn(unsigned char speed_left, unsigned char speed_right);
static void back_turn(unsigned char speed_left, unsigned char speed_right);
static void brake();
static void spin180(boolean dir);
//sensor functions
unsigned char line_sense();
unsigned char obj_sense();

//local variables
static unsigned char line_sensors;
static unsigned char obj_sensors;

//initialize
void setup()
{
//all motor controller pins to output and low.
pinMode(STBY, OUTPUT);	digitalWrite(STBY, LOW);

pinMode(APWM1, OUTPUT);	digitalWrite(APWM1, LOW);
pinMode(AIN11, OUTPUT);	digitalWrite(AIN11, LOW);
pinMode(AIN12, OUTPUT);	digitalWrite(AIN12, LOW);
pinMode(BPWM1, OUTPUT);	digitalWrite(BPWM1, LOW);
pinMode(BIN11, OUTPUT);	digitalWrite(BIN11, LOW);
pinMode(BIN12, OUTPUT);	digitalWrite(BIN12, LOW);

pinMode(APWM2, OUTPUT);	digitalWrite(APWM2, LOW);
pinMode(AIN21, OUTPUT);	digitalWrite(AIN21, LOW);
pinMode(AIN22, OUTPUT);	digitalWrite(AIN22, LOW);
pinMode(BPWM2, OUTPUT);	digitalWrite(BPWM2, LOW);
pinMode(BIN21, OUTPUT);	digitalWrite(BIN21, LOW);
pinMode(BIN22, OUTPUT);	digitalWrite(BIN22, LOW);
//init input pins
pinMode(line_left,	INPUT);
pinMode(line_right,	INPUT);
pinMode(line_back,	INPUT);
pinMode(obj_left,	INPUT);
pinMode(obj_center,	INPUT);
pinMode(obj_right,	INPUT);
//init serial
Serial.begin(9600);

//init statics
line_sensors = 0;
//start delay
delay(START_DELAY);
}
boolean waiting = false;
// the loop routine runs over and over again forever

void loop()
{
//check if waiting
while(waiting == true)
	{
	//check for go command
	if(Serial.available() > 0)
		{
		if(Serial.read() == 'g')
			{
			waiting = false;
			Serial.print("Go.\n");
			}
		}
		Serial.print("Waiting.\n");delay(500);
		brake();
	}
//check for stop command
if(Serial.available() > 0)
	{
	if(Serial.read() == 's')
		{
		waiting = true;
		}
	}
//read line sensors 
line_sensors = line_sense();
//check line sensors
if( (line_sensors & (LEFT_LINE | RIGHT_LINE)) != 0 ) // hit lines going forward.
	{
  //Serial.print("lines fwd\n");
	brake();
	delay(SPEED);
        backward(SPEED/2);
	delay(SPEED);
	brake();
	delay(SPEED);
        if( ( (line_sensors & LEFT_LINE ) !=0 ) && ( (line_sensors & RIGHT_LINE ) ==0 ) )
        	{
		spin180(false);
        	}
        else
	        {
		spin180(true);
		}
	}
else if( (line_sensors & BACK_LINE) == BACK_LINE ) //hit lines going backward.
	{
  
  //Serial.print("lines back\n");
	//delay(200);
	forward(150);delay(200);
	}
else //hit no lines
	{
	obj_sensors = obj_sense();
	if(	(((obj_sensors&LEFT_OBJ)==LEFT_OBJ)&&
		((obj_sensors&RIGHT_OBJ)==RIGHT_OBJ)&&
		((obj_sensors&CENTER_OBJ)==CENTER_OBJ))|| //all sensors lit up
		(((obj_sensors&LEFT_OBJ)==0)&&
		(obj_sensors&CENTER_OBJ)&&
		((obj_sensors&RIGHT_OBJ)==0)))//only center lit up
		{
  //Serial.print("all\n");
		//something in front of us!
		forward(RAMMING_SPEED);
		}
	else if((obj_sensors&LEFT_OBJ)&&(obj_sensors&CENTER_OBJ)) //center and left lit up
		{
  //Serial.print("lefta\n");
		//something in front but a bit to the left
		fwd_turn(SPEED/2,SPEED);
		}
	else if((obj_sensors&RIGHT_OBJ)&&(obj_sensors&CENTER_OBJ))//center and right lit up
		{
  //Serial.print("righta\n");
		//something in front but a bit to the right
		fwd_turn(SPEED,SPEED/2);
		}
	else if((obj_sensors&LEFT_OBJ)) //left only lit up
		{
  //Serial.print("leftb\n");
		//something in front but to the left
		//fwd_turn(SPEED/2,SPEED);
		left(SPEED);
		}
	else if((obj_sensors&RIGHT_OBJ)) //right only lit up
		{
  //Serial.print("rightb\n");
		//something in front but to the right
		//fwd_turn(SPEED,SPEED/2);
		right(SPEED);
		}
	else{ //no objects, no lines, go forward.
  //Serial.print("fwd\n");
		forward(SPEED);
		}
	}
}

static void forward(unsigned char speed)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(FWD);
front_right(FWD);
back_right(FWD);
back_left(FWD);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed);
analogWrite(FRONT_LEFT_PWM, speed);
analogWrite(BACK_RIGHT_PWM, speed);
analogWrite(BACK_LEFT_PWM, speed);
}

static void backward(unsigned char speed)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(BACK);
front_right(BACK);
back_right(BACK);
back_left(BACK);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed);
analogWrite(FRONT_LEFT_PWM, speed);
analogWrite(BACK_RIGHT_PWM, speed);
analogWrite(BACK_LEFT_PWM, speed);
}

static void left(unsigned char speed)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(BACK);
front_right(FWD);
back_right(FWD);
back_left(BACK);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed);
analogWrite(FRONT_LEFT_PWM, speed);
analogWrite(BACK_RIGHT_PWM, speed);
analogWrite(BACK_LEFT_PWM, speed);
}

static void right(unsigned char speed)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(FWD);
front_right(BACK);
back_right(BACK);
back_left(FWD);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed);
analogWrite(FRONT_LEFT_PWM, speed);
analogWrite(BACK_RIGHT_PWM, speed);
analogWrite(BACK_LEFT_PWM, speed);
}
//this sets all motors in forward direction
//but allows different left/right motor speeds
//creating a gradual turn while moving forward
static void fwd_turn(unsigned char speed_left, unsigned char speed_right)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(FWD);
front_right(FWD);
back_right(FWD);
back_left(FWD);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed_right);
analogWrite(FRONT_LEFT_PWM, speed_left);
analogWrite(BACK_RIGHT_PWM, speed_right);
analogWrite(BACK_LEFT_PWM, speed_left);
}
//this sets all motors in backward direction
//but allows different left/right motor speeds
//creating a gradual turn while moving backward
static void back_turn(unsigned char speed_left, unsigned char speed_right)
{
//get out of standby
digitalWrite(STBY,	HIGH);
//direction pins.
front_left(BACK);
front_right(BACK);
back_right(BACK);
back_left(BACK);
//change speed.
analogWrite(FRONT_RIGHT_PWM, speed_right);
analogWrite(FRONT_LEFT_PWM, speed_left);
analogWrite(BACK_RIGHT_PWM, speed_right);
analogWrite(BACK_LEFT_PWM, speed_left);
}
//more of a coast function...gearing allows this 
//to be a pretty good stop function...
static void brake()
{
//get out of standby
digitalWrite(STBY,	LOW);
//direction pins.
digitalWrite(AIN11,	LOW);
digitalWrite(AIN12,	LOW);
digitalWrite(AIN21,	LOW);
digitalWrite(AIN22,	LOW);
digitalWrite(BIN11,	LOW);
digitalWrite(BIN12,	LOW);
digitalWrite(BIN21,	LOW);
digitalWrite(BIN22,	LOW);
//change speed.
analogWrite(APWM1, 0);
analogWrite(APWM2, 0);
analogWrite(BPWM1, 0);
analogWrite(BPWM2, 0);
}

unsigned char line_sense()
{
unsigned char ret_val;
ret_val = 0;
int left = analogRead(line_left);
int right = analogRead(line_right);
int back = analogRead(line_back);

//Serial.print("left\t");Serial.print(left);
//Serial.print("right\t");Serial.print(right);
//Serial.print("back\t");Serial.print(back);
//Serial.print("\n");

if( BOUNDARY_IS_WHITE )
	{
	if(left > LIGHT_DARK_THRESHOLD )
		{
		ret_val |= LEFT_LINE;
		}
	if( right > LIGHT_DARK_THRESHOLD )
		{
		ret_val |= RIGHT_LINE;
		}
	if(analogRead(line_back) > LIGHT_DARK_THRESHOLD )
		{
		ret_val |= BACK_LINE;
		}
	}
else
	{
	if( left < LIGHT_DARK_THRESHOLD )
		{
		ret_val |= LEFT_LINE;
		}
	if( right < LIGHT_DARK_THRESHOLD )
		{
		ret_val |= RIGHT_LINE;
		}
	if( back < LIGHT_DARK_THRESHOLD )
		{
		ret_val |= BACK_LINE;
		}
	}
//if((ret_val&LEFT_LINE)==LEFT_LINE){Serial.print("left\n");}
//if((ret_val&RIGHT_LINE)==RIGHT_LINE){Serial.print("right\n");}
//if((ret_val&BACK_LINE)==BACK_LINE){Serial.print("back\n");}
return ret_val;
}

unsigned char obj_sense()
{

unsigned char ret_val;
ret_val = 0;

	if(digitalRead(obj_left) == LOW )
		{
		ret_val |= LEFT_OBJ;
		}
	if( digitalRead(obj_right) == LOW )
		{
		ret_val |= RIGHT_OBJ;
		}
	if(digitalRead(obj_center) == LOW )
		{
		ret_val |= CENTER_OBJ;
		}
  
//if((ret_val&LEFT_OBJ)==LEFT_OBJ){Serial.print("left\n");}
//if((ret_val&RIGHT_OBJ)==RIGHT_OBJ){Serial.print("right\n");}
//if((ret_val&CENTER_OBJ)==CENTER_OBJ){Serial.print("center\n");}
return ret_val;
}

static void spin180(boolean dir)
{
if(dir==true)
	{
	left(SPEED);
	}
else
	{
	right(SPEED);
	}
delay(DELAY_180_DEGREES);
}


static void back_right(unsigned char dir)
{
digitalWrite(AIN11,	dir?HIGH:LOW);
digitalWrite(AIN12,	dir?LOW:HIGH);
}

static void back_left(unsigned char dir)
{
digitalWrite(BIN11,	dir?LOW:HIGH);
digitalWrite(BIN12,	dir?HIGH:LOW);
}

static void front_left(unsigned char dir)
{
digitalWrite(AIN21,	dir?HIGH:LOW);
digitalWrite(AIN22,	dir?LOW:HIGH);
}

static void front_right(unsigned char dir)
{
digitalWrite(BIN21,	dir?LOW:HIGH);
digitalWrite(BIN22,	dir?HIGH:LOW);
}
