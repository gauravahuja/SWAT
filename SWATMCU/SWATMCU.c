//Current Stepper Mode: Half Step
//Stepper Full Step Angle: 7.5 Degrees
//Shoot Led: PB2
//Led1: PB0
//Led2: PB1


#define F_CPU 4000000UL

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define MoveForward		1
#define MoveRight		2
#define MoveLeft		3
#define MoveBack		4
#define StopMoving		0

#define SetLaserAngle	128
#define StartLaser		129//LED on PB2
#define StopLaser		130//LED off PB2

#define LED1On			200//LED on PB0
#define LED2On			201//LED on PB1
#define LEDOff			203//PB0/1 off

#define StepperPort		PORTC


unsigned char Message = 0;
unsigned char PrevMessage = 0;
unsigned char StepperCurrentSteps = 0;

void Stepper(unsigned char NumberOfSteps)
{
	StepperCurrentSteps = NumberOfSteps;
	unsigned char i;
	unsigned char Index = 0;
	static unsigned char Data[8] = { 0x08, 0x0C, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09};
	for(i = 0; i <= NumberOfSteps; i++)
	{
		Index = i%8;	
		StepperPort = Data[Index];
		_delay_ms(20);
	}
	StepperPort = 0;//Important else steppers takes all the current
}

void StepperToZero()
{
	unsigned char i;
	unsigned char Index = 0;
	static unsigned char Data[8] = { 0x08, 0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0C};
	for(i = 0; i <= StepperCurrentSteps; i++)
	{
		Index = i%8;	
		StepperPort = Data[Index];
		_delay_ms(20);
	}
	StepperPort = 0;//Important else steppers takes all the current
	StepperCurrentSteps = 0;
}

void StepperFullStep(unsigned char NumberOfSteps)
{

	unsigned char i;
	unsigned char Data = 1;
	for(i = 0; i <= NumberOfSteps; i++)
	{
		if(Data == 0x10)
		{
			Data = 0x01;
		}
		StepperPort = Data;
		Data = Data<<1;
		_delay_ms(20);
	}
	StepperPort = 0;//Important else steppers takes all the current
}


void StepperBringToZero()
{

	unsigned char i;
	unsigned char Data = 0x08;
	for(i = 0; i <= StepperCurrentSteps; i++)
	{
		StepperPort = ~Data;
		if(Data == 0x00)
		{
			Data = 0x08;
		}
		Data = Data>>1;
		_delay_ms(20);
	}
	StepperPort = 0;//Important else steppers takes all the current
}

inline void Forward()
{
	PORTD = 0b10101111;
}
inline void Backwards()
{
	PORTD = 0b01011111;
}
inline void TurnLeft()
{
	PORTD = 0b11101111;
}
inline void TurnRight()
{
	PORTD = 0b10111111;
}
inline void StopMotors()
{
	PORTD = 0b11111111;
}

void delay(unsigned int ms)
{
	int i = ms/250;
	int j;
	for(j=0;j<i;j++)
		_delay_ms(250);
}

void WDT_off(void)
{
/* Write logical one to WDTOE and WDE */
WDTCR = (1<<WDTOE) | (1<<WDE);
/* Turn off WDT */
WDTCR = 0x00;
}

int main()
{
	DDRD = 0xF0;
	DDRC = 0xFF;
	DDRB = 0xFF;
	DDRA = 0x00;
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	
	GICR |= (1<<INT1);	//enable INT1
	MCUCR = 0;			// Interrupt on low level
	sei();
	
	while(1)
	{
		
	}
	return 0;
}

ISR(INT1_vect)
{
	Message = PINA;
	unsigned char i;
	if(PrevMessage == SetLaserAngle)
	{
		PrevMessage = 0;
		StepperCurrentSteps = Message;
		Stepper(Message);
	}
	else
	{
		switch(Message)
		{
		case MoveForward:
			PrevMessage = MoveForward;
			Forward();
			break;
		case MoveBack:
			PrevMessage = MoveBack;
			Backwards();
			break;
		case MoveRight:
			PrevMessage = MoveRight;
			TurnRight();
			break;
		case MoveLeft:
			PrevMessage = MoveLeft;
			TurnLeft();
			break;
		case StopMoving:
			PrevMessage = StopMoving;
			StopMotors();
			break;
		case StartLaser:
			PrevMessage = StartLaser;
			PORTB |= (1<<PB2);
			break;
		case StopLaser:
			StopMotors();
			PrevMessage = StopLaser;
			PORTB &= ~(1<<PB2);
			StepperBringToZero();			
			break;
		case LED1On:
			PrevMessage = LED1On;
			PORTB |= (1<<PB0);
			break;
		case LED2On:
			PrevMessage = LED2On;
			PORTB |= (1<<PB1);
			break;
		case LEDOff:
			PrevMessage = LEDOff;
			PORTB &= ~(1<<PB0);
			PORTB &= ~(1<<PB1);
			break;
		case SetLaserAngle:
			StopMotors();
			PrevMessage = SetLaserAngle;
			do
			{
				i = PIND & (1<<4);
			}while(i == 0);
			break;
		}
	}
}	
		
		

	
