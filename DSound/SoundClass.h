//************************
// Author: Fritz Ammon
// Program: SoundClass.h
//************************

#ifndef SOUNDCLASS_H
#define SOUNDCLASS_H

#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "winmm.lib")

#include <Windows.h>
#include <MMSystem.h>
#include <dsound.h>
#include <stdio.h>

class SoundClass
{
public:
	SoundClass () ;

	BOOL Initialize (HWND) ;
	void Shutdown () ;

	BOOL LoadWaveFile (char*, IDirectSoundBuffer8**, BOOL = TRUE) ;
	void ShutdownWaveFile (IDirectSoundBuffer8**) ;

	BOOL PlayWaveFile (IDirectSoundBuffer8**, int, int) ;

private:
	struct WaveHeader
	{
		char szChunkId[4] ;
		DWORD dwChunkSize ;
		char szFormat[4] ;
		char szSubChunkId[4] ;
		DWORD dwSubChunkSize ;
		USHORT usAudioFormat ;
		USHORT usNumChannels ;
		DWORD dwSampleRate ;
		DWORD dwBytesPerSec ;
		USHORT usBlockAlign ;
		USHORT usBitsPerSamp ;
		char szDataChunkId[4] ;
		DWORD dwDataSize ;
	} ;

	BOOL InitializeDirectSound (HWND) ;
	void ShutdownDirectSound () ;

	IDirectSound8* m_dsDirectSound ;
	IDirectSoundBuffer* m_dsbPrimaryBuffer ;

} ;

#endif // SOUNDCLASS_H