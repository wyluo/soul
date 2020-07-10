#ifndef _BME300_
#define _BME300_

#include "bme300_if.h"
#include "app/bluestack/types.h"

/* control debug generation */
#ifdef DEBUG_LIB
#include <stdio.h>
#define BME_DEBUG(x)  printf x
#else
#define BME_DEBUG(x)
#endif

typedef unsigned long long uint64;
typedef signed long long   int64;

struct __accelerometer_config
{
    char *fwFilename;
    uint32 fwFilelen;

    /*! The SPI clock frequency is kHz */
    uint16 spi_clock_khz;
    /*! The PIOs used to control/communicate with the sensor */
    struct
    {
        /*! PIO used to power-on the accelerometer or #ADXL362_ON_PIO_UNUSED */
        uint8 on;
        /*! Interrupt PIO driven by the accelerometer */
        uint8 interrupt;
        /*! SPI clock PIO */
        uint8 spi_clk;
        /*! SPI chip select (active low) PIO */
        uint8 spi_cs;
        /*! SPI master-out-slave-in PIO */
        uint8 spi_mosi;
        /*! SPI master-in-slave-out PIO */
        uint8 spi_miso;
    } pios;
};

#if 0
typedef struct _firmware_header {
    uint8 magic[2];
    uint8 flags[2];
    uint8 keyFlags[2];
    uint8 version[2];
    uint8 sha256[32];
    uint8 cert[48];
    uint8 payloadLen[4];
    uint8 imageCrc[4];
    uint8 keyOffset[4];
    uint8 expectedVersion[4];
    uint8 reserved2[15];
    uint8 headerCrc[4];
} fwHeader_t;
#endif

typedef struct _fifo_buffer {
    uint32 readPos;
    uint32 readLen;
    uint32 remainLen;
    uint8 buffer[BME_DATA_FIFO_BUFFER_LENGTH];
} fifoBuffer_t;

int8 readFifo(uint8 chan, uint8 *buf, uint32 bufLen, uint32 *retLen, uint32 *remainLen);
uint8 getIntStatus(void);
int8 readParam(uint16 param, uint8 *buf, uint16 bufLen, uint16 *retLen);
int8 writeParam(uint16 param, uint8 *buf, uint16 bufLen);
int8 waitStatusReady(void);
int8 readStatus(uint16 *code, uint8 *buf, uint16 bufLen, uint16 *retLen);
void startVirtSensor(uint16 id);
void stopVirtSensor(uint16 id);
int8 virtSensorFifoFlush(uint16 id);
int8 virtSensorConfig(uint16 id, uint32 sampleRate, uint32 latency);

bitserial_handle BME_begin(void);
void BME_end(void);
void BME_init(void);
void BME_softReset(void);
#if 0
void BME_hardReset(void);
#endif
bool BME_getIntPinStatus(void);
bool BME_downloadFirmware(uint16 index, uint32 filelen);
void BME_bootFromRam(void);
void BME_parseFifo(void);
void BME_calibrateGaitSensorForward(uint8 duration);
void BME_calibrateGaitSensorDown(uint8 duration);
void BME_getGaitSensorCalibrationValue(float *cal);
void BME_setGaitSensorCalibrationValue(float *cal);
void BME_startGaitSensor(uint8 cycleTime, float height);
void BME_stopGaitSensor(void);
#endif
