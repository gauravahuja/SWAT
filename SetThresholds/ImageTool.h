#include "OpenCVHeaders.h"
#include <iostream>
using namespace std;


char *TrackbarWindowName = "Image Tool: TrackBars";
char *ImageWindowName = "Image Tool: Image";
char *RedTrackBar = "Red";
char *GreenTrackBar = "Green";
char *BlueTrackBar = "Blue";
char *XTrackBar = "X";
char *YTrackBar = "Y";
	
int Red = 0;
int Green = 0;
int Blue = 0;
int X = 0;
int Y = 0;

void ImageToolMouseCallBack(int MouseEvent, int x, int y, int flags, void* param)
{
	IplImage *Image = (IplImage *)param;

	unsigned char *RData;
	unsigned char *GData;
	unsigned char *BData;

	IplImage *R = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);
	IplImage *G = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);
	IplImage *B = cvCreateImage(cvGetSize(Image), IPL_DEPTH_8U, 1);

	cvSplit(Image, B, G, R, NULL);	

	RData = (unsigned char *)(R->imageData + R->widthStep*y + x);
	GData = (unsigned char *)(G->imageData + G->widthStep*y + x);
	BData = (unsigned char *)(B->imageData + B->widthStep*y + x);
	
	if(MouseEvent == CV_EVENT_LBUTTONDOWN)
	{
		cvSetTrackbarPos(BlueTrackBar, TrackbarWindowName, (int)(*BData));
		cvSetTrackbarPos(GreenTrackBar, TrackbarWindowName, (int)(*GData));
		cvSetTrackbarPos(RedTrackBar, TrackbarWindowName, (int)(*RData));
		cvSetTrackbarPos(XTrackBar, TrackbarWindowName, x);
		cvSetTrackbarPos(YTrackBar, TrackbarWindowName, y);
	}

	cvReleaseImage(&R);
	cvReleaseImage(&G);
	cvReleaseImage(&B);
}

void ImageTool(IplImage *Image)
{
	cvNamedWindow(TrackbarWindowName, 1);
	cvNamedWindow(ImageWindowName, 1);
	cvCreateTrackbar(RedTrackBar,TrackbarWindowName, &Red, 255, NULL);
	cvCreateTrackbar(GreenTrackBar, TrackbarWindowName, &Green, 255, NULL);
	cvCreateTrackbar(BlueTrackBar, TrackbarWindowName, &Blue, 255, NULL);
	cvCreateTrackbar(XTrackBar, TrackbarWindowName, &X, Image->width, NULL);
	cvCreateTrackbar(YTrackBar, TrackbarWindowName, &Y, Image->height, NULL);
	cvSetMouseCallback(ImageWindowName ,ImageToolMouseCallBack, (void *)Image);
	cvShowImage(ImageWindowName, Image);
	cout<<"\n\nPress ESC to close Image Tool Windows.\nDo Not Press the Close Button.";
	while(cvWaitKey() != 27);
	cvDestroyWindow(TrackbarWindowName);
	cvDestroyWindow(ImageWindowName);
}
