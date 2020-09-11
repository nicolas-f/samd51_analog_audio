#include "Arduino.h"
#include "SAMD51_AnalogAudio.h"
#include"TFT_eSPI.h"
#include "Free_Fonts.h"
#include "Spectrogram.h"

#define SAMPLE_RATE 16000
#define FFT_WINDOW_LENGTH 512
float fft_window[FFT_WINDOW_LENGTH];

TFT_eSPI tft;

TFT_eSprite spr = TFT_eSprite(&tft);  // Spectrogram canvas (must be called spr) 

Spectrogram spectrogram;

void setup() {
    Serial.begin(9600);

    tft.begin();

    tft.setRotation(3);

    tft.fillScreen(TFT_BLACK);
    
    tft.setFreeFont(FS9); //select Free, Serif, 12pt.

    //tft.drawString("Serif 9pt",70,140);//prints string at (70,140) 
 

    spectrogram.init(SAMPLE_RATE, FFT_WINDOW_LENGTH)
    .origin(0, 0)
    .width(tft.width())
    .height(tft.height())
    .drawLegend();
}

void loop() {
    memset(fft_window, 0, sizeof(float) * FFT_WINDOW_LENGTH);
    fft_window[50] = 1;
    spectrogram.drawSpectrum(fft_window);
}