#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define MAXTIMINGS	85
#define DHTPIN		7
#define DHTMODEL	22
int dhtXX_dat[5] = { 0, 0, 0, 0, 0 };

static uint8_t sizecvt(const int read)
{
    /*digitalread() and friends from wiringPi are defined as returning a value
      <256. However, they returned it as int(). this is a safety function */
    if(read > 255 || read < 0)
    {
        printf("Invalid data from wiringPi library\n");
	exit(EXIT_FAILURE);
    }
    return (uint8_t)read;
} 
void writeData(uint8_t h0, uint8_t h1, uint8_t t0, uint8_t t1) {
 	char *data_str = (char*)malloc(255 * sizeof(char));
	int status;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(data_str,"curl -X POST -H \"Content-Type: application/json\"  -d '{\"t\":\"%d.%d\",\"h\":\"%d.%d\",\"d\":\"%d-%d-%d %d:%d:%d\"}' http://192.168.1.3:8888/api/weather/temphumidity/add\n", t0,t1,h0,h1, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	status = system(data_str);
	if (status!=0) {
	  FILE *f = fopen("data.json", "a");
	  if (f == NULL)
	  {
	    printf("Error opening file!\n");
	    exit(1);
	  }
	  fprintf(f, "{\"t\":\"%d.%d\",\"h\":\"%d.%d\",\"d\":\"%d-%d-%d %d:%d:%d\"}\n", t0,t1,h0,h1, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	  fclose(f);
	}
}
void writeData22(float  humidity, float tempC) {
 	char *data_str = (char*)malloc(255 * sizeof(char));
	int status;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(data_str,"curl -X POST -H \"Content-Type: application/json\"  -d '{\"t\":\"%.2f\",\"h\":\"%.2f\",\"d\":\"%d-%d-%d %d:%d:%d\"}' http://192.168.1.3:8888/api/weather/temphumidity/add\n", tempC, humidity, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	status = system(data_str);
	/*if (status!=0) {
	  FILE *f = fopen("data.json", "a");
	  if (f == NULL)
	  {
	    printf("Error opening file!\n");
	    exit(1);
	  }
	  fprintf(f, "{\"t\":\"%d.%d\",\"h\":\"%d.%d\",\"d\":\"%d-%d-%d %d:%d:%d\"}\n", t0,t1,h0,h1, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	  fclose(f);
	} */
}
/*
int read_and_remove_Line() {
	char* inFileName = "data.json";
	char* outFileName = "tmp.json";
	FILE* inFile = fopen(inFileName, "r");
	FILE* outFile = fopen(outFileName, "a");
 	char *data_str = (char*)malloc(255 * sizeof(char));
	int status;
	char line [1024]; // maybe you have to use better value here
	int lineCount = 0;
	int ret = -1;

	if( inFile == NULL )
	{
	    printf("Open Error");
	}
	
	while( fgets(line, sizeof(line), inFile) != NULL )
	{
	    if(lineCount == 0){
		//sprintf(data_str,"curl -X POST -H \"Content-Type: application/json\"  -d '%s' http://192.168.1.3:8888/api/weather/temphumidity/add\n", line);
		status = system(data_str);
		if (status==0) {
		   ret=0;
		}
	    }
	    else
	    {
	        fprintf(outFile, "%s", line);
	    }
	
	    lineCount++;
	}
	
	
	fclose(inFile);
	fclose(outFile);
	
	 //possible that you have to remove old file here before
	printf("renaming...");
	if( !rename(outFileName, inFileName) )
	{
	    printf("Rename Error");
	}
	return ret;
}
*/

void read_dhtXX_dat()
{
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	float	f; 
	double tempC;
	double humid;
	int isNegative = 0;
	dhtXX_dat[0] = dhtXX_dat[1] = dhtXX_dat[2] = dhtXX_dat[3] = dhtXX_dat[4] = 0;
 
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	pinMode( DHTPIN, INPUT );
 
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ( sizecvt(digitalRead( DHTPIN )) == laststate )
		{
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = sizecvt(digitalRead( DHTPIN ));
 
		if ( counter == 255 )
			break;
 
		if ( (i >= 4) && (i % 2 == 0) )
		{
			dhtXX_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dhtXX_dat[j / 8] |= 1;
			j++;
		}
	}
 
	if ( (j >= 40) &&
	     (dhtXX_dat[4] == ( (dhtXX_dat[0] + dhtXX_dat[1] + dhtXX_dat[2] + dhtXX_dat[3]) & 0xFF) ) )
	{
		if (DHTMODEL == 22) 
		{
		    humid = (dhtXX_dat[0] * 256 + dhtXX_dat[1]) ;
		    /*dhtXX_dat[0] = (int)humid; dhtXX_dat[1] = (humid - (int) humid)* 100.0;*/
		    tempC = ((dhtXX_dat[2] & 0x7F) * 256 + dhtXX_dat[3]);
		    if ((dhtXX_dat[2] & 0x80) != 0) isNegative = 1;
		    /*dhtXX_dat[2] = (int)tempC; dhtXX_dat[3] = (tempC - (int)tempC)* 100.0;*/
		    if (isNegative == 1) tempC *=-1; /*dhtXX_dat[2] *=-1;*/
		    printf("Humidity=%.2f %% Temperature = %.2f C\n",
			(float)(humid/10.0), (float)(tempC/10.0));
		    writeData22((float)humid/10.0, (float)tempC/10.0);
		}
		else {
		    
		    f = dhtXX_dat[2] * 9. / 5. + 32;
		    printf( "Humidity = %d.%d %% Temperature = %d.%d C (%.1f F)\n",
			dhtXX_dat[0], dhtXX_dat[1], dhtXX_dat[2], dhtXX_dat[3], f );
		    writeData(dhtXX_dat[0], dhtXX_dat[1], dhtXX_dat[2], dhtXX_dat[3]);
		}
		//while (read_and_remove_Line()==0) {}
		exit(0);
		
	}
	else  
	{
		printf( "Data not good, skip\n" );
	}
}

 
int main( void )
{
	printf( "Raspberry Pi wiringPi dhtXX Temperature test program\n" );
 
	if ( wiringPiSetup() == -1 )
		exit( 1 );
	int nAttempts = 0;
	while ( nAttempts <100 )
	{
		read_dhtXX_dat();
		delay( 1000 );
		nAttempts++; 
	}
 
	return(0);
}
