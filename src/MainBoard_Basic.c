#define SOURCE_FILE "MAIN-BOARD-BASIC"

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

int main(void) {
    env5HwInit();
    initializeIo();

    UNITY_BEGIN();

    RUN_TEST(test_McuBlinkLed);
    RUN_TEST(test_Powersensor1);
    RUN_TEST(test_Powersensor2);
    RUN_TEST(test_Flash);
    RUN_TEST(test_Fpga);

    UNITY_END();

    reset_usb_boot(0, 0);
}
