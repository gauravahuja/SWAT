#include "OpenCVHeaders.h"
#include <stdlib.h>

int CameraImageWidth	= 640;
int CameraImagaHeight	= 480;
const int CameraFrameRate = 30;

IplImage *Frame;

int *SelectedCameraID;
int NumberOfCamSelected;
CvCapture *CameraCapture;

inline bool isFrameAvailable()
{
	Frame = cvQueryFrame(CameraCapture);
	return true;
}

inline int StopCam()
{
	cvReleaseCapture(&CameraCapture);
	//cvcamExit();
	return 0;
}
inline int StartCam(bool Video = false, bool CameraWindowEnable= true)
{
	
	//VidFormat VideoFormat;
	//VideoFormat.framerate = CameraFrameRate;
	//VideoFormat.height = CameraImagaHeight;
	//VideoFormat.width = CameraImageWidth;
	
	NumberOfCamSelected = 1;//cvcamSelectCamera(&SelectedCameraID);
	
	if(NumberOfCamSelected)
	{
		CameraCapture = cvCaptureFromCAM(-1);
/*		cvcamSetProperty(SelectedCameraID[0], CVCAM_PROP_ENABLE, CVCAMTRUE);
		cvcamSetProperty(SelectedCameraID[0], CVCAM_PROP_RENDER, CVCAMTRUE);
		cvcamSetProperty(SelectedCameraID[0], CVCAM_PROP_SETFORMAT, &VideoFormat);
*/
		/*if(cvcamInit() == 0)
		{
			MessageBoxA(NULL, "Error Initializing Camera", "Error", MB_OK | MB_ICONERROR);
			StopCam();
			return 0;
		}*/		
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

