#include <iostream>
#include "SoundClass.h"

using namespace std ;

IDirectSoundBuffer8* g_dsb8Sample1 ;
IDirectSoundBuffer8* g_dsb8Sample2 ;
SoundClass* g_scSound ;

int main ()
{
	HWND hWnd ;
	DWORD dwFirstTick ;

	g_dsb8Sample1 = 0 ;
	g_dsb8Sample2 = 0 ;
	g_scSound = new SoundClass () ;

	hWnd = GetConsoleWindow () ;

	if (!g_scSound->Initialize (hWnd))
		return 0 ;

	if (!g_scSound->LoadWaveFile ("sample1.wav", &g_dsb8Sample1))
		return 0 ;
	if (!g_scSound->LoadWaveFile ("sample1.wav", &g_dsb8Sample2))
		return 0 ;

	if (!g_scSound->PlayWaveFile (&g_dsb8Sample1, 0, DSBVOLUME_MAX))
		return 0 ;

	cout << "Starting sample1.wav again in 3 seconds...\n" ;

	dwFirstTick = GetTickCount () ;
	while (GetTickCount () - dwFirstTick < 3000) ;

	if (!g_scSound->PlayWaveFile (&g_dsb8Sample2, 0, DSBVOLUME_MAX))
		return 0 ;

	cout << "Playing for 10 seconds...\n" ;

	dwFirstTick = GetTickCount () ;
	while (GetTickCount () - dwFirstTick < 10000) ;

	cout << "Done!" ;

	g_scSound->ShutdownWaveFile (&g_dsb8Sample1) ;
	g_scSound->ShutdownWaveFile (&g_dsb8Sample2) ;
	g_scSound->Shutdown () ;

	return 0 ;
}