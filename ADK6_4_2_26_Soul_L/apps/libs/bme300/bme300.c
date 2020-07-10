#include <panic.h>
#include <pmalloc.h>
#include <print.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system_clock.h>
#include <stream.h>
#include <source.h>
#include <pio.h>
#include <pio_common.h>
#include <hydra_macros.h>
#include <bitserial_api.h>


#include "bme300.h"

#define ASSERT_BME      // comment this to turn off assertion
#ifdef ASSERT_BME
#define BME_ASSERT(test)    PanicFalse(test)
#else
#define BME_ASSERT(test)
#endif

#define PIO2BANK(pio) ((uint16)((pio) / 32))
#define PIO2MASK(pio) (1UL << ((pio) % 32))


#define REG_WRITE_MASK      0x7F
#define REG_READ_MASK       0x80
#define REG_WRITE(r)        (r & REG_WRITE_MASK)
#define REG_READ(r)         (r | REG_READ_MASK)

#define PARAM_WRITE_MASK    0xFFF
#define PARAM_READ_MASK     0x1000
#define PARAM_WRITE(p)      (p & PARAM_WRITE_MASK)
#define PARAM_READ(p)       (p | PARAM_READ_MASK)

fifoBuffer_t wakeupFifo;
fifoBuffer_t nonWakeupFifo;
fifoBuffer_t statusFifo;

uint8 gaitSensorMode;
virtSensorOutputGait_t gaitParameters;
bool isGaitParametersUpdated;
bitserial_handle bme_handle;

/* fifo parser callback functions */
static int8 frmGait(fifoBuffer_t *fifoBuffer);
static int8 frmPadding(fifoBuffer_t *fifoBuffer);

static void delay_ms(uint16 v_msec_16)
{
    /** add one to the delay to make sure we don't return early */
    uint32 v_delay = SystemClockGetTimerTime() + (v_msec_16 * 1000) + 1;

    while (((int32)(SystemClockGetTimerTime() - v_delay)) < 0);
}

static bool WriteRegister(bitserial_handle handle, uint8 reg, uint8 value)
{
    bitserial_result result;
    uint8 command[2] = { REG_WRITE(reg), value };

    result = BitserialWrite(handle, BITSERIAL_NO_MSG, command, 2, BITSERIAL_FLAG_BLOCK);

    return (result == BITSERIAL_RESULT_SUCCESS);
}

static bool writeRegMultbytes(bitserial_handle handle, uint8 reg, uint8 *data, uint32 len)
{
    bitserial_result result;
    uint8 *command = PanicUnlessMalloc(len + 1);

    command[0] = REG_WRITE(reg);
    memcpy(&command[1], data, len);

    result = BitserialWrite(handle, BITSERIAL_NO_MSG, command, len + 1, BITSERIAL_FLAG_BLOCK);

    free(command);

    return (result == BITSERIAL_RESULT_SUCCESS);
}

static bool ReadRegister(bitserial_handle handle, uint8 reg, uint8 *value)
{
    bitserial_result result;
    uint8 command[2] = { REG_READ(reg), 0x0 };

    result = BitserialTransfer(handle, BITSERIAL_NO_MSG, &command[0], 1, &command[1], 1);
    *value = command[1];

    return (result == BITSERIAL_RESULT_SUCCESS);
}

static bool readRegMultbytes(bitserial_handle handle, uint8 reg, uint8 *data, uint32 len)
{
    bitserial_result result;
    uint8 *command = PanicUnlessMalloc(len + 1);

    command[0] = REG_READ(reg);

    result = BitserialTransfer(handle, BITSERIAL_NO_MSG, &command[0], 1, &command[1], len);
    memcpy(data, &command[1], len);

    free(command);

    return (result == BITSERIAL_RESULT_SUCCESS);
}


static bitserial_handle bme300Enable(void)
{
    bitserial_config bsconfig;
    uint16 bank;
    uint32 mask;

    BME_DEBUG("bme300Enable");

    /* Enable PIO to power the accelerometer as an output */
    bank = PIO2BANK(BME_EN);
    mask = PIO2MASK(BME_EN);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, 0));
    PioCommonSetPio(BME_EN,1,FALSE);
    delay_ms(150);

    /* Setup Interrupt as input with weak pull up */
    bank = PIO2BANK(BME_SPI_INT);
    mask = PIO2MASK(BME_SPI_INT);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, 0));  //PanicNotZero(PioSet32Bank(bank, mask, mask));  

    bank = PIO2BANK(BME_RST);
    mask = PIO2MASK(BME_RST);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

    /* Setup the PIOs for Bitserial SPI use */
    bank = PIO2BANK(BME_SPI_CS);
    mask = PIO2MASK(BME_SPI_CS);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, 0));
    delay_ms(10);
    PanicNotZero(PioSet32Bank(bank, mask, mask));


    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(BME_SPI_CLK);
    mask = PIO2MASK(BME_SPI_CLK);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(BME_SPI_MISO);
    mask = PIO2MASK(BME_SPI_MISO);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(BME_SPI_MOSI);
    mask = PIO2MASK(BME_SPI_MOSI);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));


    /* Setup the PIOs for Bitserial SPI use*/
    PanicFalse(PioSetFunction(BME_SPI_CS, BITSERIAL_0_SEL_OUT));
    PanicFalse(PioSetFunction(BME_SPI_CLK, BITSERIAL_0_CLOCK_OUT));
    PanicFalse(PioSetFunction(BME_SPI_MISO, BITSERIAL_0_DATA_IN));
    PanicFalse(PioSetFunction(BME_SPI_MOSI, BITSERIAL_0_DATA_OUT));

    /* Configure Bitserial to work with bme300 */
    memset(&bsconfig, 0, sizeof(bsconfig));
    bsconfig.mode = BITSERIAL_MODE_SPI_MASTER;
    bsconfig.clock_frequency_khz = BME_CLK_KHZ;  // 8MHz
    bsconfig.u.spi_cfg.sel_enabled = TRUE;
    bsconfig.u.spi_cfg.clock_sample_offset = 0;
    bsconfig.u.spi_cfg.interword_spacing = 0;
    bsconfig.u.spi_cfg.select_time_offset = 0;
    bsconfig.u.spi_cfg.flags = BITSERIAL_SPI_MODE_0;
    return BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_0, &bsconfig);
}

static void bme300Disable(bitserial_handle handle)
{
    uint16 bank;
    uint32 mask;
    BME_DEBUG("bme300Disable");

    /* Disable interrupt and set weak pull down */
    bank = PIO2BANK(BME_SPI_INT);
    mask = PIO2MASK(BME_SPI_INT);
    PanicNotZero(PioSet32Bank(bank, mask, 0));

    /* Release bitserial instance */
    BitserialClose(handle);
    handle = BITSERIAL_HANDLE_ERROR;

    /* Power off the BME300*/
    bank = PIO2BANK(BME_EN);
    mask = PIO2MASK(BME_EN);
    PanicNotZero(PioSet32Bank(bank, mask, 1));
}

bitserial_handle BME_begin(void)
{
    bme_handle = bme300Enable();

    return bme_handle;
}

void BME_end(void)
{
        bme300Disable(bme_handle);
}

void BME_init(void)
{
    uint8 ret;
    uint8 buf[2];

    // disable cpu turbo mode
    PanicFalse(WriteRegister(bme_handle, BME_REG_CHIP_CTRL, BME_CHIPCTL_CPU_TURBO_DISABLE));
    PanicFalse(ReadRegister(bme_handle,BME_REG_CHIP_CTRL,&ret));
    BME_ASSERT(ret == BME_CHIPCTL_CPU_TURBO_DISABLE);

    // disable status interrupt on synchronous status channel
    // status and debug information will be available on asynchronous status channel only
    PanicFalse(WriteRegister(bme_handle, BME_REG_HOST_INTERRUPT_CTRL, BME_HICTL_DISABLE_STATUS));
    PanicFalse(ReadRegister(bme_handle,BME_REG_HOST_INTERRUPT_CTRL,&ret));
    BME_ASSERT(ret == BME_HICTL_DISABLE_STATUS);
    PanicFalse(WriteRegister(bme_handle, BME_REG_HOST_INTERFACE_CTRL, BME_HIFCTL_ASYNC_STATUS_CHANNEL));
    PanicFalse(ReadRegister(bme_handle,BME_REG_HOST_INTERFACE_CTRL,&ret));
    BME_ASSERT(ret == BME_HIFCTL_ASYNC_STATUS_CHANNEL);

    // make fifo clean
    memset(&wakeupFifo, 0, sizeof(wakeupFifo));
    memset(&nonWakeupFifo, 0, sizeof(nonWakeupFifo));
    memset(&statusFifo, 0, sizeof(statusFifo));

    gaitSensorMode = BME_PARAM_GAIT_SENSOR_MODE_GAIT;
    isGaitParametersUpdated = FALSE;

    // verify we're using correct chip
    PanicFalse(ReadRegister(bme_handle,BME_REG_CHIP_ID,&ret));
    BME_ASSERT(ret == BME_CHIP_ID);
    BME_DEBUG(("Product ID : 0x%x\n",ret));
    PanicFalse(ReadRegister(bme_handle,BME_REG_REVISION_ID,&ret));
    BME_DEBUG(("Product rev : 0x%x\n",ret));
    PanicFalse(readRegMultbytes(bme_handle,BME_REG_RAM_VERSION, buf,2));
    BME_DEBUG(("RAM ver :  %d 0x%x 0x%x\n", BME_LE2U16(buf),buf[0],buf[1]));
    PanicFalse(readRegMultbytes(bme_handle,BME_REG_ROM_VERSION, buf, 2));
    BME_DEBUG(("ROM ver :  %d 0x%x 0x%x\n", BME_LE2U16(buf),buf[0],buf[1]));
} 

void BME_softReset(void) 
{
    uint8 ret;

    // trigger soft reset
    PanicFalse(WriteRegister(bme_handle, BME_REG_RESET_REQUEST, BME_RSTREQ_RESET));
    delay_ms(100);
    // check host interface is ready after reset
    PanicFalse(ReadRegister(bme_handle,BME_REG_BOOT_STATUS,&ret));
	BME_DEBUG(("===========ret========= : 0x%x\n",ret));
    BME_ASSERT(ret & BME_BST_HOST_INTERFACE_READY);
}

#if 0
void BME_hardReset(void)
{
    uint8 ret;
    uint16 bank;
    uint32 mask;

    // trigger hard reset using reset pin
    /* Disable interrupt and set weak pull down */
    bank = PIO2BANK(BME_RST);
    mask = PIO2MASK(BME_RST);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, 0));
    delay_ms(1);
    PanicNotZero(PioSet32Bank(bank, mask, mask));
    delay_ms(100);
    // check host interface is ready after reset
    PanicFalse(ReadRegister(bme_handle,BME_REG_BOOT_STATUS,&ret));
    BME_ASSERT(ret & BME_BST_HOST_INTERFACE_READY);
}
#endif

bool BME_getIntPinStatus(void)
{
    if (PioCommonGetPio(BME_SPI_INT) == TRUE) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

static int8 executeFWDownload(Source fwFileSource, uint16 cmd, uint32 len)
{
    //uint8 *cmdBuf = PanicUnlessMalloc(BME_COMMAND_PACKET_LEN);
    uint8 cmdBuf[BME_COMMAND_PACKET_LEN];
    uint32 aLen, pos, transLen, copyLen;
    uint32 remain;

    if (len % 4) {      // align 4 bytes
        aLen = 4 * (len / 4 + 1);
    }
    else {
        aLen = len;
    }

    cmdBuf[0] = cmd & 0xFF;
    cmdBuf[1] = (cmd >> 8) & 0xFF;
    cmdBuf[2] = (aLen / 4) & 0xFF;
    cmdBuf[3] = ((aLen / 4) >> 8) & 0xFF;
  
    pos = 4;
    remain = len;
 
    while (remain + pos > 0) {
        if (remain + pos > BME_COMMAND_PACKET_LEN) {
            transLen = BME_COMMAND_PACKET_LEN;
            copyLen = BME_COMMAND_PACKET_LEN - pos;
        }
        else {
            transLen = remain + pos;
            copyLen = remain;
            if ((transLen & 0x3) != 0) {    // not multiple of 4
                transLen = ((transLen >> 2) + 1) << 2;
            }
        }

        if (copyLen > 0) {
        
            const uint8 *buf = SourceMap(fwFileSource);
            memmove(cmdBuf + pos, buf/* + len - remain*/, copyLen);
            //BME_DEBUG("Firmware image copyLen %ld buf[0]=0x%x\n", copyLen, buf[0]);
            SourceDrop(fwFileSource,copyLen);
            //BME_DEBUG("Firmware image remain %ld buf[0]=0x%x\n", remain, cmdBuf[pos]);
        }

        if (transLen - pos - copyLen > 0) {
            memset(cmdBuf + pos + copyLen, 0, transLen - pos - copyLen);
        }


        // start SPI transaction
        writeRegMultbytes(bme_handle, BME_REG_CHAN_CMD, cmdBuf, transLen);

        pos = 0;
        remain -= copyLen;
    }

    //free(cmdBuf);
    return BME_HIF_E_SUCCESS;
}

static int8 executeCommand(uint16 cmd, const uint8 *buf, uint16 len)
{
    if (cmd == 0x0 || (len > 0 && !buf))
    return BME_HIF_E_INVALID;

    uint8 *cmdBuf = PanicUnlessMalloc(MIN(BME_COMMAND_HEADER_LEN+len, BME_COMMAND_PACKET_LEN));
    uint16 aLen, pos, remain, transLen, copyLen;

    if (len % 4) {      // align 4 bytes
        aLen = 4 * (len / 4 + 1);
    }
    else {
        aLen = len;
    }

    cmdBuf[0] = cmd & 0xFF;
    cmdBuf[1] = (cmd >> 8) & 0xFF;
    cmdBuf[2] = aLen & 0xFF;
    cmdBuf[3] = (aLen >> 8) & 0xFF;

    pos = 4;
    remain = len;
 

    while (remain + pos > 0) {
        if (remain + pos > BME_COMMAND_PACKET_LEN) {
            transLen = BME_COMMAND_PACKET_LEN;
            copyLen = BME_COMMAND_PACKET_LEN - pos;
        }
        else {
            transLen = remain + pos;
            copyLen = remain;
            if ((transLen & 0x3) != 0) {    // not multiple of 4
                transLen = ((transLen >> 2) + 1) << 2;
            }
        }

        if (copyLen > 0) {
            memcpy(cmdBuf + pos, buf + len - remain, copyLen);

        }

        if (transLen - pos - copyLen > 0) {
            memset(cmdBuf + pos + copyLen, 0, transLen - pos - copyLen);
        }


        // start SPI transaction
        writeRegMultbytes(bme_handle, BME_REG_CHAN_CMD, cmdBuf,transLen);

        pos = 0;
        remain -= copyLen;
    }

    free(cmdBuf);
    return BME_HIF_E_SUCCESS;
} 

static int8 checkBootStatusRam(void)
{
    uint8 ret;
    uint16 i;

    for (i=0; i<BME_BST_CHECK_RETRY; i++) {
        delay_ms(50);
        PanicFalse(ReadRegister(bme_handle,BME_REG_BOOT_STATUS, &ret));

        if ((ret & BME_BST_HOST_INTERFACE_READY) &&
            (ret & BME_BST_HOST_FW_VERIFY_DONE) &&
            (!(ret & BME_BST_HOST_FW_VERIFY_ERROR))) {
            break;
        }
    }

    if (i == BME_BST_CHECK_RETRY) {
        return BME_HIF_E_TIMEOUT;
    }

    return BME_HIF_E_SUCCESS;
}


bool BME_downloadFirmware(uint16 index, uint32 filelen)
{
    uint8 ret;
    int8 res;
    uint16 magic;
    uint32 len;
    Source fwFileSource;
    const uint8 *data;

    PanicFalse(ReadRegister(bme_handle,BME_REG_BOOT_STATUS,&ret));
 
    fwFileSource = StreamFileSource(index);

    PanicFalse(fwFileSource);
    len = filelen;  
    data = SourceMap(fwFileSource);

    magic = (uint16_t)(data[0]|data[1]<< 8);


    BME_DEBUG(("Firmware image magic 0x%x\n", magic));
    if (magic != BME_FW_MAGIC) {
        BME_DEBUG(("Firmware image is incorrect\n)"));
        return FALSE;
    }

    res = executeFWDownload(fwFileSource, BME_CMD_UPLOAD_TO_PROGRAM_RAM, len);
    if (res != BME_HIF_E_SUCCESS) {
        BME_DEBUG(("Error occured during firmware download\n"));
        SourceClose(fwFileSource);
        return FALSE;
    }

    res = checkBootStatusRam();
    if (res != BME_HIF_E_SUCCESS) {
        BME_DEBUG(("Firmware download failed\n"));
        SourceClose(fwFileSource);
        return FALSE;
    }
    SourceClose(fwFileSource);
    BME_DEBUG(("Firmware download done\n"));

    return TRUE;
} 

void BME_bootFromRam(void)
{
    uint8 ret, err;
    int8 res;
    uint8 buf[2];     // ram version register has 2 bytes data
    uint16 ramVer;

    PanicFalse(ReadRegister(bme_handle,BME_REG_BOOT_STATUS,&ret));


    if ((ret & BME_BST_HOST_INTERFACE_READY) == BME_BST_HOST_INTERFACE_READY) {
        if ((ret & BME_BST_HOST_FW_VERIFY_DONE) == BME_BST_HOST_FW_VERIFY_DONE) {

            PanicFalse(readRegMultbytes(bme_handle,BME_REG_RAM_VERSION, buf,2));
            ramVer = BME_LE2U16(buf);
            if (ramVer == 0) {
                res = executeCommand(BME_CMD_BOOT_PROGRAM_RAM, NULL, 0);
                if (res != BME_HIF_E_SUCCESS) {
                                        ReadRegister(bme_handle,BME_REG_ERROR,&err);
                    BME_DEBUG(("Error : %d\n",err));
                    return;
                }

                res = checkBootStatusRam();
                if (res != BME_HIF_E_SUCCESS) {
                                        ReadRegister(bme_handle,BME_REG_ERROR,&err);
                    BME_DEBUG(("Error : %d\n",err));
                    return;
                }
            }

            PanicFalse(readRegMultbytes(bme_handle,BME_REG_RAM_VERSION, buf,2));
            ramVer = BME_LE2U16(buf);
            if (ramVer) {
                BME_DEBUG(("RAM ver : %d\n", ramVer));
            }
            else {
                BME_DEBUG(("Booting RAM failed\n"));
                return;
            }
        }
        else {
            BME_DEBUG(("Download firmware to RAM before boot\n"));
            return;
        }
    }
    else {
        BME_DEBUG(("Host interface is not ready\n"));
        return;
    }

    BME_DEBUG(("Boot from RAM done\n"));
}

uint8 getIntStatus(void)
{
    uint8 ret;
    PanicFalse(ReadRegister(bme_handle,BME_REG_INT_STATUS, &ret));
    BME_DEBUG(("int status: 0x%x\n",ret));
    return ret;
}

void BME_parseFifo(void)
{
    uint8 intStatus;
    uint32 readLen;
    uint16 sensorId;

    PanicFalse(ReadRegister(bme_handle,BME_REG_INT_STATUS, &intStatus));

    while (intStatus || wakeupFifo.remainLen || nonWakeupFifo.remainLen || statusFifo.remainLen) {
        // read wakeup fifo
        if ((BME_IS_IRQ_FIFO_W(intStatus) == BME_IST_FIFO_W_DRDY)
            || (BME_IS_IRQ_FIFO_W(intStatus) == BME_IST_FIFO_W_LTCY)
            || (BME_IS_IRQ_FIFO_W(intStatus) == BME_IST_FIFO_W_WM)
            || (wakeupFifo.remainLen)) {
            wakeupFifo.readPos = 0;
            readFifo(BME_REG_CHAN_FIFO_W,
                     &wakeupFifo.buffer[wakeupFifo.readLen],
                     (BME_DATA_FIFO_BUFFER_LENGTH - wakeupFifo.readLen),
                     &readLen,
                     &wakeupFifo.remainLen);
            wakeupFifo.readLen += readLen;
            BME_DEBUG(("W_FIFO_INT\n"));
        }

        // read non wakeup fifo
        if ((BME_IS_IRQ_FIFO_NW(intStatus) == BME_IST_FIFO_NW_DRDY)
            || (BME_IS_IRQ_FIFO_NW(intStatus) == BME_IST_FIFO_NW_LTCY)
            || (BME_IS_IRQ_FIFO_NW(intStatus) == BME_IST_FIFO_NW_WM)
            || (nonWakeupFifo.remainLen)) {
            nonWakeupFifo.readPos = 0;
            readFifo(BME_REG_CHAN_FIFO_NW,
                     &nonWakeupFifo.buffer[nonWakeupFifo.readLen],
                     (BME_DATA_FIFO_BUFFER_LENGTH - nonWakeupFifo.readLen),
                     &readLen,
                     &nonWakeupFifo.remainLen);
            nonWakeupFifo.readLen += readLen;
            BME_DEBUG(("NW_FIFO_INT\n"));
        }

        // read status fifo
        if ((BME_IS_IRQ_ASYNC_STATUS(intStatus) == BME_IST_MASK_DEBUG) || statusFifo.remainLen) {
            statusFifo.readPos = 0;
            readFifo(BME_REG_CHAN_STATUS,
                     &statusFifo.buffer[statusFifo.readLen],
                     (BME_DATA_FIFO_BUFFER_LENGTH - statusFifo.readLen),
                     &readLen,
                     &statusFifo.remainLen);
            statusFifo.readLen += readLen;
            BME_DEBUG(("BME_IST_MASK_DEBUG\n"));
        }

        if (BME_IS_IRQ_STATUS(intStatus) == BME_IST_MASK_STATUS) {
            // will not parse result of command here
            BME_DEBUG(("BME_IST_MASK_STATUS\n"));
        }

        if (BME_IS_IRQ_RESET(intStatus) == BME_IST_MASK_RESET_FAULT) {
            // will not get post mortem data now
            BME_DEBUG(("BME_IST_MASK_RESET_FAULT\n"));
        }

        // clear interrupt status
        intStatus = 0;

        //BME_DEBUG(("wk : " + String(wakeupFifo.readPos) + ", " + String(wakeupFifo.readLen)));
        //BME_DEBUG(("nonwk : " + String(nonWakeupFifo.readPos) + ", " + String(nonWakeupFifo.readLen)));
        //BME_DEBUG(("stat : " + String(statusFifo.readPos) + ", " + String(statusFifo.readLen)));

        // parse wakeup fifo
        while (wakeupFifo.readPos < wakeupFifo.readLen) {
            sensorId = wakeupFifo.buffer[wakeupFifo.readPos];
            //BME_DEBUG(("wkup frame id: " + String(sensorId) + ", read pos: " + String(wakeupFifo.readPos) + ", read len: " + String(wakeupFifo.readLen));

            if (sensorId < BME_SYS_SENSOR_ID_MAX) {
                if (sensorId == BME_SENSOR_ID_GAIT) {
                    if (frmGait(&wakeupFifo) != BME_HIF_E_SUCCESS) {
                        BME_DEBUG(("wkup frmGait failed\n"));
                        break;
                    }
                }
                else {
                    if (frmPadding(&wakeupFifo) != BME_HIF_E_SUCCESS) {
                        BME_DEBUG(("wkup frmPadding failed\n"));
                        break;
                    }
                }
            }
            else {
                BME_DEBUG(("no sensor matched in wakeup FIFO\n"));
            }
        }
        if (wakeupFifo.readLen) {
            wakeupFifo.readLen -= wakeupFifo.readPos;
            if (wakeupFifo.readLen) {
                memmove(&wakeupFifo.buffer[0], &wakeupFifo.buffer[wakeupFifo.readPos], wakeupFifo.readLen);
            }
        }

        // parse non wakeup fifo
        while (nonWakeupFifo.readPos < nonWakeupFifo.readLen) {
            sensorId = nonWakeupFifo.buffer[nonWakeupFifo.readPos];
            //BME_DEBUG(("non wkup frame id: " + String(sensorId) + ", read pos: " + String(nonWakeupFifo.readPos) + ", read len: " + String(nonWakeupFifo.readLen));

            if (sensorId < BME_SYS_SENSOR_ID_MAX) {
                if (frmPadding(&nonWakeupFifo) != BME_HIF_E_SUCCESS) {
                    BME_DEBUG(("non wkup frmPadding failed\n"));
                    break;
                }
            }
            else {
                BME_DEBUG(("no sensor matched in non wakeup FIFO\n"));
            }
        }
        if (nonWakeupFifo.readLen) {
            nonWakeupFifo.readLen -= nonWakeupFifo.readPos;
            if (nonWakeupFifo.readLen) {
                memmove(&nonWakeupFifo.buffer[0], &nonWakeupFifo.buffer[nonWakeupFifo.readPos], nonWakeupFifo.readLen);
            }
        }

        // parse status fifo
        while (statusFifo.readPos < statusFifo.readLen) {
            sensorId = statusFifo.buffer[statusFifo.readPos];
            //BME_DEBUG(("status frame id: " + String(sensorId) + ", read pos: " + String(statusFifo.readPos) + ", read len: " + String(statusFifo.readLen));

            if (sensorId < BME_SYS_SENSOR_ID_MAX) {
                if (frmPadding(&statusFifo) != BME_HIF_E_SUCCESS) {
                    BME_DEBUG(("status frmPadding failed\n"));
                    break;
                }
            }
            else {
                BME_DEBUG(("no sensor matched in status FIFO\n"));
            }
        }
        if (statusFifo.readLen) {
            statusFifo.readLen -= statusFifo.readPos;
            if (statusFifo.readLen) {
                memmove(&statusFifo.buffer[0], &statusFifo.buffer[statusFifo.readPos], statusFifo.readLen);
            }
        }
    }
}

int8 readFifo(uint8 chan, uint8 *buf, uint32 bufLen, uint32 *retLen, uint32 *remainLen)
{
    uint32 readLen;

    if (!buf || !retLen || !remainLen || (bufLen == 0)) {
        return BME_HIF_E_INVALID;
    }

    if (*remainLen == 0) {
        uint8 tmpBuf[2];
        PanicFalse(readRegMultbytes(bme_handle,chan, tmpBuf,2));
        *remainLen = BME_LE2U16(tmpBuf);
        if (*remainLen == 0) {
            *retLen = 0;
            return BME_HIF_E_SUCCESS;
        }
    }

    if (bufLen < *remainLen) {
        *retLen = bufLen;
    }
    else {
        *retLen = *remainLen;
    }

    readLen = *retLen;

    while (readLen > BME_DEFAULT_MAX_READ_BURST) {
                PanicFalse(readRegMultbytes(bme_handle,chan, buf,BME_DEFAULT_MAX_READ_BURST));
        readLen -= BME_DEFAULT_MAX_READ_BURST;
        buf += BME_DEFAULT_MAX_READ_BURST;
    }

        PanicFalse(readRegMultbytes(bme_handle,chan, buf, readLen));
    *remainLen -= readLen;

    return BME_HIF_E_SUCCESS;
}

/*
void getPresentVirtSensors(void)
{
    uint16 retLen = 0;
    int8 res;

    res = readParam(BME_PARAM_SYS_VIRT_SENSOR_PRESENT, virtSensorPresent, sizeof(virtSensorPresent), &retLen);
    if (res != BME_HIF_E_SUCCESS) {
        BME_DEBUG(("read param error : %d\n",res));
        return;
    }

    for (uint16 i=0; i<sizeof(virtSensorPresent); i++) {
        for (uint8 j=0; j<8; j++) {
            virtSensor[i * 8 + j].info.present = (virtSensorPresent[i] >> j) & 0x01;
        }
    }


    BME_DEBUG(("present sensor: \n"));
    for (uint16 i=0; i<BME_SYS_SENSOR_ID_MAX; i++) {
        if (virtSensor[i].info.present) {
            BME_DEBUG(("ID : %d\n" ,i));
        }
    }
}
*/

int8 readParam(uint16 param, uint8 *buf, uint16 bufLen, uint16 *retLen)
{
    uint16 code;
    uint8 oldStatus;
    uint8 temp;
    int8 res;

    *retLen = 0;
    PanicFalse(ReadRegister(bme_handle,BME_REG_HOST_INTERFACE_CTRL, &temp));
    oldStatus = temp;
    temp &= ~BME_HIFCTL_ASYNC_STATUS_CHANNEL;
    WriteRegister(bme_handle, BME_REG_HOST_INTERFACE_CTRL, temp);

    res = executeCommand(PARAM_READ(param), buf, 0);
    if (res != BME_HIF_E_SUCCESS) {
        return res;
    }

    res = waitStatusReady();
    if (res != BME_HIF_E_SUCCESS) {
        return res;
    }

    res = readStatus(&code, buf, bufLen, retLen);
    if (res != BME_HIF_E_SUCCESS) {
        return res;
    }

    temp = oldStatus;
    WriteRegister(bme_handle, BME_REG_HOST_INTERFACE_CTRL, temp);

    if (code != param) {
        return BME_HIF_E_TIMEOUT;
    }

    return BME_HIF_E_SUCCESS;
}

int8 writeParam(uint16 param, uint8 *buf, uint16 bufLen)
{
    return executeCommand(PARAM_WRITE(param), buf, bufLen);
}

int8 waitStatusReady(void)
{
    uint8 intStatus;
    int8 res = BME_HIF_E_TIMEOUT;

    for (uint16 retry=0; retry<BME_QUERY_PARAM_STATUS_READY_MAX_RETRY; retry++) {
        intStatus = getIntStatus();
        if (intStatus & BME_IST_MASK_STATUS) {
            res = BME_HIF_E_SUCCESS;
            break;
        }

        delay_ms(10);  // 10ms
    }

    return res;
}

int8 readStatus(uint16 *code, uint8 *buf, uint16 bufLen, uint16 *retLen)
{
    uint8 temp[4];

    PanicFalse(readRegMultbytes(bme_handle,BME_REG_CHAN_STATUS, (uint8*)&temp,sizeof(temp)));
    *code = BME_LE2U16(&temp[0]);
    *retLen = BME_LE2U16(&temp[2]);
    if (*retLen == 0) {
        return BME_HIF_E_SUCCESS;
    }
    if (bufLen < *retLen) {
        return BME_HIF_E_BUF;
    }
    PanicFalse(readRegMultbytes(bme_handle,BME_REG_CHAN_STATUS, buf, *retLen));

    return BME_HIF_E_SUCCESS;
}

#if 1
int8 frmGait(fifoBuffer_t *fifoBuffer)
{
    uint32 pos = fifoBuffer->readPos;
    uint16 idx = 1;

    if ((fifoBuffer->readPos + 68 + 1) > fifoBuffer->readLen) {
        return BME_HIF_E_BUF;
    }

    if (isGaitParametersUpdated){
        BME_DEBUG(("gait parameters not consumed!\n"));
    }

    memcpy(&gaitParameters.header, &fifoBuffer->buffer[pos + idx], 2);          idx += 2;
    memcpy(&gaitParameters.state, &fifoBuffer->buffer[pos + idx], 1);           idx += 1;
    memcpy(&gaitParameters.elapsed, &fifoBuffer->buffer[pos + idx], 4);         idx += 4;
    memcpy(&gaitParameters.stepCount, &fifoBuffer->buffer[pos + idx], 1);       idx += 1;
    memcpy(&gaitParameters.cadence, &fifoBuffer->buffer[pos + idx], 4);         idx += 4;
    memcpy(&gaitParameters.stepWidth, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.headAngle, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wssl_rgtl, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wssr_rgtr, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wdsl_rftl, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wdsr_rgtr, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wfpl_rmll, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.wfpr_rmlr, &fifoBuffer->buffer[pos + idx], 4);       idx += 4;
    memcpy(&gaitParameters.shock, &fifoBuffer->buffer[pos + idx], 4);           idx += 4;
    memcpy(&gaitParameters.symmetry, &fifoBuffer->buffer[pos + idx], 4);        idx += 4;
    memcpy(&gaitParameters.vo, &fifoBuffer->buffer[pos + idx], 4);              idx += 4;
    memcpy(&gaitParameters.legStiffness, &fifoBuffer->buffer[pos + idx], 4);    idx += 4;
    memcpy(&gaitParameters.consistency, &fifoBuffer->buffer[pos + idx], 4);     idx += 4;
    memcpy(&gaitParameters.speed, &fifoBuffer->buffer[pos + idx], 4);
    
    fifoBuffer->readPos += 68 + 1;
    isGaitParametersUpdated = TRUE;

    BME_DEBUG(("gait parameters\n"));

    return BME_HIF_E_SUCCESS;
}

#else
int8 frmGait(fifoBuffer_t *fifoBuffer)
{
    uint32 pos = fifoBuffer->readPos;

    if ((fifoBuffer->readPos + sizeof(gaitParameters) + 1) > fifoBuffer->readLen) {
        return BME_HIF_E_BUF;
    }

    if (isGaitParametersUpdated){
        BME_DEBUG(("gait parameters not consumed!\n"));
    }

    memcpy(&gaitParameters, &fifoBuffer->buffer[pos + 1], sizeof(gaitParameters));
    fifoBuffer->readPos += sizeof(gaitParameters) + 1;
    isGaitParametersUpdated = TRUE;

    BME_DEBUG(("gait parameters\n"));

    return BME_HIF_E_SUCCESS;
}
#endif

int8 frmPadding(fifoBuffer_t *fifoBuffer)
{
    //(void *)instance;
    int8 res = BME_HIF_E_SUCCESS;
    uint8 type = fifoBuffer->buffer[fifoBuffer->readPos];

    switch (type) {
        case BSX_VIRTUAL_SENSOR_ID_INVALID:
        case BME_SENSOR_ID_SIG_WU:
        case BME_SENSOR_ID_SIG_HW_WU:
        case BME_SENSOR_ID_STD:
        case BME_SENSOR_ID_STD_WU:
        case BME_SENSOR_ID_STD_HW:
        case BME_SENSOR_ID_STD_HW_WU:
        case BME_SENSOR_ID_TILT_WU:
        case BME_SENSOR_ID_WAKE_WU:
        case BME_SENSOR_ID_GLANCE_WU:
        case BME_SENSOR_ID_PICKUP_WU:
        case BME_SENSOR_ID_ANY_MOTION:
        case BME_SENSOR_ID_ANY_MOTION_WU:
        case BME_SYS_SENSOR_ID_FILLER:
            if((fifoBuffer->readPos + 1) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 1;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_PROX:
        case BME_SENSOR_ID_PROX_WU:
        case BME_SENSOR_ID_HUM:
        case BME_SENSOR_ID_HUM_WU:
        case BME_SYS_SENSOR_ID_TS_SMALL_DELTA:
        case BME_SYS_SENSOR_ID_TS_SMALL_DELTA_WU:
        case BME_SENSOR_ID_EXCAMERA:
            if((fifoBuffer->readPos + 2) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 2;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_LIGHT:
        case BME_SENSOR_ID_LIGHT_WU:
        case BME_SENSOR_ID_TEMP:
        case BME_SENSOR_ID_TEMP_WU:
        //case BME_SENSOR_ID_ATEMP:
        //case BME_SENSOR_ID_ATEMP_WU:
        case BME_SENSOR_ID_AR_WU:
        case BME_SYS_SENSOR_ID_TS_LARGE_DELTA:
        case BME_SYS_SENSOR_ID_TS_LARGE_DELTA_WU:
            if((fifoBuffer->readPos + 3) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 3;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_BARO:
        case BME_SENSOR_ID_BARO_WU:
        case BME_SYS_SENSOR_ID_META_EVENT:
        case BME_SYS_SENSOR_ID_META_EVENT_WU:
            if((fifoBuffer->readPos + 4) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 4;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_GAS:
        case BME_SENSOR_ID_GAS_WU:
            if((fifoBuffer->readPos + 5) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 5;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SYS_SENSOR_ID_TS_FULL:
        case BME_SYS_SENSOR_ID_TS_FULL_WU:
            if((fifoBuffer->readPos + 6) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 6;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_ACC:
        case BME_SENSOR_ID_ACC_WU:
        case BME_SENSOR_ID_ACC_PASS:
        case BME_SENSOR_ID_ACC_RAW:
        case BME_SENSOR_ID_ACC_RAW_WU:
        case BME_SENSOR_ID_ACC_BIAS:
        case BME_SENSOR_ID_MAG:
        case BME_SENSOR_ID_MAG_WU:
        case BME_SENSOR_ID_MAG_PASS:
        case BME_SENSOR_ID_MAG_RAW:
        case BME_SENSOR_ID_MAG_RAW_WU:
        case BME_SENSOR_ID_MAG_BIAS:
        case BME_SENSOR_ID_ORI:
        case BME_SENSOR_ID_ORI_WU:
        case BME_SENSOR_ID_GYRO:
        case BME_SENSOR_ID_GYRO_WU:
        case BME_SENSOR_ID_GYRO_PASS:
        case BME_SENSOR_ID_GYRO_RAW:
        case BME_SENSOR_ID_GYRO_RAW_WU:
        case BME_SENSOR_ID_GYRO_BIAS:
        case BME_SENSOR_ID_GRA:
        case BME_SENSOR_ID_GRA_WU:
        case BME_SENSOR_ID_LACC:
            if((fifoBuffer->readPos + 7) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 7;   // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SENSOR_ID_LACC_WU:
            break;
        case BME_SENSOR_ID_RV:
        case BME_SENSOR_ID_RV_WU:
        case BME_SENSOR_ID_GAMERV:
        case BME_SENSOR_ID_GAMERV_WU:
        case BME_SENSOR_ID_GEORV:
        case BME_SENSOR_ID_GEORV_WU:
            if((fifoBuffer->readPos + 11) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 11;  // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        //case BME_SENSOR_ID_HEART:
        //case BME_SENSOR_ID_HEART_WU:
            //fifoBuffer->readPos += 2;
            //break;
        case BME_SENSOR_ID_GPS:
            if((fifoBuffer->readPos + 27) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 27;  // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SYS_SENSOR_ID_BSX_LOG_UPDATE_SUB:
        case BME_SYS_SENSOR_ID_BSX_LOG_DOSTEP:
            if((fifoBuffer->readPos + 23) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 23;  // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
        case BME_SYS_SENSOR_ID_DEBUG_MSG:
            if((fifoBuffer->readPos + 18) <= fifoBuffer->readLen) {
                fifoBuffer->readPos += 18;  // sizeof(ID + data)
            }
            else {
                res = BME_HIF_E_BUF;
            }
            break;
    }

    return res;
}

void startVirtSensor(uint16 id)
{
    int8 res;
    uint16 retLen = 0;
    float sampleRate = 0.0f;
    uint8 tempBuf[32];

    /* check sensor payload */
    memset(tempBuf, 0, sizeof(tempBuf));
    res = readParam(BME_PARAM_SENSOR_INFO_0 + id, tempBuf, sizeof(tempBuf), &retLen);
    if (res != BME_HIF_E_SUCCESS) {
        BME_DEBUG(("read param error : %d\n",res));
        return;
    }

    if (tempBuf[0] == 0) {
        BME_DEBUG(("sensor %d has no output\n"));
        return;
    }
#if 1    
    switch (id) {
        case BME_SENSOR_ID_GAIT:
            if ((tempBuf[20] - 1) != 68) {
                BME_DEBUG(("incorrect payload size : defined %d, read %d\n", 68, tempBuf[20] - 1));
                return;
            }
            sampleRate = 1.0f;
            break;
        default:
            sampleRate = 0.0f;
            BME_DEBUG(("not defined sensor id\n"));
            break;
    }
#else
    switch (id) {
        case BME_SENSOR_ID_GAIT:
            if ((tempBuf[20] - 1) != sizeof(gaitParameters)) {
                BME_DEBUG(("incorrect payload size : defined %d, read %s\n", sizeof(gaitParameters), tempBuf));
                return;
            }
            sampleRate = 1.0f;
            break;
        default:
            sampleRate = 0.0f;
            BME_DEBUG(("not defined sensor id\n"));
            break;
    }
#endif    

    if (sampleRate > 0.0f) {
        res = virtSensorFifoFlush(id);
        if (res != BME_HIF_E_SUCCESS) {
            BME_DEBUG(("sensor FIFO flush error : %d\n",res));
            return;
        }

        res = virtSensorConfig(id, *(uint32 *)&sampleRate, 0);
        if (res != BME_HIF_E_SUCCESS) {
            BME_DEBUG(("start sensor error : %d\n",res));
            return;
        }

        memset(tempBuf, 0, sizeof(tempBuf));
        res = readParam(BME_PARAM_SENSOR_CONF_0 + id, tempBuf, sizeof(tempBuf), &retLen);
        if (res != BME_HIF_E_SUCCESS) {
            BME_DEBUG(("read param error : %d\n",res));
            return;
        }

        /*
        uint32 tempSampleRate = tempBuf[0] + (tempBuf[1] << 8) + (tempBuf[2] << 16) + (tempBuf[3] << 24);
        float odr = *(float *)&tempSampleRate;
        uint32 lat = tempBuf[4] + (tempBuf[5] << 8) + (tempBuf[6] << 16) + (tempBuf[7] << 24);
        uint16 sensitivity = tempBuf[8] + (tempBuf[9] << 8);
        uint16 range = tempBuf[10] + (tempBuf[11] << 8);
        Serial.println("sensor config :"));
        Serial.println("ODR : " + String(odr));
        Serial.println("latency : " + String(lat));
        Serial.println("sensitivity : " + String(sensitivity));
        Serial.println("range : " + String(range));
        */
    }
}

void stopVirtSensor(uint16 id)
{
    int8 res;
    float sampleRate = 0.0f;

    res = virtSensorConfig(id, *(uint32 *)&sampleRate, 0);
    if (res != BME_HIF_E_SUCCESS) {
        BME_DEBUG(("stop sensor error : %d\n",res));
        return;
    }
}

int8 virtSensorFifoFlush(uint16 id)
{
    int8 res;
    uint8 buf[4];
    buf[0] = id & 0xFF;
    buf[1] = buf[2] = buf[3] = 0;
    res = executeCommand(BME_CMD_FIFO_FLUSH, buf, 4);

    return res;
}

int8 virtSensorConfig(uint16 id, uint32 sampleRate, uint32 latency)
{
    int8 res;
    uint8 buf[8];

    buf[0] = id;
    buf[1] = sampleRate & 0xFF;
    buf[2] = (sampleRate >> 8) & 0xFF;
    buf[3] = (sampleRate >> 16) & 0xFF;
    buf[4] = (sampleRate >> 24) & 0xFF;
    buf[5] = latency & 0xFF;
    buf[6] = (latency >> 8) & 0xFF;
    buf[7] = (latency >> 16) & 0xFF;
    res = executeCommand(BME_CMD_CONFIG_SENSOR, buf, sizeof(buf));

    return res;
}

void BME_startGaitSensor(uint8 cycleTime, float height)
{
    // cycle time of gait sensor is fixed to 2sec now
    // input cycleTime will be used later when gait sensor supports variable cycle time feature
    uint8 buf[6] = {BME_PARAM_GAIT_SENSOR_MODE_GAIT, 2, 0, 0, 0, 0};
        cycleTime = 2; //Temp
    memcpy(&buf[2], (uint8 *)&height, sizeof(height));
    writeParam(BME_PARAM_GAIT_SENSOR_MODE, buf, sizeof(buf));
    isGaitParametersUpdated = FALSE;
    gaitSensorMode = BME_PARAM_GAIT_SENSOR_MODE_GAIT;
    startVirtSensor(BME_SENSOR_ID_GAIT);
}

void BME_stopGaitSensor(void)
{
    stopVirtSensor(BME_SENSOR_ID_GAIT);
}

void BME_calibrateGaitSensorForward(uint8 duration)
{
    uint8 buf[2] = {BME_PARAM_GAIT_SENSOR_MODE_CAL_FORWARD, duration};
    writeParam(BME_PARAM_GAIT_SENSOR_MODE, buf, sizeof(buf));
    isGaitParametersUpdated = FALSE;
    gaitSensorMode = BME_PARAM_GAIT_SENSOR_MODE_CAL_FORWARD;
    startVirtSensor(BME_SENSOR_ID_GAIT);
}

void BME_calibrateGaitSensorDown(uint8 duration)
{
    uint8 buf[2] = {BME_PARAM_GAIT_SENSOR_MODE_CAL_DOWN, duration};
    writeParam(BME_PARAM_GAIT_SENSOR_MODE, buf, sizeof(buf));
    isGaitParametersUpdated = FALSE;
    gaitSensorMode = BME_PARAM_GAIT_SENSOR_MODE_CAL_DOWN;
    startVirtSensor(BME_SENSOR_ID_GAIT);
}

void BME_getGaitSensorCalibrationValue(float *cal)
{
    float temp[BME_PARAM_GAIT_SENSOR_CAL_LENGTH];
    uint16 retLen = 0;

    for (uint8 i=0; i<BME_PARAM_GAIT_SENSOR_CAL_LENGTH; i++) {
        temp[i] = 0.0F;
    }

    readParam(BME_PARAM_GAIT_SENSOR_CAL, (uint8 *)temp, sizeof(temp), &retLen);
    memcpy(cal, temp, sizeof(temp));
}

void BME_setGaitSensorCalibrationValue(float *cal)
{
    float temp[BME_PARAM_GAIT_SENSOR_CAL_LENGTH];
    memcpy(temp, cal, sizeof(temp));
    writeParam(BME_PARAM_GAIT_SENSOR_CAL, (uint8 *)temp, sizeof(temp));
}
