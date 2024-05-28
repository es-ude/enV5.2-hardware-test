/*!
 * @IMPORTANT
 *   To Run the implemented tests a python script is required.
 *   This script provides functions to automate the testing of the flash/fpga.
 */

#define SOURCE_FILE "MAIN-BOARD-BASIC"

#include "Common.h"
#include "Esp.h"
#include "Flash.h"
#include "FpgaConfigurationHandler.h"
#include "I2cTypedefs.h"
#include "Network.h"
#include "Pac193x.h"
#include "SpiTypedefs.h"
#include "enV5HwController.h"

#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>

#include "unity.h"

bool flashOK = false;

/* region Powersensor config */
#define I2C_INSTANCE i2c1

static pac193xSensorConfiguration_t sensor1 = {
    .i2c_host = I2C_INSTANCE,
    .i2c_slave_address = PAC193X_I2C_ADDRESS_499R,
    .powerPin = -1,
    .usedChannels = {.uint_channelsInUse = 0b00001111},
    .rSense = {0.82f, 0.82f, 0.82f, 0.82f},
};

static pac193xSensorConfiguration_t sensor2 = {
    .i2c_host = I2C_INSTANCE,
    .i2c_slave_address = PAC193X_I2C_ADDRESS_806R,
    .powerPin = -1,
    .usedChannels = {.uint_channelsInUse = 0b00001111},
    .rSense = {0.82f, 0.82f, 0.82f, 0.82f},
};
/* endregion Powersensor config */
/* region Flash/FPGA config */
#define SPI_CS_PIN 1
#define SPI_INSTANCE spi0
#define SPI_MISO_PIN 0
#define SPI_MOSI_PIN 3
#define SPI_CLOCK_PIN 2

spi_t flashConfig = {.spi = SPI_INSTANCE,
                     .sckPin = SPI_CLOCK_PIN,
                     .misoPin = SPI_MISO_PIN,
                     .mosiPin = SPI_MOSI_PIN,
                     .baudrate = 1000000};

spi_t fpgaConfig = {.spi = SPI_INSTANCE,
                    .sckPin = SPI_CLOCK_PIN,
                    .misoPin = SPI_MISO_PIN,
                    .mosiPin = SPI_MOSI_PIN,
                    .baudrate = 5000000};

char urlSlow[] = "http://192.168.178.74:5000/getslow";
char urlFast[] = "http://192.168.178.74:5000/getfast";
/* region S15 config */
size_t slowBinfileLength = 85540;
size_t fastBinfileLength = 86116;
/* endregion S15 config */
/* region S50 config */
// size_t slowBinfileLength = 231608;
// size_t fastBinfileLength = 232360;
/* endregion S50 config */
/* endregion Flash/FPGA config */

void setUp(void) {}
void tearDown(void) {}

static void initializeIo(void) {
    stdio_init_all();
    while ((!stdio_usb_connected())) {
        // Wait for Serial Connection
    }
    sleep_ms(1000);
}

static void test_McuBlinkLed(void) {
    env5HwLedsAllOn();
    PRINT("Please press 'o' if the MCU leds are on!");
    int result;
    do {
        result = getchar_timeout_us(UINT32_MAX);
    } while (result == PICO_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_CHAR('o', (char)result);
    env5HwLedsAllOff();
}

static void test_PowerSensor(pac193xSensorConfiguration_t sensorToTest) {
    /* region initialize */
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(PAC193X_NO_ERROR, pac193xPowerUpSensor(sensorToTest),
                                    "Power Up Failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(PAC193X_NO_ERROR, pac193xInit(sensorToTest),
                                    "Initialization failed");
    /* endregion initialize */

    /* region ID */
    pac193xSensorId_t id;
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(PAC193X_NO_ERROR, pac193xGetSensorInfo(sensorToTest, &id),
                                    "Get ID failed");
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0x58, id.product_id, "Product ID to low");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(0x5B, id.product_id, "Product ID to high");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x5D, id.manufacturer_id, "Manufacturer ID mismatch");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x03, id.revision_id, "Revision ID mismatch");
    /* endregion ID */

    // TODO: get Measurements
}
static void test_PowerSensor1(void) {
    test_PowerSensor(sensor1);
}
static void test_PowerSensor2(void) {
    test_PowerSensor(sensor2);
}

static void test_Flash(void) {
    /* region initialize */
    env5HwFpgaPowersOff();
    flashInit(&flashConfig, SPI_CS_PIN);
    /* endregion initialize */

    /* region ID */
    uint8_t data[3];
    data_t idBuffer = {.data = data, .length = sizeof(data)};
    TEST_ASSERT_EQUAL_INT(sizeof(data), flashReadId(&idBuffer));
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, data[0], "Manufacture ID mismatch");
    uint16_t deviceId = data[1];
    deviceId = deviceId << 8 | data[2];
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x0219, deviceId, "Device ID mismatch");
    /* endregion ID */

    /* region data */
    // Erase Sector
    TEST_ASSERT_EQUAL_UINT8(FLASH_NO_ERROR, flashEraseSector(0x00000000));

    // write test data
    uint8_t testWrite[FLASH_BYTES_PER_PAGE];
    for (size_t i = 0; i < sizeof(testWrite); i++) {
        testWrite[i] = 0xAB;
    }
    TEST_ASSERT_EQUAL_INT(sizeof(testWrite),
                          flashWritePage(FLASH_BYTES_PER_PAGE, testWrite, sizeof(testWrite)));

    // read test data
    uint8_t testRead[FLASH_BYTES_PER_PAGE];
    data_t readBuffer = {.data = testRead, .length = sizeof(testWrite)};
    TEST_ASSERT_EQUAL_INT(sizeof(testWrite), flashReadData(FLASH_BYTES_PER_PAGE, &readBuffer));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testWrite, testRead, sizeof(testWrite));
    /* endregion data */

    flashOK = true;
}

static void test_BlinkFpga(char *url, size_t length) {
    env5HwFpgaPowersOff();
    fpgaConfigurationHandlerDownloadConfigurationViaHttp(url, length, 1);
    sleep_ms(500);
    env5HwFpgaPowersOn();
    PRINT("Please press 'o' if the FPGA leds are on!");
    int result;
    do {
        result = getchar_timeout_us(UINT32_MAX);
    } while (result == PICO_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_CHAR('o', (char)result);
    env5HwFpgaPowersOff();
}
static void test_Fpga(void) {
    flashInit(&flashConfig, SPI_CS_PIN);
    espInit();
    while (ESP_NO_ERROR != espSendCommand("AT+CWMODE=1", "OK", 100))
        ;
    networkTryToConnectToNetworkUntilSuccessful();

    test_BlinkFpga(urlSlow, slowBinfileLength);
    test_BlinkFpga(urlFast, fastBinfileLength);

    // TODO: Reconfigure
    // TODO: Send/Receive
}

int main(void) {
    env5HwInit();
    initializeIo();

    UNITY_BEGIN();

    RUN_TEST(test_McuBlinkLed);
    RUN_TEST(test_PowerSensor1);
    RUN_TEST(test_PowerSensor2);
    RUN_TEST(test_Flash);
    if (flashOK) {
        RUN_TEST(test_Fpga);
    }

    UNITY_END();

    reset_usb_boot(0, 0);
}
