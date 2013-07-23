#include <math.h>
#include "OpenCVHeaders.h"
const long double PI = 3.1415926535897932384626433832795;

struct Vector
{
	long int x;
	long int y;
	long int z;
	double Modulus;
};

inline double Modulus(Vector &A)
{
	double temp = A.x*A.x + A.y*A.y + A.z*A.z;
	temp = sqrt(temp);
	return temp;
}

inline Vector CreatePositionVector(long int x, long int y, long int z = 0)
{
	Vector Temp;
	Temp.x = x;
	Temp.y = y;
	Temp.z = z;
	Temp.Modulus = Modulus(Temp);
	return Temp;
}

inline Vector UnitVector(Vector A)
{
	if(A.Modulus != 0)
	{
		A.x = ((double)A.x)/A.Modulus;
		A.y = ((double)A.y)/A.Modulus;
		A.z = ((double)A.z)/A.Modulus;
	}
	return A;
}

inline double AngleBetween(Vector A, Vector B)
{
	double cosofangle = (A.x*B.x + A.y*B.y + A.z*B.z)/(Modulus(A)*Modulus(B));
	double angle = acos(cosofangle)*180/PI;
	return angle;
}

inline int VectorAwrtB(Vector A, Vector B)
{
	//find BxA for A and B in xy plane
	//return 0 if A is anticlockwise to B
	//return 1 if A is clockwise to B
	//return 2 if A is Parallel or AntiParallel to B
	//return -1 on error
	
	if(A.z-B.z)
		return -1;

	long int z = B.x*A.y - B.y*A.x;
	if(z > 0)
		return 0;
	if(z < 0)
		return 1;
	if(z == 0)
		return 2;
	return -1;
}

inline double DistanceBetweenPoints(CvPoint A, CvPoint B)
{
	Vector Temp = CreatePositionVector(A.x - B.x, A.y - B.y);
	return Modulus(Temp);
}

inline Vector VectorFromAtoB(CvPoint A, CvPoint B)
{
	Vector Temp;
	Temp =  CreatePositionVector(B.x - A.x, B.y - A.y);
	return Temp;
}