// This example compute the zero crossing intervals in order to check the real frequency of the sampling
// So for a sample rate of 16000 Hz, and a tone of 1000 Hz you should have 16 samples between each zero crossing
#include "SAMD51_AnalogAudio.h"

#define SAMPLE_RATE 16000

void setup() {
  Serial.begin(115200);

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

int lastPrint = 0;

void loop() {
  // read audio samples
  int16_t buffer[MAX_AUDIO_WINDOW_SIZE];
  int read = AnalogAudio.read(0, buffer, MAX_AUDIO_WINDOW_SIZE);
  // Find first zero cross
  int cursor = 1;
  int firstZeroCross = 0;
  int secondZeroCross = 0;
  float oldValue = filter.step(buffer[0]);
  float newValue;
  while(cursor < read && firstZeroCross == 0) {
    newValue = filter.step(buffer[cursor]);
    if(oldValue >= 0 && newValue < 0) {
      firstZeroCross = cursor;
    }
    oldValue = newValue;
    cursor += 1;  
  }  
  // Find second zero cross
  cursor = firstZeroCross + 1;
  while(cursor < read) {
    newValue = filter.step(buffer[cursor]);
    if(oldValue >= 0 && newValue < 0) {
      secondZeroCross = cursor;
      break;
    }
    oldValue = newValue;
    cursor += 1;  
  }  
  unsigned long now = millis();
  if(now - lastPrint > 1000) {
    Serial.print("Got ");
    Serial.print(secondZeroCross - firstZeroCross);
    Serial.println(" samples between each zero crossing");
    lastPrint = now;
  }
  delay(30);  
}