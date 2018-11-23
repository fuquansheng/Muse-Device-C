/* Copyright 2015 LifeQ Global Ltd.
 * All Rights Reserved. All information contained herein is, and remains the
 * property of LifeQ Global Ltd. and/or its licensors. Unauthorized copying
 * of this file via any medium is strictly prohibited. The intellectual and
 * technical concepts contained herein are proprietary to LifeQ Global Ltd.
 * and/or its licensors and may be covered by patents, patents in process, and
 * are protected by trade secret or copyright law. Dissemination or use of
 * this software or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained from LifeQ Global Ltd.
 */


/**
 * @file pp_config.h
 * @author Eugene Pretorius
 * @date 2015/06/19
 * @brief Heart rate algorithm configuration API
 * ---------------------------------------------
 * This file defines the available customizations for configuration of the
 * LifeQ heart rate algorithm behavior.
 ******************************************************************************/


#ifndef __lifeqinside__lifeq_config__
#define __lifeqinside__lifeq_config__

#include <stdbool.h>
#include <stdint.h>
#include "hqerror.h"


#if (defined(EMBEDDED_ARM) && ~defined(EMBEDDED))
#define EMBEDDED
#endif


//If a function is marked for removal or not supported in future releases.
#ifdef __MINGW_ATTRIB_DEPRECATED
#define __lifeq_deprecated__      __MINGW_ATTRIB_DEPRECATED
#endif

#ifndef __lifeq_deprecated__
#if !defined(EMBEDDED) && !defined(__MINGW_ATTRIB_DEPRECATED)
#define __lifeq_deprecated__      __attribute__ ((deprecated))
#else
#define __lifeq_deprecated__
#endif
#endif



//If a function has the not implemented tag it should not be called and is reserved for future implementation.
#ifndef __not_implemented__
#define __not_implemented__
#endif


//If a function has the experimental tag it is not considered ready for commercial use.
#ifndef __experimental__
#define __experimental__
#endif

#define CONFIG_OFF			0	/// see config_t
#define CONFIG_ON			1	//  see config_t
#define CONFIG_DEFAULT		2   //  see config_t

typedef enum {
    OFF     = CONFIG_OFF,		/// Property is disabled.
    ON      = CONFIG_ON,		/// Property is activated.
    DEFAULT = CONFIG_DEFAULT    /// Property default value is set.
} config_t;	        /// Configuration options applied properties.


/**
 * Available window durations.
 */
typedef enum {
    PP_CONFIG_WINDOW_UNSET =  0,
    PP_CONFIG_WINDOW_1SEC  =  1000,   /// Select a 1 second window.
    PP_CONFIG_WINDOW_2SEC  =  2000,   /// Select a 2 second window.
    PP_CONFIG_WINDOW_3SEC  =  3000,   /// Select a 3 second window.
    PP_CONFIG_WINDOW_5SEC  =  5000,   /// Select a 5 second window.
    PP_CONFIG_WINDOW_7SEC  =  7000,   /// Select a 5 second window.
    PP_CONFIG_WINDOW_10SEC = 10000,   /// Select a 10 second window.
    PP_CONFIG_WINDOW_20SEC = 20000,   /// Select a 20 second window.
    PP_CONFIG_WINDOW_30SEC = 30000,   /// Select a 30 second window.
    PP_CONFIG_WINDOW_40SEC = 40000,   /// Select a 40 second window.
    PP_CONFIG_WINDOW_50SEC = 50000    /// Select a 50 second window.
} config_sec_t;                       /// Collection of default timing options.


/**
 * LifeQ algorithm operation modes.
 */
typedef enum {
    PP_CONFIG_LIFE_STYLE  = 0x00,    /// Non-exercise to aid calorie and vo2 calculations.
    PP_CONFIG_EXERCISE    = 0xB0,    /// ACTIVITY_EXERCISE_OTHER, unspecified activity.
    PP_CONFIG_WALKING     = 0x50,    /// Parameters optimised for walking-type exercise.
    PP_CONFIG_RUNNING     = 0x60,    /// Parameters optimised for running-type exercise.
    PP_CONFIG_ROWING      = 0xB1,    /// Parameters optimised for rowing-type exercise.
    PP_CONFIG_CYCLING     = 0xB3,    /// Parameters optimised for cycling-type exercise.
    PP_CONFIG_GYM_WEIGHT  = 0xA1,    /// Parameters optimised for weight-type exercise.

    PP_CONFIG_SLEEP       = 0x20,    /// Parameters optimised for sleep.
    //    PP_CONFIG_SWIMMING  =  13,      __not_implemented__
    
    PP_CONFIG_AUTO        = 0xFF,    /// Auto detect user activity

} config_mode_t;


/**
 * Available input data speed rates. @see PP_Init and PP_Set_Fs
 */
typedef enum {
  DEVICE_DATA_INPUT_FS_25HZ  =  25,
  DEVICE_DATA_INPUT_FS_50HZ  =  50,
  DEVICE_DATA_INPUT_FS_128HZ = 128,
} config_input_fs_t ;


/**
 * Available accelerometer scaling ranges. @see PP_Set_Gs
 */
typedef enum {
  DEVICE_CONFIG_ACC_2G  =  2,
  DEVICE_CONFIG_ACC_4G  =  4,
  DEVICE_CONFIG_ACC_6G  =  6,
  DEVICE_CONFIG_ACC_8G  =  8,
} config_g_t;


typedef enum {
    AFE_TI4404  = 0,
    AFE_TI4405  = 1,
    AFE_A = 10,
} config_afe_t;


/**
 * Available LED wavelengths. @see PP_Set_SkinDetectWavelength
 */
typedef enum {
    LED_WL_GREEN,
    LED_WL_RED,
    LED_WL_IRED
} led_wl_t;


typedef enum {
    CONFIG_WEAR_DEVICE_DISABLED  = 0,     ///< Default value, uses skin detection on leds.
    CONFIG_WEAR_DEVICE_OFF  = 1,          ///< Overrides the sensor on skin detection as OFF.
    CONFIG_WEAR_DEVICE_ON  = 2,           ///< Overrides the sensor on skin detection as ON.
} config_wear_detection_t;


/**
 * If using an external wear/skin detection then you can specify this here.
 * This setting overrides skin detection based on other methods and
 * @see PP_IsSensorContactDetected will return this setting.
 *
 * @param val   Toggle ON, OFF, DEFAULT for wear detected.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_IsDeviceOnUser(config_wear_detection_t val);


/**
 * Reset configuration settings to DEFAULT's.
 * If called after @see PP_Init(config_input_fs_t fs);
 * it sets the fs input with the default.
 * Call PP_Set_Defaults() first and PP_Init(...) second.

 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_Defaults(void);


/**
 * Activate the LifeQ calculation forced in a mode of operation.
 * Used to optimize algorithms in short term and to train better algorithms in
 * the long term.
 * Overrides auto switching of activities.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_ActivityMode(config_mode_t config_property);


/**
 * Switch on/off detection of off-skin events. HR reports 0bpm while off-skin
 * events occur.
 * A feedback mechanism for partners and users to identify in a robust and
 * sensitive manner how strap or mechanical design, affects on-off skin events
 * during i.e. during exercise.
 * @param val   Toggle ON, OFF, DEFAULT
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_SkinDetect(config_t val);


/**
 * LED wavelengths to be used for skin detection.
 * @param LED_WL_DC   LED wavelength for DC check.
 * @param LED_WL_AC   LED wavelength for AC check.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_SkinDetectWavelength(led_wl_t LED_WL_DC, led_wl_t LED_WL_AC);


/**
 * THRESHOLD for ambient upper bound on skin detection.
 * @param thresh Upper ambient boundary for skin detection, value supplied by LifeQ.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_SkinDetectBlankThreshold(uint32_t thresh);


/**
 * THRESHOLD for LED lower and upper bound on skin detection.
 * Parameters that affect the sensitivity and range for the infrared proximity
 * detection.
 * @param low   Lower boundary for skin detection, value supplied by LifeQ.
 * @param high  Upper boundary for skin detection, value supplied by LifeQ.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_SkinDetectThreshold(uint32_t low, uint32_t high);


/**
 * THRESHOLD for LED lower and upper bound on AC skin detection.
 * Parameters that affect the sensitivity and range for the green AC
 * detection.
 * @param low   Lower boundary for AC skin detection, value supplied by LifeQ.
 * @param high  Upper boundary for AC skin detection, value supplied by LifeQ.
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_SkinDetectACThreshold(uint32_t low, uint32_t high);


/**
 * Select the number of seconds to wait for active cadence before reporting values.
 * Before reporting cadence a level of certainty is required to determine if constant
 * rhythmic motion is detected.
 * Default used PP_CONFIG_WINDOW_1SEC
 * @param val   Accepts OFF, ON, DEFAULT config_t or input range [0 - 10 000] msec
 * @return RET_OK if successful, else hqret_t error if out of range.
 */
hqret_t PP_Set_CadenceWait(uint16_t val);


/**
 * Set sampling rate of the input signals.
 * @param val  DEVICE_DATA_INPUT_FS_25HZ,
               DEVICE_DATA_INPUT_FS_50HZ,
               DEVICE_DATA_INPUT_FS_128HZ
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_Fs(config_input_fs_t val) ;


/**
 * Set G range of accelerometer.
 * @param val  DEVICE_CONFIG_ACC_2G,
               DEVICE_CONFIG_ACC_4G,
               DEVICE_CONFIG_ACC_6G,
               DEVICE_CONFIG_ACC_8G
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_Gs(config_g_t val);


/**
 * Switch on/off AC based cadence tracking.
 * @param val   Toggle ON, OFF, DEFAULT
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_AC_Cadence_Tracking(config_t val);


/**
 * Switch on/off automatic HR recovery.
 * NOTE: Requires AC based cadence tracking.
 * @param val   Toggle ON, OFF, DEFAULT
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_Recovery(config_t val) __experimental__;


/**
 * Switch on/off HR initialisation by peak detection. If off, HR is seeded at 1Hz.
 * @param val   Toggle ON, OFF, DEFAULT
 * @return RET_OK if successful, else hqret_t error.
 */
hqret_t PP_Set_Peak_Initialisation(config_t val);


/**
 * This feature allows device manufacturers to suppress the very short
 * physiological deviations in heart rate for users that are not comfortable
 * with the natural variations.
 * @param val   Accepts OFF, ON, DEFAULT config_t or input range [500 - 10 000] msec
 * @return RET_OK if successful, else hqret_t error if out of range.
 */
hqret_t PP_Set_HeartRateSmoothWindow(uint16_t val);


/**
 * Fill in gaps in heart rate measurements due to off-skin events or tracking loss.
 * Instead of outputting 0bpm, report the best estimate available.
 *
 * Mechanical designs are not perfect, therefore we can compensate for possible
 * signal loss and allow partners/users to estimation the heart rate in periods
 * where off-skin events occur or tracking loses the signal.
 *
 * Default window used PP_CONFIG_WINDOW_1SEC, PP_CONFIG_WINDOW_UNSET switches off.
 *
 * @param val   Accepts OFF, ON, DEFAULT config_t or input range [500 - 10 000] msec
 * @return RET_OK if successful, else hqret_t error if out of range.
 */
hqret_t PP_Set_HeartRateFillerWindow(uint16_t val);


/**
 * Select the number of seconds to 'fill' with estimate cadence predictions,
 * when FillerEstimate is used.
 *
 * Set behavior when cadence signal is lost instead of outputting 0 rpm, report the best
 * estimate available. Default window used PP_CONFIG_WINDOW_3SEC.
 *
 * @param val   Accepts OFF, ON, DEFAULT config_t or input range [0 - 10 000] msec
 * @return RET_OK if successful, else hqret_t error if out of range.
 */
hqret_t PP_Set_CadenceFillerWindow(uint16_t val);



/**
 * Set the AFE used by the current device. Only available for unit testing.
 * @param @see config_afe_t to enable  PP_addSamplesTI4404.
 */
hqret_t PP_Set_Device_AFE(config_afe_t use_afe);


/**
 * Get configuration: ActivityMode. @see PP_Set_ActivityMode
 * @return config_mode_t
 */
config_mode_t PP_Get_ActivityMode(void);


/**
 * Get configuration: SkinDetect. @see PP_Set_SkinDetect
 * @return RET_OK if successful, else hqret_t error.
 */
config_t PP_Get_SkinDetect(void);


/**
 * Get configuration: Skin detection LED wavelengths. @see PP_Set_SkinDetectWavelength
 * @see PP_Set_SkinDetectWavelength
 * @return wavelength.
 */
led_wl_t PP_Get_SkinDetectWavelengthDC(void);
led_wl_t PP_Get_SkinDetectWavelengthAC(void);

/*
 * Get configuration: Upper limit on ambient for skin detection. @see PP_Set_SkinDetectBlankThreshold
 * @return Upper ambient limit value
 */
uint32_t PP_Get_SkinDetectBlankThreshold(void);

/**
 * Get configuration: Lower boundary for skin detection. @see PP_Set_SkinDetectThreshold
 * @return Lower skin detect threshold value
 */
uint32_t PP_Get_SkinDetectThresholdLow(void);


/**
 * Get configuration: Upper boundary for skin detection. @see PP_Set_SkinDetectThreshold
 * @return Upper skin detect threshold value
 */
uint32_t PP_Get_SkinDetectThresholdHigh(void);


/**
 * Get configuration: Upper AC boundary for skin detection.
 * @return Upper skin detect AC threshold value
 */
uint32_t PP_Get_SkinDetectACThresholdLow(void);


/**
 * Get configuration: Lower AC boundary for skin detection.
 * @return Lower skin detect AC threshold value
 */
uint32_t PP_Get_SkinDetectACThresholdHigh(void);


/**
 * Get configuration: Duration of cadence wait. @see PP_Set_CadenceWait
 * @return config_sec_t
 */
uint16_t PP_Get_CadenceWait(void);


/**
 * Deprecated: Use
 * Get configuration: Sampling rate of the input signals. @see PP_Set_Fs
 * @return Sampling rate setting of the input signals
 */
config_input_fs_t PP_Get_Fs(void) __lifeq_deprecated__;


/**
 * Get configuration: G range of accelerometer. @see PP_Set_Gs
 * @return G range setting of accelerometer
 */
config_g_t PP_Get_Gs(void);


/**
 * Get configuration: Is cadence based AC tracking active. @see PP_Set_AC_Cadence_Tracking
 * @return RET_OK if successful, else hqret_t error.
 */
config_t PP_Get_AC_Cadence_Tracking(void);


/**
 * Get configuration: Is HR recovery active. @see PP_Set_Recovery
 * @return RET_OK if successful, else hqret_t error.
 */
config_t PP_Get_Recovery(void);


/**
 * Get configuration: Is peak initialisation active. @see PP_Set_Peak_Initialisation
 * @return RET_OK if successful, else hqret_t error.
 */
config_t PP_Get_Peak_Initialisation(void);


/**
 * Get configuration: Duration of heart rate smooth window. @see PP_Set_HeartRateSmoothWindow
 * @return config_sec_t
 */
uint16_t PP_Get_HeartRateSmoothWindow(void);


/**
 * Get configuration: Duration of heart rate filler window. @see PP_Set_HeartRateFillerWindow
 * @return config_sec_t
 */
uint16_t PP_Get_HeartRateFillerWindow(void);


/**
 * Get configuration: Duration of cadence filler window. @see PP_Get_CadenceFillerWindow
 * @return config_sec_t
 */
uint16_t PP_Get_CadenceFillerWindow(void);


/**
 * @return PP_Get_HeartRateFillerWindow() != PP_CONFIG_WINDOW_UNSET
 */
bool PP_Is_HeartRateFilling(void);


/**
 * @return PP_Get_CadenceFillerWindow() != PP_CONFIG_WINDOW_UNSET
 */
bool PP_Is_CadenceFilling(void);


/**
 * Get the AFE used by the current algorithm setup.
 * @return config_afe_t
 */
config_afe_t PP_Get_Device_AFE(void);


#endif /* defined(__lifeqinside__lifeq_config__) */
