#define SOURCE_FILE "MAIN-BOARD-FULL"

#include "Common.h"
#include "Flash.h"
#include "Pac193x.h"
#include "enV5HwController.h"
#include "middleware.h"

#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>

#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

static void initializeIo(void) {
    stdio_init_all();
    while ((!stdio_usb_connected())) {
        // Wait for Serial Connection
    }
}

static void test_McuBlinkLed(void) {
    env5HwLedsAllOn();
    PRINT("Please press 'o' if the MCU leds are on!");
    int result;
    do {
        result = getchar_timeout_us(UINT32_MAX);
    } while (result == PICO_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_CHAR('o', (char)result);
}

static void test_Powersensor1(void) {
    // TODO: initialize
    // TODO: get ID
    // TODO: get Measurements
}
static void test_Powersensor2(void) {
    // TODO: initialize
    // TODO: get ID
    // TODO: get Measurements
}

static void test_Flash(void) {
    // TODO: Write
    // TODO: Read
}

static void test_Fpga(void) {
    // TODO: Powered On
    // TODO: Reconfigure
    // TODO: Blink LED
    // TODO: Send/Receive
}

static void test_EspModule(void) {
    espInit();
    PRINT_DEBUG("ESP initialized!");
    TEST_ASSERT_EQUAL_UINT8(NETWORK_NO_ERROR, networkTryToConnectToNetworkUntilSuccessful());
}

static void test_Accelerometer(void) {
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR, adxl345bInit(ADXL345B_I2C, ADXL345B_ADDRESS));

    uint8_t serialNumber = 0;
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR, adxl345bReadSerialNumber(&serialNumber));
    TEST_ASSERT_EQUAL_UINT8(0xE5, serialNumber);

    int delta_x, delta_y, delta_z;
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR,
                            adxl345bPerformSelfTest(&delta_x, &delta_y, &delta_z));
    PRINT_DEBUG("  X: %iLSB, Y: %iLSB, Z: %iLSB", delta_x, delta_y, delta_z);
}

static void test_Amplifier(void) {
    microphoneIntialize(MICRO_GPIO);
    PRINT_DEBUG("Amplifier initialized");
    microphoneSetSamplingRate(MICRO_SAMPLING_RATE);
    PRINT_DEBUG("Sampling-rate set to %u", MICRO_SAMPLING_RATE);

    uint8_t samples[MICRO_SAMPLE_COUNT];
    microphoneCapture(samples, MICRO_SAMPLE_COUNT, MICRO_GPIO);
    PRINT_BYTE_ARRAY("Microphone samples:", samples, MICRO_SAMPLE_COUNT);
}

int main(void) {
    env5HwInit();
    initializeIo();

    UNITY_BEGIN();

    RUN_TEST(test_McuBlinkLed);
    RUN_TEST(test_Powersensor1);
    RUN_TEST(test_Powersensor2);
    RUN_TEST(test_Flash);
    RUN_TEST(test_Fpga);

    RUN_TEST(test_EspModule);
    RUN_TEST(test_Accelerometer);
    RUN_TEST(test_Amplifier);

    UNITY_END();

    reset_usb_boot(0, 0);
}
