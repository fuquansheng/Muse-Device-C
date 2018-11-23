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
 * @file level1_metrics.h
 *
 * @brief Level 1 metrics API is used to calculated the 
 * 1Hz metrics: Calories, VO2, est Blood Latate and EPOC, 
 * and provides an interface for calculating VO2 Max and 
 * heart rate stabilization.
 * NOTE: User profile information is required for level1 metrics.
 * ----------------------------------------------
 * This file defines the API for the LifeQ level 1 metrics.
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif


#ifndef LEVEL1_METRICS_H_
#define LEVEL1_METRICS_H_

#include <stdbool.h>
#include <stdint.h>
#include "hqerror.h"
#include "pp_config.h"

#ifndef __lifeq_deprecated__
    #if !defined(EMBEDDED) && !defined(__MINGW_ATTRIB_DEPRECATED)
        #define __lifeq_deprecated__      __attribute__ ((deprecated))

    #else
        #define __lifeq_deprecated__
    #endif
#endif


/**
 * Gender type enum.
 */
typedef enum {
    PP_GENDER_UNDEFINED     = 0,    /// This will return an error state on PP_InitLifeQ
    PP_GENDER_MALE          = 1,
    PP_GENDER_FEMALE        = 2,
    PP_GENDER_OTHER         = 3,    /// This will default to female calculations.
} gender_t;


/**
 * LifeQ user profile enum.
 */
typedef enum {
    LQ_AGE,                 /// AGE [0, 110] years
    LQ_WEIGHT,              /// WEIGHT [100, 50 000] in kg*100
    LQ_HEIGHT,              /// HEIGHT [2000, 25 000] in cm*100
    LQ_HEART_RATE_REST,     /// HR REST [15, 240] in bpm, (HR REST < HR MAX)
    LQ_HEART_RATE_MAX,      /// HR MAX [15, 240] in bpm, (HR REST < HR MAX)
    LQ_VO2_MAX,             /// VO2MAX [0, 1500] in (L/min)*100
    LQ_BODY_FAT,            /// BF [0, 50] in %
    LQ_GENDER,              /// (@see gender_t)
} lq_profile_t;

/**
 * LifeQ user profile struct.
 */
typedef struct LQ_User {
    uint16_t age_years;
    uint16_t height_centimeter;
    uint16_t hr_rest_bpm;
    uint16_t hr_max_bpm;
    uint16_t vo2min_liter_pm;
    uint16_t vo2max_liter_pm;
    float bf_percentage;
    float weight_kg;
    gender_t gender;
} lq_user_t;

    
#ifndef PP_STUB_HR_ALGORITHM
/**
 * Validate LifeQ user profile inputs as used in @see PP_InitLifeQ.
 * @return RET_OK if successful.
 */
    hqret_t LQ_Validate(lq_profile_t property, int32_t val);
#endif

    
/**
 * Update LifeQ with new data at a rate of 1Hz (once per second).
 * Can be called after user profile is set up with PP_InitLifeQ.
 * @param hr        -- Heart rate in beats per minute (bpm). Accepted range [15,... ,240].
 * @param activity  -- Activity measure, 1 if excersing else 0.
 * @return RET_OK if successful.
 */
    hqret_t LQ_Update_Metrics(int32_t hr, uint8_t activity);
   
    
/**
 * LifeQ derived metrics initialize user profile.
 * Can call LQ_Init before or after PP_Init(...).
 * Note: Only do this once per session.
 * @param user  -- user profile values. 
 * @return RET_SUCC or lqret_t ERR_
 */
    hqret_t LQ_Init(lq_user_t user);
    

/**
 * Reset internal parameters.
 */
    void LQ_Reset(void);


/**
* LifeQ metric: Heart Rate (bpm)
* Available after PP_InitLifeQ(...) used and updated after calls to PP_add(Afe)Samples(...).
* @return A value between 15 and 240 (beats per minute). If not initialized will always return 0.
*         If hardware other that LifeQ hardware is used, a once of calibration can be done to
*         detect when the device is removed from skin @see PP_Set_SkinDetect. In this case a
*         heart rate of 0 is also reported while device is not on skin.
*
*         Type changed to uint16_t for java mappings to function properly.
*/
uint16_t PP_Get_HR(void);
uint16_t PP_GetLifeQ_HR(void);

/**
 * HRV metrics struct.
 */
typedef struct hrv_metrics {
        uint32_t very_low_fr;     ///< Very-low frequency components. unit=Rad/sec.
        uint32_t low_fr;          ///< Low frequency components.  unit=Rad/sec.
        uint32_t high_fr;         ///< High frequency components. unit=Rad/sec.
        uint32_t sdnn;            ///< standard deviation of RR intervals. unit=time in sec (scaled)
        uint32_t sdsd;            ///< Standard deviation of successive differences. unit=time in sec (scaled)
        uint32_t rmssd;           ///< Root mean square of successful differences. unit=time in sec (scaled)
        uint32_t pnn50;           ///< Proportion of successful differences > 50. (scaled)
        uint32_t hrv_score;       ///< Derived from RMSSD. (scaled)
} hrv_metrics_t;


/**
 * HRV calculation state enum.
 */
typedef enum {
    PP_HRV_BUSY  = 0,       /// Deperecated use HRV_BUSY
    PP_HRV_READY = 1,       /// Deperecated use HRV_READY
    PP_HRV_FAILED = 2,      /// Deperecated use HRV_FAILED

    HRV_BUSY  = 0,
    HRV_READY = 1,
    HRV_FAILED = 2,
} hrv_state_t;


/**
 * @brief Initialisation function for HRV algorithm (should be run beforehand).
 * @param fs -- sampling frequency (Hz)
 */
hqret_t PP_HRVstartProcess (config_input_fs_t fs_t);


/**
 * @brief Get flag that signifies if last RR interval has been updated.
 * @return 1 if new RR interval update, otherwise 0.
 */
uint8_t PP_HRVready (void);


/**
 * @brief Get the previously detected RR interval and its outlier flag.
 * @param *rr -- pointer to last RR interval
 * @return Outlier check, 0 if good RR interval and 1 if outlier
 */
uint8_t PP_HRVGetLastRR (uint16_t *rr);


/**
 * @brief Once RR array filled, this functions calculates the 8 HRV metrics @see(hrv_metrics_t).
 *
 * @param *output -- pointer to array in which 8 output metrics will be stored
 * @return The factor by which values in output array are scaled for float to uint32_t conversion.
 */
uint16_t PP_HRVcalcMetrics (hrv_metrics_t *output);


/**
 * @brief Get the confidence metric for RR intervals.
 *
 * NOTE that this is based on the accelerometer and SNR and will currently
 * only return a 0 if any motion is present (or the signal quality is bad)
 * and 1 if a sufficient level of stillness is attained.
 *
 * @return 1 if confidence for accurate RR intervals is high, otherwise 0.
 ****************************************************************************/
uint8_t HRV_getRRConfidence (void);

    
#endif /* defined(PHYS_PARAMS_CALC_H_) */

#ifdef __cplusplus
}
#endif
