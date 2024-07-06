#ifndef ENV5_MICROPHONE_HEADER
#define ENV5_MICROPHONE_HEADER

#include <stddef.h>
#include <stdint.h>

#include <Gpio.h>

/*!
 * @brief setup adc for microphone
 * @param gpio[in] input pin for raw data (GPIO26 to GPIO29)
 */
void microphoneInitialize(gpioPin_t gpio);

/*!
 * @brief set the required sampling rate for the ADC
 * @param samplingRate[in] sampling rate in Hertz
 */
void microphoneSetSamplingRate(uint32_t samplingRate);

/*!
 * @brief capture specified number of samples
 * @param sampleBuffer[inout] buffer to store samples
 * @param samples[in] number of samples to capture
 * @param source[in] gpio for raw data input (GPIO26 to GPIO29)
 */
void microphoneCapture(uint8_t *sampleBuffer, size_t samples, gpioPin_t source);

#endif // ENV5_MICROPHONE_HEADER
