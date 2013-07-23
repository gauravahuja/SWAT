#include "OpenCVHeaders.h"
#include <stdlib.h>

int CameraImageWidth	= 640;
int CameraImagaHeight	= 480;
const int CameraFrameRate = 15;
const char *CameraWindowBasicName = "Camera";
const char *AVIFileName = "SWAT.avi";


IplImage *Frame;
bool FrameAvailable = false;

char **CameraWindow;
int *SelectedCameraID;
int NumberOfCamSelected;
unsigned long int NumberofFrames = 0;
CvVideoWriter * VideoFileWriterHandle;
bool WriteVideo = false;

inline bool isFrameAvailable()
{
	if(FrameAvailable)
	{
		FrameAvailable = false;
		return true;
	}
	else
		return false;
}

void CameraFrameHandler(IplImage *Camera0Frame)
{
	FrameAvailable = false;
	/*
	if(NumberofFrames == 0)
	{
		//Frame = cvCreateImage(cvGetSize(Camera0Frame), Camera0Frame->depth, Camera0Frame->nChannels);
		CameraImagaHeight = Camera0Frame->height;
		CameraImageWidth = Camera0Frame->width;
	}
	NumberofFrames++;
	*/
	cvReleaseImage(&Frame);
	Frame = cvCloneImage(Camera0Frame);
	//Frame->origin = 0;
	//cvFlip(Frame);
	/*
	if(WriteVideo)
	{
		cvWriteFrame(VideoFileWriterHandle, Camera0Frame);
	}
	*/
	FrameAvailable = true;
}
inline int StopCam()
{
	cvcamStop();
	cvcamExit();
	cvReleaseImage(&Frame);
	if(WriteVideo)
	{
		cvReleaseVideoWriter(&VideoFileWriterHandle);
	}

	return 0;
}
inline int StartCam(bool Video = false, bool CameraWindowEnable= true)
{
	VidFormat VideoFormat;
	VideoFormat.framerate = CameraFrameRate;
	VideoFormat.height = CameraImagaHeight;
	VideoFormat.width = CameraImageWidth;
	
	NumberOfCamSelected = cvcamSelectCamera(&SelectedCameraID);
	CameraWindow = new char* [NumberOfCamSelected];
	
	int Digits = NumberOfCamSelected/10 + 1;
	char *Temp = new char[Digits+1];

	//cout<<"Number of Cameras Selected: "<<NumberOfCamSelected<<"\n";
	
	for(int i = 0; i< NumberOfCamSelected; i++)
	{
		//cout<<"\nCameraID: "<<SelectedCameraID[i];
		
		itoa(i, Temp, 10);
		CameraWindow[i] = new char [strlen(CameraWindowBasicName) + Digits +1];
		strcpy(CameraWindow[i], CameraWindowBasicName);
		strcat(CameraWindow[i], Temp);
		
		cvNamedWindow(CameraWindow[i], CV_WINDOW_AUTOSIZE);

		cvcamSetProperty(SelectedCameraID[i], CVCAM_PROP_ENABLE, CVCAMTRUE);
		cvcamSetProperty(SelectedCameraID[i], CVCAM_PROP_RENDER, CVCAMTRUE);
		cvcamSetProperty(SelectedCameraID[i], CVCAM_PROP_SETFORMAT, &VideoFormat);
		
		if(CameraWindowEnable)
		{
			void* hWnd = cvGetWindowHandle(CameraWindow[i]);
			cvcamSetProperty(SelectedCameraID[i], CVCAM_PROP_WINDOW, &hWnd);
		}
		cvcamSetProperty(SelectedCameraID[i], CVCAM_PROP_CALLBACK, CameraFrameHandler);
	}

	if(Video)
	{
		WriteVideo = true;
		VideoFileWriterHandle = cvCreateVideoWriter(AVIFileName, -1, CameraFrameRate, cvSize(CameraImageWidth, CameraImagaHeight), 1);
		if(VideoFileWriterHandle == NULL)
		{
			MessageBoxA(NULL,"Error in writing video", "Error", MB_OK|MB_ICONERROR);
		}
	}
	else
	{
		WriteVideo = false;
	}
	
	if(NumberOfCamSelected)
	{
		if(cvcamInit() == 0)
		{
			MessageBoxA(NULL, "Error Initializing Camera", "Error", MB_OK | MB_ICONERROR);
			StopCam();
			return 0;
		}
		if(cvcamStart() < 0)
		{
			MessageBoxA(NULL, "Error Starting Camera", "Error", MB_OK | MB_ICONERROR);
			StopCam();
			return 0;
		}

		return 1;
	}
	else
	{
		MessageBoxA(NULL,"No Camera Selected", "Information", MB_OK | MB_ICONINFORMATION);
		StopCam();
		return 0;
	}
	return 0;
}

