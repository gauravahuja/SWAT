//Put Led 2 in forward direction
//Current Stepper Mode: Half Step
//Step Angle = 7.5

#include "ConsoleSettings.h"
#include "OpenCVHeaders.h"
#include "Camera-2.h"
#include "Messages.h"
#include "ImageTool.h"
#include "Utility.h"
#include <iostream>
#include <fstream>
#include <conio.h>

using namespace std;

#define BF 0
#define YF 1
#define RF 2

char ThresholdFileName[] = "ThresholdFile";
const unsigned int NumberOfFortsPerBaseCamp = 3;


struct ColourThreshold
{
	unsigned char Rmax;
	unsigned char Rmin;
	unsigned char Gmax;
	unsigned char Gmin;
	unsigned char Bmax;
	unsigned char Bmin;

}LedThreshold, FortThreshold[NumberOfFortsPerBaseCamp];

IplImage *LedImage()
{
	SendMessageToMCU(LEDOff);
	IplImage *ArenaImage = NULL;

	Sleep(500);

	while(!isFrameAvailable());
	ArenaImage = cvCloneImage(Frame);
	
	SendMessageToMCU(LED1);
	SendMessageToMCU(LED2);
	IplImage *LedOnImage = NULL;

	IplImage *OffR = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OffG = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OffB = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	
	IplImage *OnR = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OnG = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OnB = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	
	Sleep(500);
		
	while(!isFrameAvailable());
	LedOnImage = cvCloneImage(Frame);
	
	SendMessageToMCU(LEDOff);
	cvSplit(ArenaImage, OffB, OffG, OffR, NULL);
	cvSplit(LedOnImage, OnB, OnG, OnR, NULL);
	
	cvSub(OnB, OffB, OnB);
	cvSub(OnG, OffG, OnG);
	cvSub(OnR, OffR, OnR);

	cvMerge(OnB, OnG, OnR, NULL, LedOnImage);
	return LedOnImage;
}

void SetThreshold(ColourThreshold &ObjectThreshold, IplImage *Object)
{
	cout<<"\n\nSet Object Threshold";
	
	ImageTool(Object);

	unsigned int Rmax;
	unsigned int Rmin;
	unsigned int Gmax;
	unsigned int Gmin;
	unsigned int Bmax;
	unsigned int Bmin;

	cout<<"\n\nEnter Rmax: ";
	cin>>Rmax;
	cout<<"Enter Rmin: ";
	cin>>Rmin;
	cout<<"Enter Gmax: ";
	cin>>Gmax;
	cout<<"Enter Gmin: ";
	cin>>Gmin;
	cout<<"Enter Bmax: ";
	cin>>Bmax;
	cout<<"Enter Bmin: ";
	cin>>Bmin;

	ObjectThreshold.Rmax = (unsigned char)Rmax;
	ObjectThreshold.Rmin = (unsigned char)Rmin;
	ObjectThreshold.Gmax = (unsigned char)Gmax;
	ObjectThreshold.Gmin = (unsigned char)Gmin;
	ObjectThreshold.Bmax = (unsigned char)Bmax;
	ObjectThreshold.Bmin = (unsigned char)Bmin;

	cout<<"\nSet Threshold:\n";
	cout<<"\nRmax: "<<(unsigned int)ObjectThreshold.Rmax;
	cout<<"\nRmin: "<<(unsigned int)ObjectThreshold.Rmin;
	cout<<"\nGmax: "<<(unsigned int)ObjectThreshold.Gmax;
	cout<<"\nGmin: "<<(unsigned int)ObjectThreshold.Gmin;
	cout<<"\nBmax: "<<(unsigned int)ObjectThreshold.Bmax;
	cout<<"\nBmin: "<<(unsigned int)ObjectThreshold.Bmin;
	cout<<"\n\nPress any key to continue";
	_getch();
}

void ReadThresholds()
{
	fstream file;
	int i;

	file.open(ThresholdFileName, ios::in|ios::binary);
	file.seekg(0, ios::beg);
	file.read((char *)&LedThreshold, sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[BF], sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[YF], sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[RF], sizeof(ColourThreshold));
	file.close();

	for(i = 0; i <  4; i++)
	{
		if(!i)
		{
			cout<<"\nLed Threshold:\n";
			cout<<"\nRmax: "<<(unsigned int)LedThreshold.Rmax;
			cout<<"\nRmin: "<<(unsigned int)LedThreshold.Rmin;
			cout<<"\nGmax: "<<(unsigned int)LedThreshold.Gmax;
			cout<<"\nGmin: "<<(unsigned int)LedThreshold.Gmin;
			cout<<"\nBmax: "<<(unsigned int)LedThreshold.Bmax;
			cout<<"\nBmin: "<<(unsigned int)LedThreshold.Bmin;
		}
		else
		{
			cout<<"\n\nFort "<<i-1<<" Threshold:\n";
			cout<<"\nRmax: "<<(unsigned int)FortThreshold[i-1].Rmax;
			cout<<"\nRmin: "<<(unsigned int)FortThreshold[i-1].Rmin;
			cout<<"\nGmax: "<<(unsigned int)FortThreshold[i-1].Gmax;
			cout<<"\nGmin: "<<(unsigned int)FortThreshold[i-1].Gmin;
			cout<<"\nBmax: "<<(unsigned int)FortThreshold[i-1].Bmax;
			cout<<"\nBmin: "<<(unsigned int)FortThreshold[i-1].Bmin;
		}
	}
}
void Initialize()
{
	cout<<"\n\nInitializing...";
	ZeroMemory(&LedThreshold, sizeof(ColourThreshold));
	ZeroMemory(FortThreshold, sizeof(ColourThreshold)*NumberOfFortsPerBaseCamp);
	SetConsoleScreen();
	InitializeMessages();
	cout<<"\n\nPress any key when the camera gets stable...";
	StartCam(false);
}

void DeInitialize()
{
	StopCam();
	DeInitializeMessages();
	cvDestroyAllWindows();
}

CvPoint CenterOfIntensity(IplImage *Image)
{
	unsigned long SumOfIntensities = 0;
	double Xcm = 0, Ycm = 0;
	unsigned char *Data;
	int i, j;
	
	for(i = 0; i < Image->height; i++)
	{
		for(j = 0; j < Image->width; j++)
		{
			Data = (unsigned char *)(Image->imageData + Image->widthStep*i + j) ;
			Xcm = Xcm + (*Data)*j;
			Ycm = Ycm + (*Data)*i;
			SumOfIntensities = SumOfIntensities + (*Data);
		}
	}
	Xcm = Xcm/SumOfIntensities;
	Ycm = Ycm/SumOfIntensities;
	
	CvPoint CenterOfIntensityPoint = cvPoint(cvRound(Xcm), cvRound(Ycm));
	return CenterOfIntensityPoint;
}

CvPoint FindLed(unsigned char LedNumber, ColourThreshold Threshold)
{
	SendMessageToMCU(LEDOff);

	IplImage *ArenaImage = NULL;

	cout<<"\n\nFinding led: ";
	switch(LedNumber)
	{
	case LED1:
		cout<<"Led1";
		break;
	case LED2:
		cout<<"Led2";
		break;
	}

	cvWaitKey(500);

	cout<<"\nTaking Frame";
	while(!isFrameAvailable());
	ArenaImage = cvCloneImage(Frame);
	cout<<"\nFrame Taken";
	SendMessageToMCU(LedNumber);

	IplImage *LedOnImage = NULL;
	IplImage *OffR = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OffG = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OffB = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);

	IplImage *OnR = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OnG = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	IplImage *OnB = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);

	unsigned char *RData = NULL;
	unsigned char *GData = NULL;
	unsigned char *BData = NULL;
	int i, j;

	//Delay required
	cvWaitKey(500);
	
	while(!isFrameAvailable());
	LedOnImage = cvCloneImage(Frame);
		
	SendMessageToMCU(LEDOff);
	cvSplit(ArenaImage, OffB, OffG, OffR, NULL);
	cvSplit(LedOnImage, OnB, OnG, OnR, NULL);

	cvSub(OnB, OffB, OnB);
	cvSub(OnG, OffG, OnG);
	cvSub(OnR, OffR, OnR);

	//cvMerge(OnB, OnG, OnR, NULL, LedOnImage);
	//ImageTool(LedOnImage);

	
	for(i = 0; i < ArenaImage->height; i++)
	{
		for(j = 0; j < ArenaImage->width; j++)
		{
			RData = (unsigned char *)(OnR->imageData + OnR->widthStep*i + j);
			GData = (unsigned char *)(OnG->imageData + OnG->widthStep*i + j);
			BData = (unsigned char *)(OnB->imageData + OnB->widthStep*i + j);
				
			if(isColourInBetween(*RData, Threshold.Rmin, Threshold.Rmax) && isColourInBetween(*GData, Threshold.Gmin, Threshold.Gmax) && isColourInBetween(*BData, Threshold.Bmin, Threshold.Bmax))
			{
				*RData = 255;
				*GData = 255;
				*BData = 255;
			}
			else
			{
				*RData = 0;
				*GData = 0;
				*BData = 0;
			}
		}
	}
	
	
	CvPoint LedCoord = CenterOfIntensity(OnR);	
	
	cvCircle(ArenaImage, LedCoord, 2, CV_RGB(0,255,0), 2);
	cvNamedWindow("LED");
	cvShowImage("LED", ArenaImage);

	cvNamedWindow("LED on Image");
	cvShowImage("LED on Image", LedOnImage);
	cvNamedWindow("Threshold Points");
	cvShowImage("Threshold Points", OnR);
	cvWaitKey();
	

	cvReleaseImage(&ArenaImage);
	cvReleaseImage(&LedOnImage);
	cvReleaseImage(&OffR);
	cvReleaseImage(&OffG);
	cvReleaseImage(&OffB);
	cvReleaseImage(&OnR);
	cvReleaseImage(&OnG);
	cvReleaseImage(&OnB);
	
	return LedCoord;
}
int main()
{
	char ch = 0;

	IplImage *Logo = cvLoadImage("Logo.jpg");
	cvNamedWindow("SWAT");
	cvShowImage("SWAT", Logo);
	
	cout<<"Press any key when the camera gets initialized and stable";
	Initialize();
	cvWaitKey();

	IplImage *Object = NULL;

	fstream file;

	do
	{
		cout<<"Main Controls\n";
		cout<<"\n1. Set Led Thresholds";
		cout<<"\n2. Set Blue Fort Thresholds";
		cout<<"\n3. Set Yellow Fort Thresholds";
		cout<<"\n4. Set Red Fort Thresholds";
		cout<<"\n5. Write Thresholds to File";
		cout<<"\n6. Read Thresholds From File";
		cout<<"\n7. Exit";
		cout<<"\n8. Find Led1";
		cout<<"\n9. Find Led2";
		cout<<"\n\nEnter your choice: ";
		cin>>ch;
		switch(ch)
		{
		case '1':
			Object = LedImage();
			SetThreshold(LedThreshold, Object);
			break;
		case '2':
			cout<<"\nPress any to take Image...";
			cvWaitKey();
			while(!isFrameAvailable());
			Object =  cvCloneImage(Frame);
			SetThreshold(FortThreshold[BF], Object);			
			break;
		case '3':
			cout<<"\nPress any to take Image...";
			cvWaitKey();
			while(!isFrameAvailable());
			Object =  cvCloneImage(Frame);
			SetThreshold(FortThreshold[YF], Object);			
			break;
		case '4':
			cout<<"\nPress any to take Image...";
			cvWaitKey();
			while(!isFrameAvailable());
			Object =  cvCloneImage(Frame);
			SetThreshold(FortThreshold[RF], Object);			
			break;
		
		case '5':
			file.open(ThresholdFileName, ios::out|ios::binary);
			file.seekp(0, ios::beg);
			file.write((char *)&LedThreshold, sizeof(ColourThreshold));
			file.write((char *)&FortThreshold[BF], sizeof(ColourThreshold));
			file.write((char *)&FortThreshold[YF], sizeof(ColourThreshold));
			file.write((char *)&FortThreshold[RF], sizeof(ColourThreshold));
			file.close();
			break;
		case '6':
			ReadThresholds();
			break;
		case '8':
			FindLed(LED1, LedThreshold);
			break;
		case '9':
			FindLed(LED2, LedThreshold);
			break;
		case '7':
			ch = 27;
			break;
		case 27:
			break;
		}
		cout<<"\n\n";
	}while(ch != 27);

	cvReleaseImage(&Logo);
	DeInitialize();
		
	return 0;

}
