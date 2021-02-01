#include <tls.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#ifdef DATA_FROM_FILE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

#include "language_layer.h"
#include "json.c"
#include "settings.h"

#define HOST_ADDRESS "api.openweathermap.org"
#define HOST_PORT    "443" // default https port

// API: https://openweathermap.org/api/one-call-api

internal char *
TimeToString(time_t Value, char *Format)
{
	local_persist char Buffer[64] = { 0 };
	strftime(Buffer, sizeof(Buffer), Format, localtime(&Value));
	return (Buffer);
}

int
main(void)
{
	simplified_json_object *Response = malloc(MEBIBYTES(1));

#ifdef DATA_FROM_FILE
	int FD = open("example_result.txt", O_RDONLY);
	assert(FD > 0);
	void *JsonResultBuffer = mmap(0, MEBIBYTES(1), PROT_READ, MAP_PRIVATE, FD, 0);
	assert(JsonResultBuffer);
	Response = JsonParseStringIntoTraversableObject(JsonResultBuffer, Response);
#else
	// Make request and put response into json buffer
	{
		size_t TempBufferSize = MEBIBYTES(1);
		char *TempBuffer = malloc(TempBufferSize);

		struct tls *TlsContext = tls_client();
		int ConnectResult = tls_connect(TlsContext, HOST_ADDRESS, HOST_PORT);
		assert(0 == ConnectResult);

		// Send Request
		{
			int BytesToSend = snprintf(TempBuffer, TempBufferSize,
			                           "GET /data/2.5/onecall?lat=%s&lon=%s&appid=%s&units=%s&exclude=minutely,alerts\r\n"
			                           "HOST: %s\r\n"
			                           "\r\n",
			                           LOCATION_LATITUDE, LOCATION_LONGITUDE,
			                           API_TOKEN, UNITS,
			                           HOST_ADDRESS);
			ssize_t BytesWritten = tls_write(TlsContext, TempBuffer, (size_t)BytesToSend);
			assert(BytesWritten == BytesToSend);
		}

		// Get response
		{
			ssize_t TotalBytesReceived = 0;
			ssize_t BytesJustReceived = 0;
			do
			{
				BytesJustReceived = tls_read(TlsContext, TempBuffer + TotalBytesReceived, 
				                             TempBufferSize - (size_t)TotalBytesReceived);
				TotalBytesReceived += BytesJustReceived;
			} while(BytesJustReceived != 0);
			TempBuffer[TotalBytesReceived++] = 0;
		}

		tls_close(TlsContext);
		Response = JsonParseStringIntoTraversableObject(TempBuffer, Response);
		free(TempBuffer);
	}
#endif
	
	simplified_json_object *CurrentWeatherObject = JsonFindDataByNameBreadthFirst(Response, "current");
	time_t TimeCurrent      = atoll(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "dt"));
	time_t TimeSunrise      = atoll(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "sunrise"));
	time_t TimeSunset       = atoll(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "sunset"));
	int    CurrentTemp      = atoi(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "temp"));
	int    CurrentTempFeels = atoi(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "feels_like"));
	char *WeatherDescription = JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "description");
	f64    WeatherSpeed     = atof(JsonFindDataByNameBreadthFirst(CurrentWeatherObject, "wind_speed"));
	
	printf("\nResponse timestamp: %s\n",  TimeToString(TimeCurrent, "%Y-%m-%d %H:%M:%S"));
	printf("Temperature:       %3d°C\n", CurrentTemp);
	printf("Temperature Feels: %3d°C\n", CurrentTempFeels);
	printf("Weather: %s\n", WeatherDescription);
	printf("Wind speed: %5.2f m/s\n", WeatherSpeed);
	printf("Sunrise:    %s\n", TimeToString(TimeSunrise, "%H:%M"));
	printf("Sunset:     %s\n", TimeToString(TimeSunset, "%H:%M"));

	free(Response);
	return (0);
}
