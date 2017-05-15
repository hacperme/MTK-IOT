IC_CONFIG                             = mt2523
BOARD_CONFIG                          = mt2523_hdk

PLATFORM_DEVICE = BAND
FORCE_DL_ENABLE = y


# v3 band senor config
# combo sensor of accelerometer and gyroscope
MT2511_E1 = y
MT2511_INTERFACE = SPI
MTK_SENSOR_ACCELEROMETER_USE = BMA255
MTK_SENSOR_BIO_USE = MT2511
MTK_SENSOR_BIO_MODULE = EVERLIGHT
MT2511_ENABLE_LEADOFF=
BMA255_ORIENTATION = OPT1
BMA255_USE_POLLING = y

# common project feature
MTK_SMART_BATTERY_ENABLE = y
MTK_NVDM_ENABLE = y

# USB PORT service for syslog
#MTK_PORT_SERVICE_ENABLE = y
#MTK_USB_DEMO_ENABLED = y

MTK_LED_ENABLE = y

# fusion algorithm
CFLAGS += -DSENSOR_MANAGER_LOG_DISABLE
CFLAGS += -DFUSION_REPORT_RAW_DATA_ENABLE

# debug mode
MTK_DEBUG_LEVEL = none

# lcm mirror screen
CFLAGS += -DLCD_MIRROR_DISPLAY

#enable graphic based task
GT_PROJECT_ENABLE = y
ifeq ($(GT_PROJECT_ENABLE),y)
# fusion algorithm
#FUSION_PEDOMETER_USE = M_INHOUSE_PEDOMETER
FUSION_SLEEP_TRACKER_USE = M_INHOUSE_SLEEP_TRACKER
FUSION_SLEEP_STAGING_USE = M_INHOUSE_SLEEP_STAGING
FUSION_HEART_RATE_MONITOR_USE = M_INHOUSE_HEART_RATE_MONITOR
FUSION_HEART_RATE_VARIABILITY_USE = M_INHOUSE_HEART_RATE_VARIABILITY
FUSION_BLOOD_PRESSURE_USE = M_INHOUSE_BLOOD_PRESSURE
endif

# bt module enable
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
MTK_BT_HFP_ENABLE                   = n
MTK_BT_AVRCP_ENABLE                 = n
MTK_BT_AVRCP_ENH_ENABLE             = n
MTK_BT_A2DP_ENABLE                  = n
MTK_BT_PBAP_ENABLE                  = n
MTK_BT_SPP_ENABLE                   = y
MTK_BT_AT_COMMAND_ENABLE            = y



