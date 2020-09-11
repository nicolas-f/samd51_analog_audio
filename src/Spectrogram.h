#include<TFT_eSPI.h>
#include"seeed_graphics_base.h"

#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

class Spectrogram {
    public:

        Spectrogram* height(int height);

        Spectrogram* width(int width);

        Spectrogram* origin(int x, int y);

        Spectrogram* drawSpectrum(float* spectrum);

        Spectrogram* drawLegend();

        Spectrogram* clear();

        /**
         * @rate Sample rate
         * @spectrum_window_length FFT window length
         */ 
        Spectrogram* init(int rate, int spectrum_window_length);
    private:
        int canvasHeight;
        int canvasWidth;
        int originX;
        int originY;
        int rate;
        int spectrum_window_length;
        int vertical_ticks = 8;
        int horizontal_ticks = 12;
};

#endif