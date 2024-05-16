#define SOURCE_FILE "MAIN"

#include "Adxl345b.h"
#include "Adxl345bTypedefs.h"
#include "Common.h"
#include "Esp.h"
#include "Network.h"

#include <hardware/i2c.h>
#include <pico/stdlib.h>

static void initializeIo(void) {
    stdio_init_all();
    while ((!stdio_usb_connected())) {}
}

static void testWifiModule(void) {
    PRINT("=== TEST WIFI START ===");

    espInit();
    PRINT("  ESP initialized!");

    networkErrorCode_t error;

    error = networkTryToConnectToNetworkUntilSuccessful();
    if (error != NETWORK_NO_ERROR) {
        PRINT("  Failed to Connect to WIFI (0x%02X)", error);
        return;
    }
    PRINT("  \033[0;32mPASSED\033[0m");
    PRINT("=== TEST WIFI DONE ===");
}
static void getSerialNumber() {
    uint8_t serialNumber = 0;

    PRINT("Requesting serial number:");
    adxl345bErrorCode_t errorCode = adxl345bReadSerialNumber(&serialNumber);
    if (errorCode == ADXL345B_NO_ERROR) {
        PRINT("  Expected: 0xE5, Actual: 0x%02X", serialNumber);
        PRINT(serialNumber == 0xE5 ? "  \033[0;32mPASSED\033[0m" : "  \033[0;31mFAILED\033[0m;");
    } else {
        PRINT("  \033[0;31mFAILED\033[0m adxl345b_ERROR: %02X", errorCode);
    }
}
static void makeSelfTest() {
    PRINT("Start self test:");
    int delta_x, delta_y, delta_z;
    adxl345bErrorCode_t errorCode = adxl345bPerformSelfTest(&delta_x, &delta_y, &delta_z);
    PRINT("  X: %iLSB, Y: %iLSB, Z: %iLSB", delta_x, delta_y, delta_z);
    if (errorCode == ADXL345B_NO_ERROR) {
        PRINT("  \033[0;32mPASSED\033[0m");
    } else {
        PRINT("  \033[0;31mFAILED\033[0m; adxl345b_ERROR: %02X", errorCode);
    }
}
static void testAccelerometer(void) {
    PRINT("=== TEST ADXL345b START");
    adxl345bErrorCode_t error;

    error = adxl345bInit(i2c1, ADXL345B_I2C_ALTERNATE_ADDRESS);
    if (error != ADXL345B_NO_ERROR) {
        PRINT("  Initialize Sensor failed! (0x%02X)", error);
        return;
    }
    PRINT("  Sensor initialized successful!");

    getSerialNumber();
    makeSelfTest();

    PRINT("=== TEST ADXL345b DONE");
}
static void testAmplifier(void) {
    // TODO
}
_Noreturn static void testBoard(void) {
    while (1) {
        char input = getchar_timeout_us(UINT32_MAX);

        switch (input) {
        case 'w':
            testWifiModule();
            break;
        case 'a':
            testAccelerometer();
            break;
        case 'm':
            testAmplifier();
            break;
        default:
            PRINT("WRONG INPUT");
        }
    }
}

int main() {
    initializeIo();
    testBoard();
}
