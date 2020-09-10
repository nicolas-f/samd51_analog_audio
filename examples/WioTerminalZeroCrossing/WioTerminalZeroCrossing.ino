// This example compute the zero crossing intervals in order to check the real frequency of the sampling
#include "SAMD51_AnalogAudio.h"

#define SAMPLE_RATE 16000

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  AnalogAudio.addChannel(WIO_MIC);

  AnalogAudio.init(SAMPLE_RATE);

  delay(2000); // wait for clean audio
}

// http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=be&passmode=hp&order=1&usesr=usesr&sr=16000&frequencyLow=25&noteLow=&noteHigh=&pw=pw&calctype=float&run=Send
// High pass bessel filter order=1 alpha1=0.0015625 
class  FilterBeHp1
{
	public:
		FilterBeHp1()
		{
			v[0]=0.0;
		}
	private:
		float v[2];
	public:
		float step(float x) //class II 
		{
			v[0] = v[1];
			v[1] = (9.951152004481873536e-1f * x)
				 + (0.99023040089637459626f * v[0]);
			return 
				 (v[1] - v[0]);
		}
};
FilterBeHp1 filter;

void loop() {
  // put your main code here, to run repeatedly:
  int16_t buffer[MAX_AUDIO_WINDOW_SIZE];
  int read = AnalogAudio.read(0, buffer, MAX_AUDIO_WINDOW_SIZE);
  delay(1000);  
}