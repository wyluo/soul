#ifndef _BME300_HOST_INTERFACE_
#define _BME300_HOST_INTERFACE_

/* host interface error codes */
#define BME_HIF_E_SUCCESS           0
#define BME_HIF_E_INVALID           -1
#define BME_HIF_E_IO                -2
#define BME_HIF_E_MAGIC             -3
#define BME_HIF_E_CRC               -4
#define BME_HIF_E_TIMEOUT           -5
#define BME_HIF_E_BUF               -6
#define BME_HIF_E_INITIALIZED       -7
#define BME_HIF_E_BUS               -8
#define BME_HIF_E_NOMEM             -9

#define BME_DEFAULT_MAX_READ_BURST  44

/* registers */
#define BME_REG_CHAN_0                  0x0
#define BME_REG_CHAN_CMD                BME_REG_CHAN_0
#define BME_REG_CHAN_1                  0x1
#define BME_REG_CHAN_FIFO_W             BME_REG_CHAN_1
#define BME_REG_CHAN_2                  0x2
#define BME_REG_CHAN_FIFO_NW            BME_REG_CHAN_2
#define BME_REG_CHAN_3                  0x3
#define BME_REG_CHAN_STATUS             BME_REG_CHAN_3
#define BME_REG_CHIP_CTRL               0x5
#define BME_REG_HOST_INTERFACE_CTRL     0x6
#define BME_REG_HOST_INTERRUPT_CTRL     0x7
#define BME_REG_RESET_REQUEST           0x14
#define BME_REG_EV_TIME_REQ             0x15
#define BME_REG_HOST_CTRL               0x16
#define BME_REG_HOST_STATUS             0x17
#define BME_REG_CRC                     0x18    // 4 bytes
#define BME_REG_CHIP_ID                 0x1C
#define BME_REG_REVISION_ID             0x1D
#define BME_REG_ROM_VERSION             0x1E    // 2 bytes
#define BME_REG_RAM_VERSION             0x20    // 2 bytes
#define BME_REG_FLASH_VERSION           0x22    // 2 bytes
#define BME_REG_FEATURE_STATUS          0x24
#define BME_REG_BOOT_STATUS             0x25
#define BME_REG_HOST_IRQ_TIMESTAMP      0x26    // 5 bytes
#define BME_REG_INT_STATUS              0x2D
#define BME_REG_ERROR                   0x2E
#define BME_REG_INT_STATE               0x2F
#define BME_REG_DEBUG_VALUE             0x30
#define BME_REG_DEBUG_STATE             0x31

// chip control register (0x5) bits
#define BME_CHIPCTL_CPU_TURBO_DISABLE   (1 << 0)

// host interface control register (0x6) bits
#define BME_HIFCTL_ABORT_TRANSFER_CHANNEL_0     (1 << 0)
#define BME_HIFCTL_ABORT_TRANSFER_CHANNEL_1     (1 << 1)
#define BME_HIFCTL_ABORT_TRANSFER_CHANNEL_2     (1 << 2)
#define BME_HIFCTL_ABORT_TRANSFER_CHANNEL_3     (1 << 3)
#define BME_HIFCTL_AP_SUSPENDED                 (1 << 4)
#define BME_HIFCTL_NED_COORDINATES              (1 << 5)
#define BME_HIFCTL_IRQ_TIMESTAMP_CONTROL        (1 << 6)
#define BME_HIFCTL_ASYNC_STATUS_CHANNEL         (1 << 7)

// host interrupt control register (0x7) bits
#define BME_HICTL_DISABLE_FIFO_W        (1 << 0)
#define BME_HICTL_DISABLE_FIFO_NW       (1 << 1)
#define BME_HICTL_DISABLE_STATUS        (1 << 2)
#define BME_HICTL_DISABLE_DEBUG         (1 << 3)
#define BME_HICTL_DISABLE_FAULT         (1 << 4)
#define BME_HICTL_ACTIVE_LOW            (1 << 5)
#define BME_HICTL_EDGE                  (1 << 6)
#define BME_HICTL_OPEN_DRAIN            (1 << 7)

// reset request register (0x14) bits
#define BME_RSTREQ_RESET                (1 << 0)

// host control (0x16)
#define BME_HOCTL_SPI_3_WIRE            (1 << 0)
#define BME_HOCTL_I2C_WDOG_EN           (1 << 1)
#define BME_HOCTL_50MS_TIMEOUT          (1 << 2)

// chip ID register (0x1C) bits
#define BME_CHIP_ID                     0x89

// boot status register (0x25) bits
#define BME_BST_FLASH_DETECTED          (1 << 0)
#define BME_BST_FLASH_VERIFY_DONE       (1 << 1)
#define BME_BST_FLASH_VERIFY_ERROR      (1 << 2)
#define BME_BST_NO_FLASH                (1 << 3)
#define BME_BST_HOST_INTERFACE_READY    (1 << 4)
#define BME_BST_HOST_FW_VERIFY_DONE     (1 << 5)
#define BME_BST_HOST_FW_VERIFY_ERROR    (1 << 6)
#define BME_BST_HOST_FW_IDLE            (1 << 7)
#define BME_BST_CHECK_RETRY             100

// interrupt status register (0x2D) masks
#define BME_IST_MASK_ASSERTED           0x1
#define BME_IST_MASK_FIFO_W             0x6
#define BME_IST_MASK_FIFO_NW            0x18
#define BME_IST_MASK_STATUS             0x20
#define BME_IST_MASK_DEBUG              0x40
#define BME_IST_MASK_RESET_FAULT        0x80
#define BME_IST_FIFO_W_DRDY             0x2
#define BME_IST_FIFO_W_LTCY             0x4
#define BME_IST_FIFO_W_WM               0x6
#define BME_IST_FIFO_NW_DRDY            0x8
#define BME_IST_FIFO_NW_LTCY            0x10
#define BME_IST_FIFO_NW_WM              0x18
#define BME_IS_IRQ_FIFO_W(status)       ((status) & BME_IST_MASK_FIFO_W)
#define BME_IS_IRQ_FIFO_NW(status)      ((status) & BME_IST_MASK_FIFO_NW)
#define BME_IS_IRQ_STATUS(status)       ((status) & BME_IST_MASK_STATUS)
#define BME_IS_IRQ_ASYNC_STATUS(status) ((status) & BME_IST_MASK_DEBUG)
#define BME_IS_IRQ_RESET(status)        ((status) & BME_IST_MASK_RESET_FAULT)
#define BME_IST_MASK_FIFO               (BME_IST_MASK_FIFO_W | BME_IST_MASK_FIFO_NW)
#define BME_IS_IRQ_FIFO(status)         ((status) & BME_IST_MASK_FIFO)

/* command packets */
#define BME_CMD_REQ_POST_MORTEM_DATA   0x1
#define BME_CMD_UPLOAD_TO_PROGRAM_RAM  0x2
#define BME_CMD_BOOT_PROGRAM_RAM       0x3
#define BME_CMD_ERASE_FLASH            0x4
#define BME_CMD_WRITE_FLASH            0x5
#define BME_CMD_BOOT_FLASH             0x6
#define BME_CMD_SET_INJECT_MODE        0x7
#define BME_CMD_INJECT_DATA            0x8
#define BME_CMD_FIFO_FLUSH             0x9
#define BME_CMD_SW_PASSTHROUGH         0xA
#define BME_CMD_REQ_SELF_TEST          0xB
#define BME_CMD_REQ_FOC                0xC
#define BME_CMD_CONFIG_SENSOR          0xD
#define BME_CMD_CHANGE_RANGE           0xE
#define BME_CMD_TRIG_FATAL_ERROR       0x13
#define BME_CMD_BSX_LOGGING_CTRL       0x14
#define BME_CMD_FIFO_FORMAT_CTRL       0x15

/* command packet length, should be multiple of 4 bytes, at least 4 bytes */
#define BME_COMMAND_HEADER_LEN      4
#define BME_COMMAND_PACKET_LEN      44

/* system parameters */
#define BME_PARAM_FIFO_CTRL               0x103
#define BME_PARAM_SYS_VIRT_SENSOR_PRESENT 0x11F
#define BME_PARAM_SYS_PHYS_SENSOR_PRESENT 0x120
#define BME_PARAM_SYS_PHYS_SENSOR_INFO_0  0x120

/* physical sensor information parameters */
#define BME_PARAM_PHYSICAL_SENSOR_BASE    0x120

/* algorithm parameters */
#define BME_PARAM_CALIB_STATE_BASE        0x200
#define BME_PARAM_SIC                     0x27D
#define BME_PARAM_BSX_VERSION             0x27E
#define BME_PARAM_SET_SENSOR_CTRL         0x0E00
#define BME_PARAM_GET_SENSOR_CTRL         0x1E00
#define BME_PARAM_SENSOR_CTRL_FOC         0x1
#define BME_PARAM_SENSOR_CTRL_OIS         0x2
#define BME_PARAM_SENSOR_CTRL_FST         0x3
#define BME_PARAM_SENSOR_CTRL_READ        0x80

/* sensor parameters */
#define BME_PARAM_SENSOR_INFO_0           0x300
#define BME_PARAM_SENSOR_CONF_0           0x500

/* gait sensor parameters */
#define BME_PARAM_GAIT_SENSOR_MODE        0x900
#define BME_PARAM_GAIT_SENSOR_MODE_GAIT             0x00
#define BME_PARAM_GAIT_SENSOR_MODE_CAL_FORWARD      0x01
#define BME_PARAM_GAIT_SENSOR_MODE_CAL_DOWN         0x02
#define BME_PARAM_GAIT_SENSOR_CAL         0x901
#define BME_PARAM_GAIT_SENSOR_CAL_LENGTH  9

#define BME_QUERY_PARAM_STATUS_READY_MAX_RETRY 1000
#define BME_QUERY_FLASH_MAX_RETRY              1000

/* firmware header related fields */
#define BME_FW_MAGIC        0x662B

/* FIFO depth */
#define BME_DATA_FIFO_BUFFER_LENGTH     128 // malloc error test 512

/* sensor ID definitions */
#define BME_SYS_SENSOR_ID_MAX            256 // malloc error test  256
#define BME_SENSOR_ID_MAX                200

/* system data IDs */
#define BME_SYS_SENSOR_ID_TS_SMALL_DELTA        251
#define BME_SYS_SENSOR_ID_TS_LARGE_DELTA        252
#define BME_SYS_SENSOR_ID_TS_FULL               253
#define BME_SYS_SENSOR_ID_META_EVENT            254
#define BME_SYS_SENSOR_ID_TS_SMALL_DELTA_WU     245
#define BME_SYS_SENSOR_ID_TS_LARGE_DELTA_WU     246
#define BME_SYS_SENSOR_ID_TS_FULL_WU            247
#define BME_SYS_SENSOR_ID_META_EVENT_WU         248
#define BME_SYS_SENSOR_ID_FILLER                255
#define BME_SYS_SENSOR_ID_DEBUG_MSG             250
#define BME_SYS_SENSOR_ID_BSX_LOG_UPDATE_SUB    243
#define BME_SYS_SENSOR_ID_BSX_LOG_DOSTEP        244
#define BME_IS_SYS_SENSOR_ID(SENSOR_ID)         ((SENSOR_ID) >= 0xE0)

/* virtual sensor IDs */
#define BSX_VIRTUAL_SENSOR_ID_INVALID             (0)
#define BSX_OUTPUT_ID_ACCELERATION_PASSTHROUGH    (2)
#define BSX_CUSTOM_ID_ACCELERATION_PASSTHROUGH    (4)
#define BSX_OUTPUT_ID_ACCELERATION_RAW            (6)
#define BSX_OUTPUT_ID_ACCELERATION_CORRECTED      (8)
#define BSX_OUTPUT_ID_ACCELERATION_OFFSET         (10)
#define BSX_WAKEUP_ID_ACCELERATION_OFFSET         (182)
#define BSX_WAKEUP_ID_ACCELERATION_CORRECTED      (12)
#define BSX_WAKEUP_ID_ACCELERATION_RAW            (14)
#define BSX_CUSTOM_ID_ACCELERATION_CORRECTED      (16)
#define BSX_CUSTOM_ID_ACCELERATION_RAW            (18)
#define BSX_OUTPUT_ID_ANGULARRATE_PASSTHROUGH     (20)
#define BSX_CUSTOM_ID_ANGULARRATE_PASSTHROUGH     (22)
#define BSX_OUTPUT_ID_ANGULARRATE_RAW             (24)
#define BSX_OUTPUT_ID_ANGULARRATE_CORRECTED       (26)
#define BSX_OUTPUT_ID_ANGULARRATE_OFFSET          (28)
#define BSX_WAKEUP_ID_ANGULARRATE_OFFSET          (184)
#define BSX_WAKEUP_ID_ANGULARRATE_CORRECTED       (30)
#define BSX_WAKEUP_ID_ANGULARRATE_RAW             (32)
#define BSX_CUSTOM_ID_ANGULARRATE_CORRECTED       (34)
#define BSX_CUSTOM_ID_ANGULARRATE_RAW             (36)
#define BSX_OUTPUT_ID_MAGNETICFIELD_PASSTHROUGH   (38)
#define BSX_CUSTOM_ID_MAGNETICFIELD_PASSTHROUGH   (40)
#define BSX_OUTPUT_ID_MAGNETICFIELD_RAW           (42)
#define BSX_OUTPUT_ID_MAGNETICFIELD_CORRECTED     (44)
#define BSX_OUTPUT_ID_MAGNETICFIELD_OFFSET        (46)
#define BSX_WAKEUP_ID_MAGNETICFIELD_OFFSET        (186)
#define BSX_WAKEUP_ID_MAGNETICFIELD_CORRECTED     (48)
#define BSX_WAKEUP_ID_MAGNETICFIELD_RAW           (50)
#define BSX_CUSTOM_ID_MAGNETICFIELD_CORRECTED     (52)
#define BSX_CUSTOM_ID_MAGNETICFIELD_RAW           (54)
#define BSX_OUTPUT_ID_GRAVITY                     (56)
#define BSX_WAKEUP_ID_GRAVITY                     (58)
#define BSX_CUSTOM_ID_GRAVITY                     (60)
#define BSX_OUTPUT_ID_LINEARACCELERATION          (62)
#define BSX_WAKEUP_ID_LINEARACCELERATION          (64)
#define BSX_CUSTOM_ID_LINEARACCELERATION          (66)
#define BSX_OUTPUT_ID_ROTATION                    (68)
#define BSX_WAKEUP_ID_ROTATION                    (70)
#define BSX_CUSTOM_ID_ROTATION                    (72)
#define BSX_OUTPUT_ID_ROTATION_GAME               (74)
#define BSX_WAKEUP_ID_ROTATION_GAME               (76)
#define BSX_CUSTOM_ID_ROTATION_GAME               (78)
#define BSX_OUTPUT_ID_ROTATION_GEOMAGNETIC        (80)
#define BSX_WAKEUP_ID_ROTATION_GEOMAGNETIC        (82)
#define BSX_CUSTOM_ID_ROTATION_GEOMAGNETIC        (84)
#define BSX_OUTPUT_ID_ORIENTATION                 (86)
#define BSX_WAKEUP_ID_ORIENTATION                 (88)
#define BSX_CUSTOM_ID_ORIENTATION                 (90)
#define BSX_OUTPUT_ID_FLIP_STATUS                 (92)
#define BSX_CUSTOM_ID_FLIP_STATUS                 (94)
#define BSX_OUTPUT_ID_TILT_STATUS                 (96)
#define BSX_CUSTOM_ID_TILT_STATUS                 (98)
#define BSX_OUTPUT_ID_STEPDETECTOR                (100)
#define BSX_WAKEUP_ID_STEPDETECTOR                (188)
#define BSX_CUSTOM_ID_STEPDETECTOR                (102)
#define BSX_OUTPUT_ID_STEPCOUNTER                 (104)
#define BSX_WAKEUP_ID_STEPCOUNTER                 (106)
#define BSX_CUSTOM_ID_STEPCOUNTER                 (108)
#define BSX_OUTPUT_ID_SIGNIFICANTMOTION_STATUS    (110)
#define BSX_CUSTOM_ID_SIGNIFICANTMOTION_STATUS    (112)
#define BSX_OUTPUT_ID_WAKE_STATUS                 (114)
#define BSX_CUSTOM_ID_WAKE_STATUS                 (116)
#define BSX_OUTPUT_ID_GLANCE_STATUS               (118)
#define BSX_CUSTOM_ID_GLANCE_STATUS               (120)
#define BSX_OUTPUT_ID_PICKUP_STATUS               (122)
#define BSX_CUSTOM_ID_PICKUP_STATUS               (124)
#define BSX_OUTPUT_ID_ACTIVITY                    (126)
#define BSX_CUSTOM_ID_ACTIVITY                    (128)
#define BSX_OUTPUT_ID_PROPAGATION                 (130)
#define BSX_OUTPUT_ID_POSITION_STEPS              (132)
#define BSX_OUTPUT_ID_WRIST_TILT_STATUS           (134)
#define BSX_CUSTOM_ID_WRIST_TILT_STATUS           (136)
#define BSX_OUTPUT_ID_DEVICE_ORIENTATION          (138)
#define BSX_WAKEUP_ID_DEVICE_ORIENTATION          (140)
#define BSX_CUSTOM_ID_DEVICE_ORIENTATION          (142)
#define BSX_OUTPUT_ID_POSE_6DOF                   (144)
#define BSX_WAKEUP_ID_POSE_6DOF                   (146)
#define BSX_CUSTOM_ID_POSE_6DOF                   (148)
#define BSX_OUTPUT_ID_STATIONARY_DETECT           (150)
#define BSX_CUSTOM_ID_STATIONARY_DETECT           (152)
#define BSX_OUTPUT_ID_MOTION_DETECT               (154)
#define BSX_CUSTOM_ID_MOTION_DETECT               (156)
#define BSX_OUTPUT_ID_STANDBY_STATUS              (158)
#define BSX_OUTPUT_ID_ACCELERATION_STATUS         (160)
#define BSX_OUTPUT_ID_ACCELERATION_DYNAMIC        (162)
#define BSX_OUTPUT_ID_ANGULARRATE_STATUS          (164)
#define BSX_OUTPUT_ID_MAGNETICFIELD_STATUS        (166)
#define BSX_OUTPUT_ID_ANGULARRATE_M4G             (168)
#define BSX_WAKEUP_ID_ANGULARRATE_M4G             (170)
#define BMEID(bsx_id)                             ((bsx_id) >> 1)
#define BSX_SENSOR_ID_MAX                         0x7F

#define NONBSX_SENSOR_ID_MAX                     0x9F
#define NONBSX_SENSOR_ID_TEMPERATURE             0x80
#define NONBSX_SENSOR_ID_PRESSURE                0x81
#define NONBSX_SENSOR_ID_HUMIDITY                0x82
#define NONBSX_SENSOR_ID_GAS                     0x83
#define NONBSX_SENSOR_ID_WAKE_TEMPERATURE        0x84
#define NONBSX_SENSOR_ID_WAKE_PRESSURE           0x85
#define NONBSX_SENSOR_ID_WAKE_HUMIDITY           0x86
#define NONBSX_SENSOR_ID_WAKE_GAS                0x87
#define NONBSX_SENSOR_ID_STEP_COUNTER            0x88
#define NONBSX_SENSOR_ID_STEP_DETECTOR           0x89
#define NONBSX_SENSOR_ID_SIGNIFICANT_MOTION      0x8A
#define NONBSX_SENSOR_ID_WAKE_STEP_COUNTER       0x8B
#define NONBSX_SENSOR_ID_WAKE_STEP_DETECTOR      0x8C
#define NONBSX_SENSOR_ID_WAKE_SIGNIFICANT_MOTION 0x8D
#define NONBSX_SENSOR_ID_ANY_MOTION              0x8E
#define NONBSX_SENSOR_ID_WAKE_ANY_MOTION         0x8F
#define NONBSX_SENSOR_ID_EXCAMERA                0x90
#define NONBSX_SENSOR_ID_GPS                     0x91
#define NONBSX_SENSOR_ID_LIGHT                   0x92
#define NONBSX_SENSOR_ID_WAKE_LIGHT              0x94
#define NONBSX_SENSOR_ID_PROXIMITY               0x93
#define NONBSX_SENSOR_ID_WAKE_PROXIMITY          0x95

#define SENSOR_TYPE_CUSTOMER_VISIBLE_START  (0xA0)
#define SENSOR_TYPE_CUSTOMER_VISIBLE_END    (0xBF)

#define BME_SENSOR_ID_TBD           (BME_SENSOR_ID_MAX - 1)
#define BME_SENSOR_ID_ACC           BMEID(BSX_OUTPUT_ID_ACCELERATION_CORRECTED)
#define BME_SENSOR_ID_ACC_WU        BMEID(BSX_WAKEUP_ID_ACCELERATION_CORRECTED)
#define BME_SENSOR_ID_ACC_PASS      BMEID(BSX_OUTPUT_ID_ACCELERATION_PASSTHROUGH)
#define BME_SENSOR_ID_MAG           BMEID(BSX_OUTPUT_ID_MAGNETICFIELD_CORRECTED)
#define BME_SENSOR_ID_MAG_WU        BMEID(BSX_WAKEUP_ID_MAGNETICFIELD_CORRECTED)
#define BME_SENSOR_ID_MAG_PASS      BMEID(BSX_OUTPUT_ID_MAGNETICFIELD_PASSTHROUGH)
#define BME_SENSOR_ID_ORI           BMEID(BSX_OUTPUT_ID_ORIENTATION)
#define BME_SENSOR_ID_ORI_WU        BMEID(BSX_WAKEUP_ID_ORIENTATION)
#define BME_SENSOR_ID_GYRO          BMEID(BSX_OUTPUT_ID_ANGULARRATE_CORRECTED)
#define BME_SENSOR_ID_GYRO_WU       BMEID(BSX_WAKEUP_ID_ANGULARRATE_CORRECTED)
#define BME_SENSOR_ID_GYRO_PASS     BMEID(BSX_OUTPUT_ID_ANGULARRATE_PASSTHROUGH)
#define BME_SENSOR_ID_LIGHT         NONBSX_SENSOR_ID_LIGHT
#define BME_SENSOR_ID_LIGHT_WU      NONBSX_SENSOR_ID_WAKE_LIGHT
#define BME_SENSOR_ID_BARO          NONBSX_SENSOR_ID_PRESSURE
#define BME_SENSOR_ID_BARO_WU       NONBSX_SENSOR_ID_WAKE_PRESSURE
#define BME_SENSOR_ID_TEMP          NONBSX_SENSOR_ID_TEMPERATURE
#define BME_SENSOR_ID_TEMP_WU       NONBSX_SENSOR_ID_WAKE_TEMPERATURE
#define BME_SENSOR_ID_PROX          NONBSX_SENSOR_ID_PROXIMITY
#define BME_SENSOR_ID_PROX_WU       NONBSX_SENSOR_ID_WAKE_PROXIMITY
#define BME_SENSOR_ID_GRA           BMEID(BSX_OUTPUT_ID_GRAVITY)
#define BME_SENSOR_ID_GRA_WU        BMEID(BSX_WAKEUP_ID_GRAVITY)
#define BME_SENSOR_ID_LACC          BMEID(BSX_OUTPUT_ID_LINEARACCELERATION)
#define BME_SENSOR_ID_LACC_WU       BMEID(BSX_WAKEUP_ID_LINEARACCELERATION)
#define BME_SENSOR_ID_RV            BMEID(BSX_OUTPUT_ID_ROTATION)
#define BME_SENSOR_ID_RV_WU         BMEID(BSX_WAKEUP_ID_ROTATION)
#define BME_SENSOR_ID_RV_CUS        BMEID(BSX_CUSTOM_ID_ROTATION)
#define BME_SENSOR_ID_HUM           NONBSX_SENSOR_ID_HUMIDITY
#define BME_SENSOR_ID_HUM_WU        NONBSX_SENSOR_ID_WAKE_HUMIDITY
#define BME_SENSOR_ID_ATEMP         BME_SENSOR_ID_TBD
#define BME_SENSOR_ID_ATEMP_WU      BME_SENSOR_ID_TBD
#define BME_SENSOR_ID_MAGU          BMEID(BSX_OUTPUT_ID_MAGNETICFIELD_RAW)
#define BME_SENSOR_ID_MAGU_WU       BMEID(BSX_WAKEUP_ID_MAGNETICFIELD_RAW)
#define BME_SENSOR_ID_GAMERV        BMEID(BSX_OUTPUT_ID_ROTATION_GAME)
#define BME_SENSOR_ID_GAMERV_WU     BMEID(BSX_WAKEUP_ID_ROTATION_GAME)
#define BME_SENSOR_ID_GYROU         BMEID(BSX_OUTPUT_ID_ANGULARRATE_RAW)
#define BME_SENSOR_ID_GYROU_WU      BMEID(BSX_WAKEUP_ID_ANGULARRATE_RAW)
#define BME_SENSOR_ID_SIG           BMEID(BSX_OUTPUT_ID_SIGNIFICANTMOTION_STATUS)
/* only has wakeup type */
#define BME_SENSOR_ID_SIG_WU        BME_SENSOR_ID_SIG
#define BME_SENSOR_ID_SIG_HW        BME_SENSOR_ID_TBD
/* only has wakeup type */
#define BME_SENSOR_ID_SIG_HW_WU     NONBSX_SENSOR_ID_WAKE_SIGNIFICANT_MOTION
#define BME_SENSOR_ID_STD           BMEID(BSX_OUTPUT_ID_STEPDETECTOR)
#define BME_SENSOR_ID_STD_WU        BMEID(BSX_WAKEUP_ID_STEPDETECTOR)
#define BME_SENSOR_ID_STD_HW        NONBSX_SENSOR_ID_STEP_DETECTOR
#define BME_SENSOR_ID_STD_HW_WU     NONBSX_SENSOR_ID_WAKE_STEP_DETECTOR
#define BME_SENSOR_ID_STC           BMEID(BSX_OUTPUT_ID_STEPCOUNTER)
#define BME_SENSOR_ID_STC_WU        BMEID(BSX_WAKEUP_ID_STEPCOUNTER)
#define BME_SENSOR_ID_STC_HW        NONBSX_SENSOR_ID_STEP_COUNTER
#define BME_SENSOR_ID_STC_HW_WU     NONBSX_SENSOR_ID_WAKE_STEP_COUNTER
#define BME_SENSOR_ID_GEORV         BMEID(BSX_OUTPUT_ID_ROTATION_GEOMAGNETIC)
#define BME_SENSOR_ID_GEORV_WU      BMEID(BSX_WAKEUP_ID_ROTATION_GEOMAGNETIC)
#define BME_SENSOR_ID_HEART         BME_SENSOR_ID_TBD
#define BME_SENSOR_ID_HEART_WU      BME_SENSOR_ID_TBD
#define BME_SENSOR_ID_TILT          BMEID(BSX_OUTPUT_ID_TILT_STATUS)
/* only has wakeup type */
#define BME_SENSOR_ID_TILT_WU       BME_SENSOR_ID_TILT
#define BME_SENSOR_ID_WRIST_TILT_GUESTURE   BMEID(BSX_OUTPUT_ID_WRIST_TILT_STATUS)
#define BME_SENSOR_ID_WAKE          BMEID(BSX_OUTPUT_ID_WAKE_STATUS)
/* only has wakeup type */
#define BME_SENSOR_ID_WAKE_WU       BME_SENSOR_ID_WAKE
#define BME_SENSOR_ID_GLANCE        BMEID(BSX_OUTPUT_ID_GLANCE_STATUS)
/* only has wakeup type */
#define BME_SENSOR_ID_GLANCE_WU     BME_SENSOR_ID_GLANCE
#define BME_SENSOR_ID_PICKUP        BMEID(BSX_OUTPUT_ID_PICKUP_STATUS)
/* only has wakeup type */
#define BME_SENSOR_ID_PICKUP_WU     BME_SENSOR_ID_PICKUP
#define BME_SENSOR_ID_DEVICE_ORI    BMEID(BSX_OUTPUT_ID_DEVICE_ORIENTATION)
#define BME_SENSOR_ID_DEVICE_ORI_WU BMEID(BSX_WAKEUP_ID_DEVICE_ORIENTATION)
#define BME_SENSOR_ID_STATIONARY_DET    BMEID(BSX_OUTPUT_ID_STATIONARY_DETECT)
#define BME_SENSOR_ID_MOTION_DET    BMEID(BSX_OUTPUT_ID_MOTION_DETECT)
#define BME_SENSOR_ID_AR            BMEID(BSX_OUTPUT_ID_ACTIVITY)
/* only has wakeup type */
#define BME_SENSOR_ID_AR_WU         BME_SENSOR_ID_AR
#define BME_SENSOR_ID_ACC_RAW       BMEID(BSX_OUTPUT_ID_ACCELERATION_RAW)
#define BME_SENSOR_ID_ACC_RAW_WU    BMEID(BSX_WAKEUP_ID_ACCELERATION_RAW)
#define BME_SENSOR_ID_MAG_RAW       BMEID(BSX_OUTPUT_ID_MAGNETICFIELD_RAW)
#define BME_SENSOR_ID_MAG_RAW_WU    BMEID(BSX_WAKEUP_ID_MAGNETICFIELD_RAW)
#define BME_SENSOR_ID_GYRO_RAW      BMEID(BSX_OUTPUT_ID_ANGULARRATE_RAW)
#define BME_SENSOR_ID_GYRO_RAW_WU   BMEID(BSX_WAKEUP_ID_ANGULARRATE_RAW)
#define BME_SENSOR_ID_ACC_BIAS      BMEID(BSX_OUTPUT_ID_ACCELERATION_OFFSET)
#define BME_SENSOR_ID_MAG_BIAS      BMEID(BSX_OUTPUT_ID_MAGNETICFIELD_OFFSET)
#define BME_SENSOR_ID_GYRO_BIAS     BMEID(BSX_OUTPUT_ID_ANGULARRATE_OFFSET)
#define BME_SENSOR_ID_ANY_MOTION    NONBSX_SENSOR_ID_ANY_MOTION
#define BME_SENSOR_ID_ANY_MOTION_WU NONBSX_SENSOR_ID_WAKE_ANY_MOTION
#define BME_SENSOR_ID_GAS           NONBSX_SENSOR_ID_GAS
#define BME_SENSOR_ID_GAS_WU        NONBSX_SENSOR_ID_WAKE_GAS
#define BME_SENSOR_ID_GPS           NONBSX_SENSOR_ID_GPS
#define BME_SENSOR_ID_EXCAMERA      NONBSX_SENSOR_ID_EXCAMERA
#define BME_SENSOR_ID_GAIT          SENSOR_TYPE_CUSTOMER_VISIBLE_START

/* meta event definitions */
#define BME_META_EVENT_FLUSH_COMPLETE           1
#define BME_META_EVENT_SAMPLE_RATE_CHANGED      2
#define BME_META_EVENT_POWER_MODE_CHANGED       3
#define BME_META_EVENT_ALGORITHM_EVENTS         5
#define BME_META_EVENT_SENSOR_STATUS            6
#define BME_META_EVENT_BSX_DO_STEPS_MAIN        7
#define BME_META_EVENT_BSX_DO_STEPS_CALIB       8
#define BME_META_EVENT_BSX_GET_OUTPUT_SIGNAL    9
#define BME_META_EVENT_RESERVED1                10
#define BME_META_EVENT_SENSOR_ERROR             11
#define BME_META_EVENT_FIFO_OVERFLOW            12
#define BME_META_EVENT_DYNAMIC_RANGE_CHANGED    13
#define BME_META_EVENT_FIFO_WATERMARK           14
#define BME_META_EVENT_RESERVED2                15
#define BME_META_EVENT_INITIALIZED              16
#define BME_META_TRANSFER_CAUSE                 17
#define BME_META_EVENT_SENSOR_FRAMEWORK         18
#define BME_META_EVENT_RESET                    19
#define BME_META_EVENT_SPACER                   20

#define BME_EN		53
#define BME_RST		19

#define BME_SPI_CLK		20
#define BME_SPI_MISO	18
#define BME_SPI_MOSI	17
#define BME_SPI_INT		16
#define BME_SPI_CS		15
#define BME_CLK_KHZ		8000


/* utils */
#define BME_LE2U16(x)       ((uint16_t)((x)[0] | (x)[1] << 8))

typedef struct _virt_sensor_output_gait {
    uint16 header;
    uint8 state;
    uint8 stepCount;
    uint32 elapsed;
    float cadence;
    float stepWidth;
    float headAngle;
    float wssl_rgtl;
    float wssr_rgtr;
    float wdsl_rftl;
    float wdsr_rgtr;
    float wfpl_rmll;
    float wfpr_rmlr;
    float shock;
    float symmetry;
    float vo;
    float legStiffness;
    float consistency;
    float speed;
} virtSensorOutputGait_t;


#endif
