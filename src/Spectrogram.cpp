#include "Spectrogram.h";

extern TFT_eSprite spr;

Spectrogram* Spectrogram::height(int height) {
    this->canvasHeight = height;
    return this;
}

Spectrogram* Spectrogram::width(int width) {
    this->canvasWidth = width;
}

Spectrogram* Spectrogram::origin(int x, int y) {
    this->originX = x;
    this->originY = y;
}

Spectrogram* Spectrogram::drawSpectrum(float* spectrum) {

}

Spectrogram* Spectrogram::drawLegend() {
    float hertzBySpectrumCell = rate / (float)spectrum_window_length;
    float fmax = rate;
    float fmin = 0;
    float cellByPixel = spectrum_window_length / (float)canvasHeight;
    for(int i = 0; i < vertical_ticks; i++) {
        
    }
}

Spectrogram* Spectrogram::clear() {

}

Spectrogram* Spectrogram::init(int rate, int spectrum_window_length) {
    this->rate = rate;
    this->spectrum_window_length = spectrum_window_length;
}