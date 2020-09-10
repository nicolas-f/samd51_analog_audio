/*
  SAMD51_AnalogAudio.h
  Atmel SAMD51 read 12 bits analog and convert to audio samples buffer
  
  Currently use the TC3 timer. So no other TC3 timer allowed.
  I tried DMA for writing bunch of samples into a buffer but without success so far.
  @author Nicolas Fortin (Univ. Gustave Eiffel)
*/

#ifndef SAMD51_Analog_Audio_h
#define SAMD51_Analog_Audio_h

// Internal cache size of audio samples, you can increase if you have heavy duty between read() calls 
#ifndef MAX_AUDIO_WINDOW_SIZE
#define MAX_AUDIO_WINDOW_SIZE 512
#endif

class AnalogAudioClass {
  public:
  
    /**
     * Init collecting the audio samples. Call this after adding channels.
     * @rate Sample rate. Need ~22 usec for reading analog so max frequency is 44100 khz
     */
    void init(int rate);
    
    /**
     * Init collecting the audio samples
     * @pin pin number
     * @return channel identifier
     */
    int32_t addChannel(uint32_t pin);

    /**
     * Read samples
     * @samples sample buffer
     * @len maximum buffer length
     * @return samples read
     */
    int32_t read(int32_t channel, int16_t* samples, int len);
    
    /**
     * Stop collecting analog samples and delete all channels
     */
    void stop();    
};

extern AnalogAudioClass AnalogAudio;

#endif
