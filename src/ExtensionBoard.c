#define SOURCE_FILE "EXTENSION-BOARD"

#include "Adxl345b.h"
#include "Adxl345bTypedefs.h"
#include "Common.h"
#include "Esp.h"
#include "Microphone.h"
#include "Network.h"
#include "enV5HwController.h"

#include "unity.h"
#include <hardware/i2c.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>

#define ADXL345B_I2C i2c1
#define ADXL345B_ADDRESS ADXL345B_I2C_ALTERNATE_ADDRESS

#define MICRO_GPIO 26
#define MICRO_SAMPLING_RATE 16000
#define MICRO_SAMPLE_COUNT 512

static void initializeIo(void) {
    stdio_init_all();
    while ((!stdio_usb_connected())) {
        // Wait for Serial Connection
    }
}

void setUp(void) {}
void tearDown(void) {}

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

int main() {
    env5HwInit();
    initializeIo();

    UNITY_BEGIN();

    RUN_TEST(test_EspModule);
    RUN_TEST(test_Accelerometer);
    RUN_TEST(test_Amplifier);

    UNITY_END();

    reset_usb_boot(0, 0);
}
