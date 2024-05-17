#define SOURCE_FILE "MICROPHONE"

#include "include/Microphone.h"

#include <hardware/adc.h>

#ifndef PICO_CLOCK_FREQUENCY
//! Pico Clock defaults to 48MHz
#define PICO_CLOCK_FREQUENCY (48.0f * 1000000)
#endif

void microphoneIntialize(gpioPin_t gpio) {
    adc_init();
    adc_gpio_init(gpio);
}

void microphoneSetSamplingRate(uint32_t samplingRate) {
    float clock_div = PICO_CLOCK_FREQUENCY / (float)samplingRate;
    adc_set_clkdiv(clock_div);
}

static inline uint8_t adcSource(gpioPin_t source) {
    switch (source) {
    case 26:
        return 0;
    case 27:
        return 1;
    case 28:
        return 2;
    case 29:
    default:
        return 3;
    }
}
void microphoneCapture(uint8_t *sampleBuffer, size_t samples, gpioPin_t source) {
    adc_run(false);

    adc_select_input(adcSource(source));
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (size_t sample = 0; sample < samples; sample++) {
        sampleBuffer[sample] = adc_fifo_get_blocking();
    }
    adc_run(false);
    adc_fifo_drain();
}
