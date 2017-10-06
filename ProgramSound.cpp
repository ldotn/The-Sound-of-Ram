// ProgramSound.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

// From https://stackoverflow.com/questions/1451606/programably-make-and-play-a-sound-through-speakers-c
struct WaveHeader
{
	WaveHeader(size_t NumSamples)
	{
		chunkID = 0x46464952;
		format = 0x45564157;
		subChunk1ID = 0x20746d66;
		subChunk1Size = 16;
		audioFormat = 1;
		numChannels = 2;
		sampleRate = 44100;
		bitsPerSample = 8;
		byteRate = sampleRate * numChannels * bitsPerSample/8;
		blockAlign = numChannels * bitsPerSample/8;
		subChunk2ID = 0x61746164;
		subChunk2Size = NumSamples * numChannels * bitsPerSample/8;
		chunkSize = 4 + (8 + subChunk1Size) + (8 + subChunk2Size);
	}

    DWORD chunkID;       // 0x46464952 "RIFF" in little endian
    DWORD chunkSize;     // 4 + (8 + subChunk1Size) + (8 + subChunk2Size)
    DWORD format;        // 0x45564157 "WAVE" in little endian

    DWORD subChunk1ID;   // 0x20746d66 "fmt " in little endian
    DWORD subChunk1Size; // 16 for PCM
    WORD  audioFormat;   // 1 for PCM, 3 for EEE floating point , 7 for u-law
    WORD  numChannels;   // 1 for mono, 2 for stereo
    DWORD sampleRate;    // 8000, 22050, 44100, etc...
    DWORD byteRate;      // sampleRate * numChannels * bitsPerSample/8
    WORD  blockAlign;    // numChannels * bitsPerSample/8
    WORD  bitsPerSample; // number of bits (8 for 8 bits, etc...)

    DWORD subChunk2ID;   // 0x61746164 "data" in little endian
    DWORD subChunk2Size; // numSamples * numChannels * bitsPerSample/8 (this is the actual data size in bytes)
};

static const size_t MaxSamplesCount = 10*1000*1000;

char samples[sizeof(WaveHeader) + MaxSamplesCount];

int main()
{
	cout << "PID : ";
	DWORD pid;
	cin >> pid;

	// Get handle
	auto handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pid);

	// Walk the memory of the other process
	MEMORY_BASIC_INFORMATION info;

	for(unsigned char * p = nullptr; VirtualQueryEx(handle, p, &info, sizeof(info)) == sizeof(info); p += min(info.RegionSize,MaxSamplesCount) ) 
    {
		// Load memory
		SIZE_T bytesRead;
		ReadProcessMemory(handle, p, samples + sizeof(WaveHeader), min(info.RegionSize,MaxSamplesCount), &bytesRead);

		// Play the sound
		if(bytesRead > 0)
		{
			// Update header
			*(WaveHeader*)samples = WaveHeader(bytesRead);

			cout << "Now playing : [ " << (void*)p << " , " << (void*)(p + info.RegionSize) << " ] , " << bytesRead << " samples" << endl;

			PlaySoundA(samples, NULL, SND_MEMORY);
		}
    }

    return 0;
}

