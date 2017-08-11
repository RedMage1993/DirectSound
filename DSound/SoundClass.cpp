//************************
// Author: Fritz Ammon
// Program: SoundClass.cpp
//************************

#include "SoundClass.h"

SoundClass::SoundClass ()
{
	m_dsDirectSound = 0 ;
	m_dsbPrimaryBuffer = 0 ;
}

BOOL SoundClass::Initialize (HWND hWnd)
{
	BOOL bResult ;

	// initialize direct sound and primary buffer
	if (!(bResult = InitializeDirectSound (hWnd)))
		return FALSE ;

	return TRUE ;
}

void SoundClass::Shutdown ()
{
	// shutdown directsound api
	ShutdownDirectSound () ;
}

BOOL SoundClass::InitializeDirectSound (HWND hWnd)
{
	HRESULT hResult ;
	DSBUFFERDESC dsBufferDesc ;
	WAVEFORMATEX wavFormatEx ;

	// initialize directsound interface pointer for default sound device
	if (FAILED (hResult = DirectSoundCreate8 (NULL, &m_dsDirectSound, NULL)))
		return FALSE ;

	// set cooperative level to primary so primary sound buffer can be modified
	if (FAILED (hResult = m_dsDirectSound->SetCooperativeLevel (hWnd, DSSCL_PRIORITY)))
		return FALSE ;

	// setup primary buffer description (dwFlags is most important)
	dsBufferDesc.dwSize = sizeof (DSBUFFERDESC) ;
	dsBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME ; // to get volume control
	dsBufferDesc.dwBufferBytes = 0 ;
	dsBufferDesc.dwReserved = 0 ;
	dsBufferDesc.lpwfxFormat = NULL ;
	dsBufferDesc.guid3DAlgorithm = GUID_NULL ;

	// get the control of primary buffer
	if (FAILED (hResult = m_dsDirectSound->CreateSoundBuffer (&dsBufferDesc, &m_dsbPrimaryBuffer, NULL)))
		return FALSE ;

	// setup sound format of primary buffer
	// using high quality sound
	wavFormatEx.wFormatTag = WAVE_FORMAT_PCM ;
	wavFormatEx.nSamplesPerSec = 44100 ;
	wavFormatEx.wBitsPerSample = 16 ;
	wavFormatEx.nChannels = 2 ;
	wavFormatEx.nBlockAlign = (wavFormatEx.wBitsPerSample / 8) * wavFormatEx.nChannels ;
	wavFormatEx.nAvgBytesPerSec = wavFormatEx.nSamplesPerSec * wavFormatEx.nBlockAlign ;
	wavFormatEx.cbSize = 0 ;

	// set the format specified
	if (FAILED (hResult = m_dsbPrimaryBuffer->SetFormat (&wavFormatEx)))
		return FALSE ;

	return TRUE ;
}

void SoundClass::ShutdownDirectSound ()
{
	// release primary buffer
	if (m_dsbPrimaryBuffer)
	{
		m_dsbPrimaryBuffer->Release () ;
		m_dsbPrimaryBuffer = 0 ;
	}

	// release directsound interface pointer
	if (m_dsDirectSound)
	{
		m_dsDirectSound->Release () ;
		m_dsDirectSound = 0 ;
	}
}

BOOL SoundClass::LoadWaveFile (char* szFilename, IDirectSoundBuffer8** dsb8SecondaryBuffer, BOOL bGlobal)
{
	int iError ;
	FILE* pFile ;
	UINT uiCount ;
	WaveHeader wavHeader ;
	WAVEFORMATEX wavFormatEx ;
	DSBUFFERDESC dsBufferDesc ;
	HRESULT hResult ;
	IDirectSoundBuffer* dsbTempBuffer ;
	UCHAR* ucWaveData ;
	UCHAR* ucBufferPtr ;
	DWORD dwBufferSize ;

	// open the wave file in binary
	if (iError = fopen_s (&pFile, szFilename, "rb") != 0)
		return FALSE ;

	// read in the wav file header
	if (uiCount = fread (&wavHeader, sizeof (wavHeader), 1, pFile) != 1)
		return FALSE ;

	// check chunk id is RIFF format
	if (wavHeader.szChunkId[0] != 'R' || wavHeader.szChunkId[1] != 'I' ||
		wavHeader.szChunkId[2] != 'F' || wavHeader.szChunkId[3] != 'F')
		return FALSE ;

	// check file format is WAVE format
	if (wavHeader.szFormat[0] != 'W' || wavHeader.szFormat[1] != 'A' ||
		wavHeader.szFormat[2] != 'V' || wavHeader.szFormat[3] != 'E')
		return FALSE ;

	// check sub chunk id is fmt format
	if (wavHeader.szSubChunkId[0] != 'f' || wavHeader.szSubChunkId[1] != 'm' ||
		wavHeader.szSubChunkId[2] != 't')
		return FALSE ;

	if (wavHeader.usAudioFormat != WAVE_FORMAT_PCM)
		return FALSE ;

	// check audio is stereo format
	if (wavHeader.usNumChannels != 2)
		return FALSE ;

	// check data rate of 44.1KHz
	if (wavHeader.dwSampleRate != 44100)
		return FALSE ;

	// check if recorded in 16-bit format
	if (wavHeader.usBitsPerSamp != 16)
		return FALSE ;

	// check data chunk header
	if (wavHeader.szDataChunkId[0] != 'd' || wavHeader.szDataChunkId[1] != 'a' ||
		wavHeader.szDataChunkId[2] != 't' || wavHeader.szDataChunkId[3] != 'a')
		return FALSE ;

	// set format of secondary buffer
	wavFormatEx.wFormatTag = WAVE_FORMAT_PCM ;
	wavFormatEx.nSamplesPerSec = 44100 ;
	wavFormatEx.wBitsPerSample = 16 ;
	wavFormatEx.nChannels = 2 ;
	wavFormatEx.nBlockAlign = (wavFormatEx.wBitsPerSample / 8) * wavFormatEx.nChannels ;
	wavFormatEx.nAvgBytesPerSec = wavFormatEx.nSamplesPerSec * wavFormatEx.nBlockAlign ;
	wavFormatEx.cbSize = 0 ;

	// set description of secondary buffer
	dsBufferDesc.dwSize = sizeof (DSBUFFERDESC) ;
	dsBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | (bGlobal ? DSBCAPS_GLOBALFOCUS : 0) ; // to get volume control
	dsBufferDesc.dwBufferBytes = wavHeader.dwDataSize ;
	dsBufferDesc.dwReserved = 0 ;
	dsBufferDesc.lpwfxFormat = &wavFormatEx ;
	dsBufferDesc.guid3DAlgorithm = GUID_NULL ;

	// temporary buffer must pass two tests before releasing and using
	// create temporary sound buffer with above settings
	if (FAILED (hResult = m_dsDirectSound->CreateSoundBuffer (&dsBufferDesc, &dsbTempBuffer, NULL)))
		return FALSE ;

	// test sound buffer against direct sound 8 interface
	if (FAILED (hResult = dsbTempBuffer->QueryInterface (IID_IDirectSoundBuffer8, reinterpret_cast<LPVOID*> (&*dsb8SecondaryBuffer))))
		return FALSE ;

	dsbTempBuffer->Release () ;
	dsbTempBuffer = 0 ;

	// move to beginning of wave data which is at the end
	fseek (pFile, sizeof (WaveHeader), SEEK_SET) ;

	// temporarily hold wave buffer data
	ucWaveData = new UCHAR[wavHeader.dwDataSize] ;
	if (!ucWaveData)
		return FALSE ;

	// read into temporary buffer
	if (uiCount = fread (ucWaveData, 1, wavHeader.dwDataSize, pFile) != wavHeader.dwDataSize)
		return FALSE ;

	// close wav file
	if ((iError = fclose (pFile)) != 0)
		return FALSE ;

	// lock secondary buffer to write data into it
	if (FAILED (hResult = (*dsb8SecondaryBuffer)->Lock (0, wavHeader.dwDataSize, reinterpret_cast<LPVOID*> (&ucBufferPtr),
		static_cast<DWORD*> (&dwBufferSize), NULL, 0, 0)))
		return FALSE ;

	// copy wave data into buffer
	memcpy (ucBufferPtr, ucWaveData, wavHeader.dwDataSize) ;

	// unlock secondary buffer to write data into it
	if (FAILED (hResult = (*dsb8SecondaryBuffer)->Unlock (reinterpret_cast<LPVOID> (ucBufferPtr),
		dwBufferSize, NULL, 0)))
		return FALSE ;

	// release data since it is saved into secondary buffer now
	delete [] ucWaveData ;
	ucWaveData = 0 ;

	return TRUE ;
}

void SoundClass::ShutdownWaveFile (IDirectSoundBuffer8** dsb8SecondaryBuffer)
{
	if (*dsb8SecondaryBuffer)
	{
		(*dsb8SecondaryBuffer)->Release () ;
		*dsb8SecondaryBuffer = 0 ;
	}
}

BOOL SoundClass::PlayWaveFile (IDirectSoundBuffer8** dsb8SecondaryBuffer, int iPosition, int iAttenuation)
{
	HRESULT hResult ;

	// set position at beginning of sound buffer
	if (FAILED (hResult = (*dsb8SecondaryBuffer)->SetCurrentPosition (iPosition)))
		return FALSE ;

	// set volume of buffer to 100%
	if (FAILED (hResult = (*dsb8SecondaryBuffer)->SetVolume (iAttenuation)))
		return FALSE ;

	// play contents of secondary sound buffer
	if (FAILED (hResult = (*dsb8SecondaryBuffer)->Play (0, 0, 0)))
		return FALSE ;

	return TRUE ;
}