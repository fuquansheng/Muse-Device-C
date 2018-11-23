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
 * @file phys_calc.h
 *
 * @brief Physiological Parameter Calculation API
 * ----------------------------------------------
 * This file defines the API for the LifeQ Heart Rate and other Physiological
 * Parameter calculations. Raw data is passed in and the heart rate and other
 * parameters are estimated. These calculated values can then be requested
 * through the relevant functions.
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif


#ifndef PHYS_PARAMS_CALC_H_
#define PHYS_PARAMS_CALC_H_


#include "pp_config.h"



//If a function is deprecated it means it might be removed in the future.
// ____lifeq_deprecated__ functions have been replaced with __attribute__ ((deprecated)).
#ifndef ____lifeq_deprecated__
#define ____lifeq_deprecated__      __attribute__ ((deprecated))
#endif


//If a function has the not implemented tag it should not be called and is reserved for future implementation.
#ifndef __not_implemented__
#define __not_implemented__
#endif


//If a function has the experimental tag it is not considered ready for commercial use.
#ifndef __experimental__
#define __experimental__
#endif


/**
 * A Copyright is applicable to external parties using this LifeQ library.
 * @return  The copyright text notice and contract agreement.
 */
const char* PP_Copyright(void);


/**
 * The name of the specific heart rate algorithm used can be requested.
 * @return  Name of the algorithm used
 */
const char* PP_GetAlgName(void);


/**
 * Major changes if the interface changes.
 * @return the major version number.
 */
uint16_t PP_GetVerMajor(void);


/**
 * Minor only changes on features updates.
 * @return the minor version number.
 */
uint16_t PP_GetVerMinor(void);


/**
 * Build version number increments after every successful build.
 * @return the build version number.
 */
uint16_t PP_GetVerBuild(void);

/**
 * Hotfix version number increments for patches to a stable build out in the field.
 * @return the hotfix version number.
 */
uint16_t PP_GetVerHotFix(void);


/**
 * Reset the internal state of the algorithm.
 * Note: PP_InitLifeQ should be called for a new user if available.
 */
hqret_t PP_Reset(void);


/**
 * Initialize data structures, allocate memory, etc.
 * Must be called before any of the other algorithms are called.

 * @see PP_Set_Defaults(); sets the fs input also.
 * Call PP_Set_Defaults() first and PP_Initialize() second.

 */
hqret_t PP_Initialize(void);


// Deprecated, @see PP_Initialize. Only 25Hz data is accepted.
// Reason: variable fs not supported on init,
//         instead resample data streams to 25Hz and use as input.
hqret_t PP_Init(config_input_fs_t fs) ;



/**
 * Algorithm type enum.
 */
typedef enum {
    PP_HEART_RATE  = 0,   // code for heart rate algorithm.
    PP_HRV         = 1,   // code for Heart rate variability algorithm.
    PP_SPO2        = 2,   // code for SPO2 algorithm.
    PP_CALIBRATION = 3,   // code for calibration irrespective of the algorithm in use (while initializing, to not feed unwanted data to algorithms).
    PP_HEART_RATE_AND_HRV = 4, // code for running heart rate variability and heart rate algorithms in parallel.
    PP_SLEEP       = 5,   // code for sleep algorithm, includes heart rate variability and heart rate algorithms in parallel.
    PP_STRESS = 6,         __not_implemented__
    PP_RR         = 7,    // only runs RR algorithm on inputs.
    PP_SKIN_DETECT = 50,  // all algorithms are switched off and input samples are evaluated for on-skin.

    PP_CALC_ALL    = 99,  // run all available algorithms. Under development.
} alg_input_selection_t;



typedef enum {
    LQ_LED_NONE     = 100,
    LQ_LED_GREEN    = 0,
    LQ_LED_RED      = 1,
    LQ_LED_INFRARED = 2,
    LQ_LED_AMBIENT  = 3,
} lq_led_t;        /// Wave length of LED.



typedef struct input_sample {
    uint32_t  sample;                 ///< Sample value.
    uint32_t  rf_kohm;                ///< Channel gain as kohm value.
    int16_t   isub;                   ///< only TI4404 & TI4405 otherwise 0;
    uint16_t  led_curr_mAx10;         ///< Input current in mA x 10.
    uint8_t   count;                  ///< Number of samples accumulated to get sample.
    uint8_t   num_amp;                ///< Number of trans impedance amplifiers being used.
    uint8_t   led;                    ///< Sample wavelength description.
} led_sample_t;


/**
 * Use this function when you require high performance real-time exercise results.
 * Adding raw input samples from TI AFE 4404, expects data samples at set frequency @see PP_Init and @see PP_Set_Fs.
 * The accelerometer G's sensitivity setting is optimized for +-6g and +-8g @see PP_Set_Gs.
 * Set unavailable/unused parameters to 0.
 *
 * @param accel1    Accelerometer channel 1 as uint16_t range with DC offset 32768.
 * @param accel2    Accelerometer channel 2 as uint16_t range with DC offset 32768.
 * @param accel3    Accelerometer channel 3 as uint16_t range with DC offset 32768.
 *
 * @param ch1   Data source from channel 1 ex. Green.
 * @param ch2   Data source from channel 2 ex. Ambient.
 * @param ch3   Data source from channel 3 ex. Red.
 * @param ch4   Data source from channel 4 ex. Infrared.
 *
 * @param inputEnum Type of algorithm to call @see (alg_input_selection_t).
 *
 * @return error code when values out of range.
 */

hqret_t PP_addSamplesTI4404(const int32_t accel1,
                            const int32_t accel2,
                            const int32_t accel3,
                            led_sample_t *ch1,
                            led_sample_t *ch2,
                            led_sample_t *ch3,
                            led_sample_t *ch4,
                            const alg_input_selection_t inputEnum);


/**
 * Use this function when you require high performance real-time exercise results.
 * Adding raw input samples from TI AFE 4404, expects data samples at set frequency @see PP_Init and @see PP_Set_Fs.
 * The accelerometer G's sensitivity setting is optimized for +-6g and +-8g
 * Set unavailable/unused parameters to 0.
 *
 * @param accel1    Accelerometer channel 1 as float in G's.
 * @param accel2    Accelerometer channel 2 as float in G's.
 * @param accel3    Accelerometer channel 3 as float in G's.
 *
 * @param ch1   Data source from channel 1 ex. Green.
 * @param ch2   Data source from channel 2 ex. Ambient.
 * @param ch3   Data source from channel 3 ex. Red.
 * @param ch4   Data source from channel 4 ex. Infrared.
 *
 * @param inputEnum Type of algorithm to call @see (alg_input_selection_t).
 *
 * @return error code when values out of range.
 */
hqret_t PP_addSamplesAccG_TI4404(const float accel1,
                                 const float accel2,
                                 const float accel3,
                                 led_sample_t *ch1,
                                 led_sample_t *ch2,
                                 led_sample_t *ch3,
                                 led_sample_t *ch4,
                                 const alg_input_selection_t inputEnum);
								 

/**
 * Use this function when you require high performance real-time exercise results.
 * Adding raw input samples from TI AFE 4405, expects data samples at set frequency @see PP_Init and @see PP_Set_Fs.
 * The accelerometer G's sensitivity setting is optimized for +-6g and +-8g @see PP_Set_Gs.
 * Set unavailable/unused parameters to 0.
 *
 * @param accel1    Accelerometer channel 1 as int16_t range with DC offset of 0.
 * @param accel2    Accelerometer channel 2 as int16_t range with DC offset of 0.
 * @param accel3    Accelerometer channel 3 as int16_t range with DC offset of 0.
 *
 * @param gr_sample   Data sample for green led.
 * @param am_sample   Data sample for ambient led.
 * @param rd_sample   Data sample for red led.
 * @param ir_sample   Data sample for infrared led.
 *
 * @param ledCur1   LED current 1 - TX1 (corresponding to green) as uint16_t range ( mA x 10 )
 * @param ledCur2   LED current 2 - TX2 (corresponding to red) as uint16_t range ( mA x 10 )
 * @param ledCur3   LED current 3 - TX3 (corresponding to infrared) as uint16_t range ( mA x 10 )
 *
 * @param is_signal_afe_adjustment True for AGC changes occurred on this input sample, else false.
 * @param inputEnum Type of algorithm to call @see (alg_input_selection_t).
 *
 * @return error code when values out of range.
 */

hqret_t PP_addSamplesTI4405(int16_t accel1,
                            int16_t accel2,
                            int16_t accel3,

                            int32_t gr_sample,
                            int32_t am_sample,
                            int32_t rd_sample,
                            int32_t ir_sample,

							uint16_t ledCur1,
							uint16_t ledCur2,
							uint16_t ledCur3,

                            bool is_signal_afe_adjustment,

                            alg_input_selection_t inputEnum) __experimental__;

/**
 * Use this function when you require high performance real-time exercise results.
 * Adding raw input samples from TI AFE 4405, expects data samples at set frequency @see PP_Init and @see PP_Set_Fs.
 * The accelerometer G's sensitivity setting is optimized for +-6g and +-8g @see PP_Set_Gs.
 * Set unavailable/unused parameters to 0.
 *
 * @param accel1    Accelerometer channel 1 as float in G's.
 * @param accel2    Accelerometer channel 2 as float in G's.
 * @param accel3    Accelerometer channel 3 as float in G's.
 *
 * @param gr_sample   Data sample for green led.
 * @param am_sample   Data sample for ambient led.
 * @param rd_sample   Data sample for red led.
 * @param ir_sample   Data sample for infrared led.
 *
 * @param ledCur1   LED current 1 - TX1 (corresponding to green) as uint16_t range ( mA x 10 )
 * @param ledCur2   LED current 2 - TX2 (corresponding to red) as uint16_t range ( mA x 10 )
 * @param ledCur3   LED current 3 - TX3 (corresponding to infrared) as uint16_t range ( mA x 10 )
 *
 * @param is_signal_afe_adjustment True for AGC changes occurred on this input sample, else false.
 * @param inputEnum Type of algorithm to call @see (alg_input_selection_t).
 *
 * @return error code when values out of range.
 */

hqret_t PP_addSamplesAccG_TI4405(const float accel1,
                                 const float accel2,
                                 const float accel3,

                                 int32_t gr_sample,
                                 int32_t am_sample,
                                 int32_t rd_sample,
                                 int32_t ir_sample,

                                 uint16_t ledCur1,
                                 uint16_t ledCur2,
                                 uint16_t ledCur3,

                                 bool is_signal_afe_adjustment,

                                 alg_input_selection_t inputEnum) __experimental__;

/**
 * pp results struct.
 */
typedef struct pp_results {
    
    // -------------------------------------------------------------------------
    // alg_input_selection_t.PP_HEART_RATE
    // -------------------------------------------------------------------------
    uint8_t  lq_hr;         ///< Heart rate (bpm).
    uint8_t  lq_cadence;    ///< Cadence  (rpm).
    int8_t   hr_confidence; ///< @see PP_GetHRConfidence().
    
    // -------------------------------------------------------------------------
    // lifeq level 1 metrics
    // -------------------------------------------------------------------------
    uint8_t  lq_score __not_implemented__;
    uint16_t lq_vo2; 	   ///< Oxyen consumption (l/min*100).
    uint16_t lq_epoc;	   ///< (ml/kg*10) @PP_GetLifeQ_EPOC().
    uint16_t lq_lactate;   ///< Blood lactate level (mmol/l*100).
    uint16_t lq_calories;  ///< Calories burnt (kCal).
    uint16_t lq_MaxHR;     ///< max hr (bpm).
    // -------------------------------------------------------------------------
} pp_results_t;
   
    
/**
 * pp samples struct.
 */
typedef struct pp_samples {
    int32_t accel1;
    int32_t accel2;
    int32_t accel3;
    led_sample_t ch_green;
    led_sample_t ch_ambient;
    led_sample_t ch_red;
    led_sample_t ch_infrared;
} pp_samples_t;


/**
 * Use this function when you require batch processing of samples to save power.
 * Adding raw input samples from TI AFE 4404.
 *
 * @param samples    pp_samples_t[input_arr_size].
 * @param results    pp_results_t[input_arr_size].
 * @param input_arr_size    array size input.
 * @param inputEnum Type of algorithm to call @see (alg_input_selection_t).
 *
 * @return error code when values out of range.
 */
hqret_t PP_addSamplesTI4404_batch(pp_samples_t samples[],
                                  pp_results_t results[],
                                  uint32_t input_arr_size,
                                  const alg_input_selection_t inputEnum) __experimental__;


typedef enum {
    PP_MOV_STANDING  = 0,
    PP_MOV_SITTING   = 1,
    PP_MOV_SLEEPING  = 2,
	PP_MOV_HRV_POINT_TEST = 3,
} mov_t;


/**
 * Determines if the person is moving from the G normalized accelerometer.
 * @return True if moving else false.
 */
bool PP_IsMoving(mov_t state);


/**
 * Get Heart Rate Confidence Metric as calculated from green input on PP_addSamples(...)
 * @return value between 0 and 100.
 * Note: 0 < value < 20  - no confidence
 *      20 < value < 40  - moderate confidence
 *      40 < value < 100 - high confidence
 *      127 Algorithm disabled.
 */
int8_t PP_GetHRConfidence(void);


    
/**
 * Determine whether or not sensor contact on skin is detected when PP_Set_SkinDetect(ON) is enabled.
 * Immediate feedback is provided without delays (see PP_IsSensorContactDetectedWithFiller).
 * @return true if on sensor on skin detected, else false.
 * Note: Functions independently of the heart rate filler window @see PP_Set_HeartRateFillerWindow
 * an off skin event will be reported immediately.
 *              Note: Also returns true if CONFIG_WEAR_DEVICE_ON has been set and
                      when PP_Set_SkinDetect(OFF) is used.
 */
bool PP_IsSensorContactDetected(void);


/**
 * Determine if device is on skin or not when PP_Set_SkinDetect(ON) is enabled, 
   see (PP_Set_HeartRateFillerWindow). Overridden by PP_Set_IsDeviceOnUser(...).
 * @return true if on skin detected and during filler window period, else false.
 *              Note: Also returns true if CONFIG_WEAR_DEVICE_ON has been set.
 */
bool PP_IsSensorContactDetectedWithFiller(void);


#endif /* defined(PHYS_PARAMS_CALC_H_) */

#ifdef __cplusplus
}
#endif
