#include "Messages.h"

#include <iostream>
#include <conio.h>

using namespace std;

#define HalfStep	1
#define FullStep	0

const double FullStepAngle = 7.5;
const int Mode = HalfStep;

int main()
{
	InitializeMessages();
	char ch = 0;
	double Angle = 0;
	unsigned int NumberOfSteps = 0;
	do
	{
		cout<<"Controls\n";
		cout<<"\nw. Move Forward";
		cout<<"\na. Move Left";
		cout<<"\nd. Move Right";
		cout<<"\ns. Move Backward";
		cout<<"\n . Stop Moving";
		cout<<"\n1. Led1 On";
		cout<<"\n2. Led2 On";
		cout<<"\n3. Led Off";
		cout<<"\n4. Set Laser Angle";
		cout<<"\n5. Shoot Target";	
		cout<<"\n6. Stop Shoot";
		cout<<"\n\nEnter Your Choice: ";
		ch = _getche();
		cout<<"\n\n";

		switch(ch)
		{
		case 'w':
			SendMessageToMCU(MoveForward);
			break;
		case 'a':
			SendMessageToMCU(MoveLeft);
			break;
		case 'd':
			SendMessageToMCU(MoveRight);
			break;
		case ' ':
			SendMessageToMCU(StopMoving);
			break;
		case 's':
			SendMessageToMCU(MoveBack);
			break;
		case '1':
			SendMessageToMCU(LED1);
			break;
		case '2':
			SendMessageToMCU(LED2);
			break;
		case '3':
			SendMessageToMCU(LEDOff);
			break;
		case '4':
			SendMessageToMCU(SetLaserAngle);
			Sleep(100);
			cout<<"\nEnter Angle: ";
			cin>>Angle;
			if(Mode == HalfStep)
			{
				NumberOfSteps = (Angle*2)/FullStepAngle;
			}
			else if(Mode == FullStep)
			{
				NumberOfSteps = Angle/FullStepAngle;
			}
			SendMessageToMCU(NumberOfSteps);
			break;
		case '5':
			SendMessageToMCU(StartLaser);
			break;
		case '6':
			SendMessageToMCU(StopLaser);
			break;
		}		
	}while(ch != 27);
	DeInitializeMessages();
	return 0;
}