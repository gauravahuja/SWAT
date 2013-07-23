//Put Led 2 in forward direction
//Current Stepper Mode: Half Step
//Step Angle = 7.5

#include "ConsoleSettings.h"
#include "OpenCVHeaders.h"
#include "Camera-2.h"
#include "Utility.h"
#include "VectorAlgebra.h"
#include "Messages.h"
#include "ImageTool.h"
#include <iostream>
#include <fstream>
#include <conio.h>

using namespace std;

#define BF 0
#define YF 1
#define RF 2

#define FullStep	1
#define HalfStep	2

char ThresholdFileName[] = "ThresholdFile";

const unsigned int NumberOfBaseCamps = 3;		//Circular
const unsigned int NumberOfFortsPerBaseCamp = 3;

//Distance in Meters
const double BaseCampRadius = 0.35;	
const double FortHeight[NumberOfFortsPerBaseCamp] = {0.10, 0.25, 0.25};
const double LaserHeight = 0.04;

double MetersPerPixel = 0;
double PixelsPerMeter = 0;

const int StepperMode = HalfStep;
const double StepAngle = 7.5/StepperMode;

//const bool CheckForSimilarRadius = true;		//Base Camps Have Same Radius
const bool CheckForSimilarDistance = true;		//Base Camp centers at same radius


struct ColourThreshold
{
	unsigned char Rmax;
	unsigned char Rmin;
	unsigned char Gmax;
	unsigned char Gmin;
	unsigned char Bmax;
	unsigned char Bmin;

}LedThreshold, FortThreshold[NumberOfFortsPerBaseCamp];
struct FortStruct
{
	unsigned int Colour;
	bool FortFound;
	unsigned int Points;
	CvPoint Center;
};

struct BaseCampStruct
{
	int Radius;
	CvPoint Center;
	FortStruct Fort[NumberOfFortsPerBaseCamp];
}BaseCamp[NumberOfBaseCamps];

void ReadThresholds()
{
	fstream file;
	int i;
	file.open(ThresholdFileName, ios::in | ios::binary);
	file.seekg(0, ios::beg);

	file.read((char *)&LedThreshold, sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[BF], sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[YF], sizeof(ColourThreshold));
	file.read((char *)&FortThreshold[RF], sizeof(ColourThreshold));
	
	file.close();	
}
void Initialize()
{
	cout<<"\n\nInitializing...\n";
	ZeroMemory(BaseCamp, sizeof(BaseCampStruct)*NumberOfBaseCamps);
	ZeroMemory(&LedThreshold, sizeof(ColourThreshold));
	ZeroMemory(FortThreshold, sizeof(ColourThreshold)*NumberOfFortsPerBaseCamp);
	ReadThresholds();
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

int FindBaseCamps()
{
	cout<<"\n\nFinding Base Camps\n";
	cout<<"\nTaking Frame";
	while(!isFrameAvailable());
	IplImage *ArenaImage = cvCloneImage(Frame);
	cout<<"\nFrame Taken";
	IplImage *grArenaImage = cvCreateImage(cvGetSize(ArenaImage), IPL_DEPTH_8U, 1);
	cvCvtColor(ArenaImage, grArenaImage, CV_BGR2GRAY);
	cvSmooth(grArenaImage, grArenaImage, CV_GAUSSIAN, 9);
	cvSmooth(grArenaImage, grArenaImage, CV_GAUSSIAN, 9);
	IplImage *Gradient = cvCloneImage(grArenaImage);
	IplImage *Temp = cvCloneImage(grArenaImage);
	cvMorphologyEx(grArenaImage, Gradient, Temp, NULL, CV_MOP_GRADIENT);

	IplImage *CannyCircle = cvCreateImage(cvGetSize(Frame), IPL_DEPTH_8U, 1);
	cvZero(CannyCircle);
	CvMemStorage *Storage = cvCreateMemStorage(0);
	CvSeq *FoundCircles;

	//Variables For Canny Algorithm
	int CannyMinLowThreshold = 10;
	int CannyMaxLowThreshold = 80;
	
	//Variables For HoughCircles Algorithm
	int CircleMinDistance = 15;
	int CircleAccumulatorThreshold = 0;
	int CircleMaxAccumulatorThreshold = 80;		//Needs to be set before runtime
	int CircleMinAccumulatorThreshold = 25;		//Needs to be set before runtime
	int CircleMinRadius = 15;					//To be Set to avoid inappropiate Circles
	int CircleMaxRadius = ArenaImage->width/3;

	int *Position = new int[NumberOfBaseCamps];	//Stores index of found circles in the sequence
	CvPoint *CenterPoints = new CvPoint[NumberOfBaseCamps];
	int *RadiusOfFoundCircles = new int[NumberOfBaseCamps];
	int PositionVariable = 0;
	double RadiusDeviation = 10;	//Needs to be set before runtime
	double DistanceDeviation = 20;	//Needs to be set before runtime
	
	bool Found = false;
	int i,j;
	float *Circle = NULL;
	float TempRadius;

	while((CannyMaxLowThreshold >= CannyMinLowThreshold) && (!Found))
	{
		cvCanny(Gradient, CannyCircle, CannyMinLowThreshold, CannyMinLowThreshold*3);
		
		for(CircleAccumulatorThreshold = CircleMaxAccumulatorThreshold; (CircleAccumulatorThreshold >= CircleMinAccumulatorThreshold) && (!Found); CircleAccumulatorThreshold--)
		{
			FoundCircles = cvHoughCircles(CannyCircle, Storage, CV_HOUGH_GRADIENT, 1, CircleMinDistance, 1, CircleAccumulatorThreshold, CircleMinRadius, CircleMaxRadius);
			Circle = NULL;
			TempRadius = 0;

			//Algorithm to find Base Camps(>=2) by looking for circles with similar radius and similar distance between their centers(curerntly not implemented)
			for(i = 0; (i <= FoundCircles->total - (int)NumberOfBaseCamps) && (!Found); i++)
			{
				PositionVariable = 0;
				Circle = (float*)cvGetSeqElem(FoundCircles, i);
				TempRadius = Circle[2];
				
				Position[PositionVariable] = i;
				CenterPoints[PositionVariable].x = cvRound(Circle[0]);
				CenterPoints[PositionVariable].y = cvRound(Circle[1]);
				RadiusOfFoundCircles[PositionVariable] = cvRound(Circle[2]);
				PositionVariable++;
				
				for(j = i+1; j < FoundCircles->total; j++)
				{
					Circle = (float*)cvGetSeqElem(FoundCircles, j);
					//Checking For Similar Radius
					if(isNumberInBetween(Circle[2],TempRadius-RadiusDeviation, TempRadius+RadiusDeviation))
					{
						Position[PositionVariable] = j;
						CenterPoints[PositionVariable].x = cvRound(Circle[0]);
						CenterPoints[PositionVariable].y = cvRound(Circle[1]);
						RadiusOfFoundCircles[PositionVariable] = cvRound(Circle[2]);
						PositionVariable++;				
					}
					if(PositionVariable == NumberOfBaseCamps)
					{
						break;
					}
				}
				if(PositionVariable == NumberOfBaseCamps)
				{
					//Check For Similar Distances Between Centers
					//Available for 3 base camps only
					if(CheckForSimilarDistance)
					{
						
						double *d = new double[NumberOfBaseCamps];
						d[0] = DistanceBetweenPoints(CenterPoints[0], CenterPoints[1]);
						d[1] = DistanceBetweenPoints(CenterPoints[1], CenterPoints[2]);
						d[2] = DistanceBetweenPoints(CenterPoints[2], CenterPoints[0]);
						
						bool *b = new bool[NumberOfBaseCamps-1];
						b[0] = isNumberInBetween(d[0], d[1] - DistanceDeviation, d[1] + DistanceDeviation);
						b[1] = isNumberInBetween(d[1], d[2] - DistanceDeviation, d[2] + DistanceDeviation);
						if((b[0] == true) &&(b[1] == true))
							Found = true;
					}
					else
					{
						Found = true;					
					}
				}
			}
			if(!Found)
			{
				cvClearSeq(FoundCircles);
				cvClearMemStorage(Storage);
			}
		}
		CannyMaxLowThreshold--;
	}

	cvReleaseImage(&grArenaImage);
	cvReleaseImage(&Gradient);
	cvReleaseImage(&Temp);
	cvReleaseImage(&CannyCircle); 
	cvReleaseMemStorage(&Storage);
	delete Position;
	
	double AverageRadius = 0;

	if(Found)
	{
		for(i = 0; i < NumberOfBaseCamps; i++)
		{			
			BaseCamp[i].Center = CenterPoints[i];  
			BaseCamp[i].Radius = RadiusOfFoundCircles[i];
			for(j = 0; j <NumberOfFortsPerBaseCamp; j++)
			{
				BaseCamp[i].Fort[j].Center = cvPoint(-1, -1);
				BaseCamp[i].Fort[j].Colour = j;
				BaseCamp[i].Fort[j].FortFound = false;
				BaseCamp[i].Fort[j].Points = 0;
			
			}
			AverageRadius = AverageRadius + RadiusOfFoundCircles[i];
		}
		AverageRadius = AverageRadius/NumberOfBaseCamps;
		MetersPerPixel = BaseCampRadius/AverageRadius;
		PixelsPerMeter = AverageRadius/BaseCampRadius;	
		
		cout<<"\n\nBase Camps Found:";
		cout<<"\n\nDetails";
		cout<<"\n\nCircle Accumulator Threshold: "<<CircleAccumulatorThreshold;
		cout<<"\nCanny Threshold: "<<CannyMaxLowThreshold<<endl<<endl;
		int r,g,b,R,G,B;
		r=g=b=0;
		for(i = 0; i < NumberOfBaseCamps; i++)
		{
			switch(i)
			{
			case 0:
				r = 1;
				b= 0;
				g = 0;
				break;
			case 1:
				r = 0;
				b= 1;
				g = 0;
				break;
			case 2:
				r = 0;
				b= 0;
				g = 1;
				break;
			}
			R = 255*r;
			G = 255*g;
			B = 255*b;

			cout<<"Base Camp #: "<<i+1<<endl;
			cout<<"Center x: "<<BaseCamp[i].Center.x<<endl;
			cout<<"Center y: "<<BaseCamp[i].Center.y<<endl;
			cout<<"Radius r: "<<BaseCamp[i].Radius<<endl;
			cout<<"Colour :"<<"\nR: "<<r<<"\nG: "<<g<<"\nB: "<<b<<endl<<endl;
			cvCircle(ArenaImage, BaseCamp[i].Center, BaseCamp[i].Radius, CV_RGB(R,G,B),1);
		}

		cvNamedWindow("Found Base Camps", CV_WINDOW_AUTOSIZE);
		cvShowImage("Found Base Camps", ArenaImage);

		cvReleaseImage(&ArenaImage);

		return 1;
	}
	else
	{
		//Changed on 28/9/09
		cvReleaseImage(&ArenaImage);
		
		return 0;
	}
	//Changed on 28/9/09
	cvReleaseImage(&ArenaImage);

	return -1;
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
unsigned long int ThresholdPoints(ColourThreshold Threshold, IplImage *Image, CvPoint *Center = NULL)
{
	unsigned long int NumberOfPoints = 0;
	unsigned char *RData = NULL;
	unsigned char *GData = NULL;
	unsigned char *BData = NULL;
	int i, j;

	IplImage *B = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);
	IplImage *G = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);
	IplImage *R = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);

	cvSplit(Image, B, G, R, NULL);

	for(i = 0; i < Image->height; i++)
	{
		for(j = 0; j < Image->width; j++)
		{
			RData = (unsigned char *)(R->imageData + R->widthStep*i + j);
			GData = (unsigned char *)(G->imageData + G->widthStep*i + j);
			BData = (unsigned char *)(B->imageData + B->widthStep*i + j);
				
			if(isColourInBetween(*RData, Threshold.Rmin, Threshold.Rmax) && isColourInBetween(*GData, Threshold.Gmin, Threshold.Gmax) && isColourInBetween(*BData, Threshold.Bmin, Threshold.Bmax))
			{
				NumberOfPoints++;			
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
	cvMerge(B, G, R, NULL, Image);
	if(Center)
	{
		*Center = CenterOfIntensity(R);
	}
	cvReleaseImage(&B);
	cvReleaseImage(&G);
	cvReleaseImage(&R);
	return NumberOfPoints;
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
	
	
	/*
	cvCircle(ArenaImage, LedCoord, 2, CV_RGB(0,255,0), 2);
	cvNamedWindow("LED");
	cvShowImage("LED", ArenaImage);

	cvNamedWindow("LED On Image");
	cvShowImage("LED On Image", LedOnImage);
	cvNamedWindow("Threshold Points");
	cvShowImage("Threshold Points", OnR);
	cvWaitKey();
	*/
	

	cvReleaseImage(&ArenaImage);
	cvReleaseImage(&LedOnImage);
	cvReleaseImage(&OffR);
	cvReleaseImage(&OffG);
	cvReleaseImage(&OffB);
	cvReleaseImage(&OnR);
	cvReleaseImage(&OnG);
	cvReleaseImage(&OnB);

	//Check if this can be removed
	cvWaitKey(500);
	
	return LedCoord;
}

Vector GetBotForwardVector(CvPoint &Led1Pos, CvPoint &Led2Pos)
{
	cout<<"\n\nFinding Bot Forward Vector";
	CvPoint Led1Coord = FindLed(LED1, LedThreshold);
	CvPoint Led2Coord = FindLed(LED2, LedThreshold);

	Led1Pos = Led1Coord;
	Led2Pos = Led2Coord;
	
	Vector BotForwardVector = VectorFromAtoB(Led1Coord, Led2Coord);
	cout<<"\nFound Bot Forward Vector";
	
	return BotForwardVector;
}

double AlignBotToPoint(CvPoint Point, double AngleTolerance = 2)
{
	const unsigned long int MaxAlignmentTime = 120000;	//Max Time That is allowed in ms
	const unsigned long int TimePerDegree = 15;	//Time for bot to rotate 1 degree in ms
	Vector BotVector;
	Vector VectorFromBotToPoint;
	CvPoint Led1, Led2;
	double Angle = 0;
	double Distance = 0 ;
	int Direction;
	int Delay;
	bool Aligned = false;
	unsigned long int StartTime = GetTickCount();
	unsigned long int CurrentTime = 0;
	cout<<"\n\nAligning Bot\n";
	
	do
	{
		SendMessageToMCU(StopMoving);		
		BotVector = GetBotForwardVector(Led1, Led2);
		Distance = DistanceBetweenPoints(Led1, Point)*MetersPerPixel;
		VectorFromBotToPoint = VectorFromAtoB(Led1, Point);
		Angle = AngleBetween(BotVector, VectorFromBotToPoint);
		Direction = VectorAwrtB(BotVector, VectorFromBotToPoint);
		cout<<"\nBot Angle from Point: "<<Angle;
		if(isNumberInBetween(Angle, 0, AngleTolerance))
		{
			Direction = 2;
		}
		if(Distance <= BaseCampRadius)
		{
			Direction = 3;
		}
		Delay = Angle*TimePerDegree;
		switch(Direction)
		{
		case 0:	//Left
			cout<<"\nLeft";
			SendMessageToMCU(MoveLeft);
			cvWaitKey(Delay);
			break;
		case 1:	//Right
			cout<<"\nRight";
			SendMessageToMCU(MoveRight);
			cvWaitKey(Delay);
			break;
		case 2:	//Aligned
			if(isNumberInBetween(Angle, 180 - 15, 180 + 15))
			{
				SendMessageToMCU(MoveRight);
				cvWaitKey(Delay);
			}
			else
			{
				Aligned = true;
				cout<<"\nAligned";
			}
			break;
		case 3:	
			Aligned = true;
			break;
		case -1:	//Error
			MessageBoxA(NULL, "Direction Error", "Error", MB_OK | MB_ICONERROR);
			break;
		}
		CurrentTime = GetTickCount() - StartTime;
	}while((!Aligned) && (CurrentTime <= MaxAlignmentTime));
	SendMessageToMCU(StopMoving);
	return Angle;
}
void GetToBaseCamp(int BaseCampNumber)	//Needs to be changed to accomodate aligning more than once with dynamically calculating AngleTolerance
{
	const unsigned long int TimePerMeter = 1500;	//Expected time that the bot takes to travel 1 meter
	unsigned long int Delay = 0;
	CvPoint Led1;
	double DistanceOfBotFromCenter = 0;
	double AngleTolerance = 2;
	double Angle = 0;

	cout<<"\n\nGoing To Base Camp #: "<<BaseCampNumber;	
	SendMessageToMCU(StopMoving);
	
	unsigned long int StartTime = GetTickCount();
	unsigned long int CurrentTime = 0;
	bool BaseCampReached = false;
	Angle = AlignBotToPoint(BaseCamp[BaseCampNumber].Center, AngleTolerance);
	cout<<"\nFinal Angle: "<<Angle;

	do
	{
		//cout<<"Starting of Loop\n";
		SendMessageToMCU(StopMoving);
	
		Led1 = FindLed(LED1, LedThreshold);
		DistanceOfBotFromCenter = DistanceBetweenPoints(Led1, BaseCamp[BaseCampNumber].Center)*MetersPerPixel;
		cout<<"\nDistance: "<<DistanceOfBotFromCenter;
		Delay = DistanceOfBotFromCenter*TimePerMeter;
		if(DistanceOfBotFromCenter <= BaseCampRadius)
		{
			BaseCampReached = true;
		}
		else
		{
			/*
			if(DistanceOfBotFromCenter >= 2.5*BaseCampRadius)
			{
				Angle = AlignBotToPoint(BaseCamp[BaseCampNumber].Center, AngleTolerance);
				cout<<"\nAngle: "<<Angle;
			}
			*/
			SendMessageToMCU(MoveForward);
			cvWaitKey(Delay);
			CurrentTime = GetTickCount() - StartTime;
			
		}
	}while(!BaseCampReached);
	SendMessageToMCU(StopMoving);
	cout<<"\nReached BaseCamp # "<<BaseCampNumber+1;
}

void SortForts(FortStruct Forts[], int Size = NumberOfFortsPerBaseCamp)
{
	int i, j, Flag;
	Flag = 0;
	i = 0;
	FortStruct Temp;
	/*
	cout<<"\n\nBefore Sorting, in sorting function\n";
	for(i = 0; i < NumberOfFortsPerBaseCamp; i++)
	{
		cout<<"\n\nColour: "<<Forts[i].Colour;
		cout<<"\nPoints: "<<Forts[i].Points;
	}
	//cout<<"\nPress Any key to continue...";
	//cvWaitKey();
	*/
	i =0;

	while((i < Size) && (Flag == 0))
	{
		Flag = 1;
		i++;
		for(j = 0; j < Size-1; j++)
		{
			if(Forts[j].Points > Forts[j+1].Points)
			{
				Temp=Forts[j];
				Forts[j]=Forts[j+1];
				Forts[j+1]=Temp;
				Flag=0;
			}
		}
	}
	/*
	cout<<"\n\nAfter Sorting, in sorting function\n";
	for(i = 0; i < NumberOfFortsPerBaseCamp; i++)
	{
		cout<<"\n\nColour: "<<Forts[i].Colour;
		cout<<"\nPoints: "<<Forts[i].Points;
	}
	//cout<<"\nPress Any key to continue...";
	//cvWaitKey();
	*/
}


void AttackFort(int BaseCampNumber)
{
	SendMessageToMCU(StopMoving);

	const unsigned int FortThresholdPoints = 200;
	
	DWORD MaxBaseCampTime = 1000*60*3.1;
	DWORD MaxFortTime = 1000*60*1;

	int CurrentFort = -1;
	int Attempt = 0;
	bool AllFortsIlluminated = false;
	bool FoundAnyOneBaseCamp = false;
	int FortNum, i;
	DWORD BaseCampStartTime = GetTickCount();
	DWORD FortStartTime = 0;
	DWORD BaseCampCurrentTime = 0;
	DWORD FortCurrentTime = 0;
	IplImage *Image = NULL;
	IplImage *AuxillaryImage = NULL;

	double DistanceOfLaserFromFort = 0;
	double Height = 0;
	double Angle = 0;
	unsigned char NumberOfSteps = 0;
	CvPoint Led2Coord;
	
	for(FortNum = 0; (FortNum < NumberOfFortsPerBaseCamp) && (BaseCampCurrentTime <= MaxBaseCampTime); FortNum++)
	{
		do
		{
			cvWaitKey(500);
			cout<<"\n\nTaking Frame";
			while(!isFrameAvailable());
			Image = cvCloneImage(Frame);
			cout<<"\nTaken Frame";
			FoundAnyOneBaseCamp = false;
			AllFortsIlluminated = true;
			
			for(i = 0; i < NumberOfFortsPerBaseCamp; i++)
			{
				AuxillaryImage = cvCloneImage(Image);
				cout<<"\n\nColour: "<<BaseCamp[BaseCampNumber].Fort[i].Colour;
				cout<<"\nPoints: ";
				BaseCamp[BaseCampNumber].Fort[i].Points = ThresholdPoints(FortThreshold[BaseCamp[BaseCampNumber].Fort[i].Colour], AuxillaryImage);
				cout<<BaseCamp[BaseCampNumber].Fort[i].Points;
				if(BaseCamp[BaseCampNumber].Fort[i].Points >= FortThresholdPoints)
					FoundAnyOneBaseCamp = true;
				else
					AllFortsIlluminated = false;

				//Changed on 29/8/09
				cvReleaseImage(&AuxillaryImage);
			}
			//Changed on 29/8/09
			cvReleaseImage(&Image);

			BaseCampCurrentTime = GetTickCount() - BaseCampStartTime;
		}while((!FoundAnyOneBaseCamp) && (BaseCampCurrentTime <= MaxBaseCampTime));
		
		if(AllFortsIlluminated)
		{
			break;
		}

		SortForts(BaseCamp[BaseCampNumber].Fort);//Sort on Basis of Points
		/*
		cout<<"\n\nAfter Sorting\n";
		for(i = 0; i < NumberOfFortsPerBaseCamp; i++)
		{
			cout<<"\n\nColour: "<<BaseCamp[BaseCampNumber].Fort[i].Colour;
			cout<<"\nPoints: "<<BaseCamp[BaseCampNumber].Fort[i].Points;
		}
		//cout<<"\nPress Any key to continue...";
		//cvWaitKey();
		*/

		
		CurrentFort = -1;

		for(i = NumberOfFortsPerBaseCamp - 1; i >= 0; i--)//today
		{
			if(BaseCamp[BaseCampNumber].Fort[i].FortFound == false)
			{
				CurrentFort = i;
				BaseCamp[BaseCampNumber].Fort[CurrentFort].FortFound = true;
				switch(BaseCamp[BaseCampNumber].Fort[CurrentFort].Colour)
				{
				case BF:
					cout<<"\nBlue Fort Found";
					break;
				case YF:
					cout<<"\nYellow Fort Found";
					break;
				case RF:
					cout<<"\nRed Fort Found";
					break;
				default:
					cout<<"\nError in finding forts";
					break;
				}
				break;
			}
		}

		if(CurrentFort < 0)
		{
			break;
		}
			
		FortStartTime = GetTickCount();
		FortCurrentTime = 0;
		Attempt = 0;

		while((BaseCamp[BaseCampNumber].Fort[CurrentFort].Points >= FortThresholdPoints) && (!AllFortsIlluminated) && (FortCurrentTime <= MaxFortTime))
		{

			if(Attempt == 0)
			{
				cvWaitKey(100);
				cout<<"\n\nTaking Frame";
				while(!isFrameAvailable());
				Image = cvCloneImage(Frame);
				cout<<"\nTaken Frame";
				ThresholdPoints(FortThreshold[BaseCamp[BaseCampNumber].Fort[CurrentFort].Colour], Image, &BaseCamp[BaseCampNumber].Fort[CurrentFort].Center);
				AlignBotToPoint(BaseCamp[BaseCampNumber].Fort[CurrentFort].Center);
			}
			
			Attempt++;
			cout<<"\nAttempt: "<<Attempt;
			FortCurrentTime = GetTickCount() - FortStartTime;

			if((Attempt <= 3) && (FortCurrentTime <= MaxFortTime/1.5))
			{
				SendMessageToMCU(StopMoving);
				Led2Coord =  FindLed(LED2, LedThreshold);
				DistanceOfLaserFromFort = DistanceBetweenPoints(Led2Coord, BaseCamp[BaseCampNumber].Fort[CurrentFort].Center)*MetersPerPixel;
				Height = FortHeight[BaseCamp[BaseCampNumber].Fort[CurrentFort].Colour] - LaserHeight;
				Angle = atan(Height/DistanceOfLaserFromFort)*180/PI;
				NumberOfSteps = Angle/StepAngle;
			}
			else
			{
				NumberOfSteps = 0;
			}
			
			cout<<"\n\nShooting";
			cout<<"\nAngle = "<<Angle;
			cout<<"\nNumberOfSteps: "<<(unsigned int)NumberOfSteps;
			cout<<"\nDistanceOfLaserFromFort = " <<DistanceOfLaserFromFort;
			SendMessageToMCU(StopMoving);
			SendMessageToMCU(SetLaserAngle);
			SendMessageToMCU(NumberOfSteps);
			cvWaitKey(NumberOfSteps*22);
			SendMessageToMCU(StartLaser);
			cvWaitKey(500);
			cvWaitKey(1000*4);
			SendMessageToMCU(StopLaser);
			cvWaitKey(1000*2);

			do
			{
				cvWaitKey(500);
				while(!isFrameAvailable());
				Image = cvCloneImage(Frame);
				FoundAnyOneBaseCamp = false;
				AllFortsIlluminated = true;
				for(i = 0; i < NumberOfFortsPerBaseCamp; i--)//today
				{
					AuxillaryImage = cvCloneImage(Image);
					
					BaseCamp[BaseCampNumber].Fort[i].Points = ThresholdPoints(FortThreshold[BaseCamp[BaseCampNumber].Fort[i].Colour], AuxillaryImage);
					if(BaseCamp[BaseCampNumber].Fort[i].Points >= FortThresholdPoints)
						FoundAnyOneBaseCamp = true;
					else
						AllFortsIlluminated = false;

					//Changed on 28/8/09
					cvReleaseImage(&AuxillaryImage);
				}

				//Changed on 28/8/09
				cvReleaseImage(&Image);

			}while((!FoundAnyOneBaseCamp));

			FortCurrentTime = GetTickCount() - FortStartTime;
		}
	}
}


int main()
{
	char ch = 0;
	int i = 0;
	
	IplImage *Logo = cvLoadImage("Logo.jpg");
	cvNamedWindow("SWAT");
	cvShowImage("SWAT", Logo);
	
	Initialize();
	cvWaitKey();
	
	cout<<"\nThresholds\n";
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
	cout<<"\n\nPress any key to continue...";
	cvWaitKey();

	while(!FindBaseCamps());
	cout<<"\n\nPress any key to continue...";
	cvWaitKey();
	
	//cout<<"\nEnter base camp number: "; 
	//cin>>i;
	
	for(i =0; i < NumberOfBaseCamps; i++)
	{
		cout<<"\n\nBaseCamp # "<<i<<"\n";
		GetToBaseCamp(i);
		//cout<<"\nAttacking Forts...";
		//AttackFort(i);

		cout<<"\nPress any key to continue...";
		cvWaitKey(1000);
	}
	cout<<"\n\nGaMe OvEr";
	cout<<"\nPress any key to exit...";
	cvWaitKey();

	cvReleaseImage(&Logo);
	DeInitialize();
		
	return 0;
}
