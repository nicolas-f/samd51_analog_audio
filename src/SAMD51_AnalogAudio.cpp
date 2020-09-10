/*
  SAMD51_AnalogAudio.h
  Atmel SAMD51 read 12 bits analog and convert to audio samples buffer
  @author Nicolas Fortin (Univ. Gustave Eiffel)

  TC3 calls from Dennis van Gils library https://github.com/Dennis-van-Gils/SAMD51_InterruptTimer
*/
#include "Arduino.h"
#include "SAMD51_AnalogAudio.h"
#include "wiring_private.h"  // pinPeripheral() function


#define CLOCK_HZ 48000000
     

typedef struct _channel_t {
    Adc *adc;
		int16_t input_buffer[MAX_AUDIO_WINDOW_SIZE];
		int64_t input_buffer_feed_cursor;
		int64_t input_buffer_consume_cursor;
} channel_t;

 channel_t* channels = NULL;
 int channels_length = 0;


void channel_t_init(channel_t* self, uint32_t pin) {

#if defined(PIN_A6)
  if (pin == 6) {
    pin = PIN_A6;
  } else
#endif
#if defined(PIN_A7)
    if (pin == 7) {
      pin = PIN_A7;
    } else
#endif
      if (pin <= 5) {
        pin += A0;
      }
  pinMode(pin, INPUT);
  pinPeripheral(pin, PIO_ANALOG);

  if (g_APinDescription[pin].ulPinAttribute & PIN_ATTR_ANALOG) self->adc = ADC0;
  else if (g_APinDescription[pin].ulPinAttribute & PIN_ATTR_ANALOG_ALT) self->adc = ADC1;
  else return;

  while ( self->adc->SYNCBUSY.reg & ADC_SYNCBUSY_INPUTCTRL ); //wait for sync
  self->adc->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber; // Selection for the positive ADC input

  // Control A
  /*
     Bit 1 ENABLE: Enable
       0: The ADC is disabled.
       1: The ADC is enabled.
     Due to synchronization, there is a delay from writing CTRLA.ENABLE until the peripheral is enabled/disabled. The
     value written to CTRL.ENABLE will read back immediately and the Synchronization Busy bit in the Status register
     (STATUS.SYNCBUSY) will be set. STATUS.SYNCBUSY will be cleared when the operation is complete.

     Before enabling the ADC, the asynchronous clock source must be selected and enabled, and the ADC reference must be
     configured. The first conversion after the reference is changed must not be used.
  */
  while ( self->adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  self->adc->CTRLA.bit.ENABLE = 0x01;             // Enable ADC

  // Start conversion
  while ( self->adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync

  self->adc->SWTRIG.bit.START = 1;
}


/**
   Read samples
   @samples sample buffer
   @len maximum buffer length
   @return samples read
*/
int channel_t_read(channel_t* self, int16_t* samples, int len) {
    // keep up with missed samples
    self->input_buffer_consume_cursor = max(self->input_buffer_consume_cursor, self->input_buffer_feed_cursor - MAX_AUDIO_WINDOW_SIZE);
    // copy samples
    int samples_read = 0;
    while(samples_read < len && self->input_buffer_consume_cursor + samples_read < self->input_buffer_feed_cursor) {
      samples[samples_read] = self->input_buffer[(self->input_buffer_consume_cursor + samples_read) % MAX_AUDIO_WINDOW_SIZE];
      samples_read++;
    }
    self->input_buffer_consume_cursor += samples_read;
    return samples_read;
}

void channel_t_analog_read(channel_t* self) {
    // Clear the Data Ready flag
    self->adc->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    // Start conversion again, since The first conversion after the reference is changed must not be used.
    self->adc->SWTRIG.bit.START = 1;

    // Store the value
    while (self->adc->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete

    self->input_buffer[self->input_buffer_feed_cursor % MAX_AUDIO_WINDOW_SIZE] = self->adc->RESULT.reg;
    self->input_buffer_feed_cursor += 1;
}

/**
   Stop collecting analog samples
*/
void AnalogAudioClass::stop() {
  // Stop TC3
  TC3->COUNT16.CTRLA.bit.ENABLE = 0;

  // Wait time before removing pointers
  delay(125);

  for(int i=0; i < channels_length; i++) {
    while ( channels[i].adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
    channels[i].adc->CTRLA.bit.ENABLE = 0x00;                     // Disable ADC
    while ( channels[i].adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  }
  free(channels);
  channels_length = 0;
}

static inline void TC3_wait_for_sync() {
  while (TC3->COUNT16.SYNCBUSY.reg != 0) {}
}

void AnalogAudioClass::init(int rate) {

  // Enable the TC bus clock, use clock generator 0
  GCLK->PCHCTRL[TC3_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK1_Val |
                                   (1 << GCLK_PCHCTRL_CHEN_Pos);
  while (GCLK->SYNCBUSY.reg > 0);

  TC3->COUNT16.CTRLA.bit.ENABLE = 0;
  
  // Use match mode so that the timer counter resets when the count matches the
  // compare register
  TC3->COUNT16.WAVE.bit.WAVEGEN = TC_WAVE_WAVEGEN_MFRQ;
  TC3_wait_for_sync();
  
   // Enable the compare interrupt
  TC3->COUNT16.INTENSET.reg = 0;
  TC3->COUNT16.INTENSET.bit.MC0 = 1;

  // Enable IRQ
  NVIC_EnableIRQ(TC3_IRQn);
  
  uint32_t TC_CTRLA_PRESCALER_DIVN;

  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV1024;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV256;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV64;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV16;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV4;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV2;
  TC3_wait_for_sync();
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_PRESCALER_DIV1;
  TC3_wait_for_sync();

  TC_CTRLA_PRESCALER_DIVN = TC_CTRLA_PRESCALER_DIV1;
    
  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIVN;
  TC3_wait_for_sync();

  int compareValue = CLOCK_HZ / rate;

  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC3->COUNT16.COUNT.reg = map(TC3->COUNT16.COUNT.reg, 0,
                               TC3->COUNT16.CC[0].reg, 0, compareValue);
  TC3->COUNT16.CC[0].reg = compareValue;
  TC3_wait_for_sync();

  TC3->COUNT16.CTRLA.bit.ENABLE = 1;
  TC3_wait_for_sync();
}

int32_t AnalogAudioClass::addChannel(uint32_t pin) {
  // Copy old channels
  channel_t* newChannels = (channel_t*)malloc(sizeof(channel_t) * (channels_length + 1));
  for(int i = 0; i < channels_length; i++) {
    newChannels[i] = channels[i];
  }
  // init new channel
  channel_t_init(newChannels + channels_length, pin);
  channels_length++;
  if(channels != NULL) {
    free(channels);
  }
  channels = newChannels;
}

int32_t AnalogAudioClass::read(int32_t channel, int16_t* samples, int len){
  if(channel < 0 || channel > channels_length) {
    return 0;
  }
  return channel_t_read(channels + channel, samples, len);
}

AnalogAudioClass AnalogAudio;

void TC3_Handler() {
  // If this interrupt is due to the compare register matching the timer count
  if (TC3->COUNT16.INTFLAG.bit.MC0 == 1) {
    TC3->COUNT16.INTFLAG.bit.MC0 = 1;
    for(int i = 0; i < channels_length; i++) {
      channel_t_analog_read(channels + i);
    }
  }
}


