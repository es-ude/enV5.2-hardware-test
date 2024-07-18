/*!
 * @IMPORTANT
 *  This test requires the scripts/BinFileUpload.py script to be executed
 *  and your firewall to open port 5000
 *
 * @TIP
 *  Make sure your computer and the Elastic Node are connected to the same LAN
 */

#define SOURCE_FILE "MAIN-BOARD-FULL"

#include "unity.h"
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>

#include "Adxl345b.h"
#include "Common.h"
#include "EnV5HwConfiguration.h"
#include "EnV5HwController.h"
#include "Esp.h"
#include "Flash.h"
#include "FlashTypedefs.h"
#include "FpgaConfigurationHandler.h"
#include "Microphone.h"
#include "Network.h"
#include "Pac193x.h"
#include "Spi.h"
#include "I2c.h"

bool flashOK = false;
bool espOK = false;

i2cConfiguration_t i2cConfig = {
    .i2cInstance = I2C_INSTANCE,
    .sclPin = I2C_SCL_PIN,
    .sdaPin = I2C_SDA_PIN,
    .frequency = I2C_FREQUENCY_IN_HZ,
};

/* region ADXL345B config */
adxl345bSensorConfiguration_t adxl345b = {
    .i2c_host = ADXL_HOST,
    .i2c_slave_address = ADXL_SLAVE,
};
/* endregion ADXL345B config */

/* region Amplifier config */
#define MICRO_GPIO 26
#define MICRO_SAMPLING_RATE 16000
#define MICRO_SAMPLE_COUNT 512
/* endregion Amplifier config */

/* region Powersensor config */
pac193xSensorConfiguration_t sensor1 = {
    .i2c_host = PAC_ONE_HOST,
    .i2c_slave_address = PAC_ONE_SLAVE,
    .powerPin = PAC_ONE_POWER_PIN,
    .usedChannels = PAC_ONE_USED_CHANNELS,
    .rSense = PAC_ONE_R_SENSE,
};
pac193xSensorConfiguration_t sensor2 = {
    .i2c_host = PAC_TWO_HOST,
    .i2c_slave_address = PAC_TWO_SLAVE,
    .powerPin = PAC_TWO_POWER_PIN,
    .usedChannels = PAC_TWO_USED_CHANNELS,
    .rSense = PAC_TWO_R_SENSE,
};
/* endregion Powersensor config */

/* region Flash/FPGA config */
spiConfiguration_t flashSpiConfig = {
    .spiInstance = SPI_FLASH_INSTANCE,
    .sckPin = SPI_FLASH_SCK,
    .misoPin = SPI_FLASH_MISO,
    .mosiPin = SPI_FLASH_MOSI,
    .baudrate = SPI_FLASH_BAUDRATE,
};

flashConfiguration_t flashConfig = {
    .flashSpiConfiguration = &flashSpiConfig,
    .flashBytesPerSector = FLASH_BYTES_PER_SECTOR,
    .flashBytesPerPage = FLASH_BYTES_PER_PAGE,
};

#ifndef URL_BASE
#error "URL_BASE not defined!"
#endif
char urlSlow[] = "http://" URL_BASE ":5000/getslow";
char urlFast[] = "http://" URL_BASE ":5000/getfast";

#ifdef S15
size_t slowBinfileLength = 85540;
size_t fastBinfileLength = 86116;
#elif S50
size_t slowBinfileLength = 231608;
size_t fastBinfileLength = 232360;
#else
#error "FPGA not specified!"
#endif
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
    env5HwControllerLedsAllOn();
    PRINT("Please press 'o' if the MCU leds are on!");
    int result;
    do {
        result = getchar_timeout_us(UINT32_MAX);
    } while (result == PICO_ERROR_TIMEOUT);
    env5HwControllerLedsAllOff();
    TEST_ASSERT_EQUAL_CHAR('o', (char)result);
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

static void test_EspModule(void) {
    espInit();
    PRINT_DEBUG("ESP initialized!");
    TEST_ASSERT_EQUAL_UINT8(NETWORK_NO_ERROR, networkTryToConnectToNetworkUntilSuccessful());

    espOK = true;
}

static void test_Accelerometer(void) {
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR, adxl345bInit(adxl345b));

    uint8_t serialNumber = 0;
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR, adxl345bReadSerialNumber(adxl345b, &serialNumber));
    TEST_ASSERT_EQUAL_UINT8(0xE5, serialNumber);

    int delta_x, delta_y, delta_z;
    TEST_ASSERT_EQUAL_UINT8(ADXL345B_NO_ERROR,
                            adxl345bPerformSelfTest(adxl345b, &delta_x, &delta_y, &delta_z));
    PRINT_DEBUG("  X: %iLSB, Y: %iLSB, Z: %iLSB", delta_x, delta_y, delta_z);
}

static void test_Amplifier(void) {
    microphoneInitialize(MICRO_GPIO);
    PRINT_DEBUG("Amplifier initialized");
    microphoneSetSamplingRate(MICRO_SAMPLING_RATE);
    PRINT_DEBUG("Sampling-rate set to %u", MICRO_SAMPLING_RATE);

    uint8_t samples[MICRO_SAMPLE_COUNT];
    microphoneCapture(samples, MICRO_SAMPLE_COUNT, MICRO_GPIO);
    PRINT_BYTE_ARRAY("Microphone samples:", samples, MICRO_SAMPLE_COUNT);
}

static void test_Flash(void) {
    /* region initialize */
    env5HwControllerFpgaPowersOff();
    spiInit(&flashSpiConfig);
    /* endregion initialize */

    /* region ID */
    uint8_t data[3];
    data_t idBuffer = {.data = data, .length = sizeof(data)};
    TEST_ASSERT_EQUAL_INT(sizeof(data), flashReadConfig(&flashConfig, FLASH_READ_ID, &idBuffer));
    //TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, data[0], "Manufacture ID mismatch");
    uint16_t deviceId = data[1];
    deviceId = deviceId << 8 | data[2];
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x0219, deviceId, "Device ID mismatch");
    /* endregion ID */

    /* region data */
    // Erase Sector
    TEST_ASSERT_EQUAL_UINT8(FLASH_NO_ERROR, flashEraseSector(&flashConfig, 0x00000000));

    // write test data
    uint8_t testWrite[FLASH_BYTES_PER_PAGE];
    for (size_t i = 0; i < sizeof(testWrite); i++) {
        testWrite[i] = 0xAB;
    }
    TEST_ASSERT_EQUAL_INT(sizeof(testWrite), flashWritePage(&flashConfig, FLASH_BYTES_PER_PAGE,
                                                            testWrite, sizeof(testWrite)));

    // read test data
    uint8_t testRead[FLASH_BYTES_PER_PAGE];
    data_t readBuffer = {.data = testRead, .length = sizeof(testWrite)};
    TEST_ASSERT_EQUAL_INT(sizeof(testWrite),
                          flashReadData(&flashConfig, FLASH_BYTES_PER_PAGE, &readBuffer));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testWrite, testRead, sizeof(testWrite));
    /* endregion data */

    flashOK = true;
}

static void test_BlinkFpga(char *url, size_t length) {
    env5HwControllerFpgaPowersOff();
    fpgaConfigurationHandlerDownloadConfigurationViaHttp(&flashConfig, url, length, 1);
    sleep_ms(500);
    env5HwControllerFpgaPowersOn();
    PRINT("Please press 'o' if the FPGA leds are on!");
    int result;
    do {
        result = getchar_timeout_us(UINT32_MAX);
    } while (result == PICO_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_CHAR('o', (char)result);
    env5HwControllerFpgaPowersOff();
}
static void test_Fpga(void) {
    test_BlinkFpga(urlSlow, slowBinfileLength);
    test_BlinkFpga(urlFast, fastBinfileLength);
}

int main(void) {
    env5HwControllerInit();
    initializeIo();

    i2cInit(&i2cConfig);

    UNITY_BEGIN();

#ifdef EXTENSION_BOARD
    RUN_TEST(test_Accelerometer);
    RUN_TEST(test_Amplifier);
    RUN_TEST(test_EspModule);
#endif

#ifdef MAIN_BOARD
    RUN_TEST(test_McuBlinkLed);
    RUN_TEST(test_PowerSensor1);
    RUN_TEST(test_PowerSensor2);
    RUN_TEST(test_Flash);
    if (flashOK && espOK) {
        RUN_TEST(test_Fpga);
    }
#endif

    UNITY_END();

    reset_usb_boot(0, 0);
}
