#ifndef _CONFIGS_HPP
#define _CONFIGS_HPP

namespace GlobalConfigs
{
	int width = 1920;
	int height = 1440;
	/*const int width = 720;
	const int height = 720;
	*/
	float aspect = (float)width / (float)height;
	
	const char* windowName = "Yanagi MMD Preview Image Generator";

	const int wfilenameBufferLength = 1024;
	wchar_t wfilenameBuffer[1024];
	wchar_t wStorePathBuffer[1024];
}

#endif