#include "ParallelPortLibrary.h"

#define MoveForward		1	//Keep Moving Forward until StopMoving
#define MoveRight		2	//Keep Moving Right until StopMoving
#define MoveLeft		3	//Keep Moving Left until StopMoving
#define MoveBack		4	//Keep Moving Back until StopMoving
#define StopMoving		0	

#define SetLaserAngle	128	//Next Message expected to be Number of Steps
#define StartLaser		129	//Start Laser untill StopLaser
#define StopLaser		130

#define LED1			201
#define LED2			200
#define LEDOff			203


void PulseDataReady()
{
	Out32(CONTROL, 0x01);//Inverted hence physically 0
	Sleep(10);
	Out32(CONTROL, 0x00);//Inverted Hence physically 1
	Sleep(10);
}

	
inline void SendMessageToMCU(unsigned char Data)
{
	Out32(DATA, Data);
	Sleep(5);
	PulseDataReady();
}

inline void InitializeMessages()
{
	InitPortLibrary();
}

inline void DeInitializeMessages()
{
	DeInitializePortLibrary();
}
