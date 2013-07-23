#include <windows.h>

//Address of Parallel Port in your Computer
#define PPORT_BASE	((short) 0x378)
#define DATA		(PPORT_BASE + 0)
#define STATUS		(DATA + 1)
#define CONTROL		(DATA + 2)

/**********************************Library Start********************************************/
/*
Parallel Port Library
Instruction for use:

1. Keep the inpout32.dll in same folder as the exe file
2. Call the InitPortLibrary() function before using Out32 or Inp32
3. Inp32(PORT_ADDRESS) return the data
4. Out32(PORT_ADDRESS, Data) Write Data to specified Address
5. Before terminating the Program Call DestroyPortLibrary() Function

*/

typedef short (_stdcall *inpfuncPtr)(short portaddr);
typedef void (_stdcall *oupfuncPtr)(short portaddr, short datum);

//After successful initialization, these 2 variables will contain function pointers.

inpfuncPtr inp32fp;
oupfuncPtr oup32fp;

inline short  Inp32 (short portaddr)
{
	return (inp32fp)(portaddr);
}

inline void  Out32 (short portaddr, short datum)
{
	(oup32fp)(portaddr,datum);
} 
HINSTANCE hLib = NULL;

inline bool InitPortLibrary()
{
	/* Load the library */
	hLib = LoadLibraryA("inpout32.dll");
	if (hLib == NULL) 
	{
		MessageBoxA(NULL,"Error","LoadLibrary Failed.",MB_OK|MB_ICONERROR);
		return false;
    }
	/* get the address of the function */
	inp32fp = (inpfuncPtr) GetProcAddress(hLib, "Inp32");
	if (inp32fp == NULL)
	{
		MessageBoxA(NULL,"Error","GetProcAddress for Inp32 Failed.",MB_OK|MB_ICONERROR);
		return false;
	}

	oup32fp = (oupfuncPtr) GetProcAddress(hLib, "Out32");
	if (oup32fp == NULL)
	{
		MessageBoxA(NULL,"Error","GetProcAddress for Oup32 Failed.",MB_OK|MB_ICONERROR);
		return false;
	}
	return true;
}

inline void DeInitializePortLibrary()
{
	FreeLibrary(hLib);
}
/*********************************Library End*********************************************/
