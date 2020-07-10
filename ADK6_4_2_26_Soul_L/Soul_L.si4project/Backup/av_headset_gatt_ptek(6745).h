#ifndef AV_HEADSET_GATT_PTEK_H
#define AV_HEADSET_GATT_PTEK_H

#ifdef INCLUDE_PTEK
#include <gatt_heart_rate_server.h>
#include <csrtypes.h>
#include <message.h>

/* Heart Rate notification update time as per spec the heart rate measurements 
    shall be notified for every one second.
*/
#define GATT_SERVER_HR_UPDATE_TIME    1000

/* This timeout is to stop the Heart Rate notifications which is simluated within the application
    Note: This is to be removed when actual sensor implementation is in place.
    
*/
#define STOP_HR_NOTIFICATION_TIME    10000

/* Max HR data that can be send in one ATT notification */
#define MAX_ATT_HR_NOTI_LEN                               (20)

/* HEART RATE MEASUREMENT FLAGS */

#define SENSOR_MEASUREVAL_FORMAT_UINT8   (0x00)
#define SENSOR_MEASUREVAL_FORMAT_UINT16  (0x01)

/* Sensor contact bit */

/* Sensor contact feature support (bit 2 of the flag) */
#define SENSOR_CONTACT_FEATURE_SUPPORTED (0x04)

/* Sensor contact bits when feature is supported (bit 1 - sensor contact bit)*/
#define SENSOR_NOT_IN_CONTACT \
                                       (SENSOR_CONTACT_FEATURE_SUPPORTED | 0x00)
#define SENSOR_IN_CONTACT\
                                       (SENSOR_CONTACT_FEATURE_SUPPORTED | 0x02)

/* RR-Interval status bit (Bit 4 of the flag) */
#define RR_INTERVAL_PRESENT             (0x10)

/* Heart Rate used as a base for HR Measurements */
#define HEART_RATE_IN_BPM               (72)

/* Dummy RR interval in milliseconds */
#define RR_INTERVAL_IN_MS              (0x1e0)

/* Extract low order byte of 16-bit */
#define LE8_L(x)                       ((x) & 0xff)

/* Extract low order byte of 16-bit */
#define LE8_H(x)                       (((x) >> 8) & 0xff)

//#define PTEK_I2C_SLAVE_ADDR     0x45
//#define PTEK_I2C_WRITE_ADDR     0x8A
//#define PTEK_I2C_READ_ADDR		0x8B

#define PTEK_RSP_LENGTH_OFFSET  0x7C
#define PTEK_READ_OFFSET		0x7F

#define PTEK_PKT_START_CHAR     0x44

#define PTEK_CMD_START			0x01
#define PTEK_CMD_STOP           0x02
#define PTEK_CMD_SET            0x04
#define PTEK_CMD_GET            0x08
#define PTEK_CMD_ACK            0x10
#define PTEK_CMD_DATA           0x20

#define PTEK_POST_RESULT        0x13
#define PTEK_HEART_RATE         0x20
#ifdef SOUL_BLADE		
#define PTEK_OFFSKIN            0x11	
#endif


typedef struct __ptek_config
{
    /*! The I2C clock frequency */
    uint16 i2c_clock_khz;
    /*! The PIOs used to control/communicate with PTEK */
    struct
    {
        /*! PIO used to power-on PTEK */
        uint8 on;
        /*! PIO used to reset PTEK */
        uint8 rst;
        /*! Interrupt PIO driven by PTEK */
        uint8 interrupt;
        /*! Interrupt out PIO driven by PTEK */
        uint8 int_out;
        /*! I2C serial data PIO */
        uint8 i2c_sda;
        /*! I2C serial clock PIO */
        uint8 i2c_scl;
    } pios;
}ptekConfig;



#if 1
void appBleHRSensorInContact(GATT_HR_SERVER_TIMER_IND_T * ind);
void appBleHRSensorNotInContact(uint16 cid);
#endif

bool ptekGetIntOutPinStatus(const ptekConfig *config);

bitserial_handle ptekEnable(const ptekConfig *config);
void ptekDisable(const ptekConfig *config);
void ptekReset(const ptekConfig *config);
void PtekIR_Det(void);
void Cancel_Advertising_HR(void);

void handleReadHrMeasurementClientConfig(GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T * ind);
void handleWriteHrMeasurementClientConfig(GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind);
#endif
#endif // AV_HEADSET_GATT_PTEK_H
