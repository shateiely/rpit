/*
 * File: jitter_private.h
 *
 * Real-Time Workshop code generated for Simulink model jitter.
 *
 * Model version                        : 1.11
 * Real-Time Workshop file version      : 7.6.2  (R2010bSP2)  02-Mar-2012
 * Real-Time Workshop file generated on : Fri Apr 22 17:18:52 2016
 * TLC version                          : 7.6 (Jul 13 2010)
 * C/C++ source code generated on       : Fri Apr 22 17:18:52 2016
 *
 * Target selection: ert_rpi.tlc
 * Embedded hardware selection: ARM Compatible->ARM 11
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_jitter_private_h_
#define RTW_HEADER_jitter_private_h_
#include "rtwtypes.h"

/* Private macros used by the generated code to access rtModel */
#ifndef rtmSetTFinal
# define rtmSetTFinal(rtm, val)        ((rtm)->Timing.tFinal = (val))
#endif

#ifndef rtmGetTPtr
# define rtmGetTPtr(rtm)               (&(rtm)->Timing.taskTime0)
#endif

#ifndef __RTWTYPES_H__
#error This file requires rtwtypes.h to be included
#else
#ifdef TMWTYPES_PREVIOUSLY_INCLUDED
#error This file requires rtwtypes.h to be included before tmwtypes.h
#else

/* Check for inclusion of an incorrect version of rtwtypes.h */
#ifndef RTWTYPES_ID_C08S16I32L32N32F1
#error This code was generated with a different "rtwtypes.h" than the file included
#endif                                 /* RTWTYPES_ID_C08S16I32L32N32F1 */
#endif                                 /* TMWTYPES_PREVIOUSLY_INCLUDED */
#endif                                 /* __RTWTYPES_H__ */

extern void rpi_sfun_time_Outputs_wrapper(real_T *y0,
  const real_T *rpi_Ts, const int_T p_width0);

#endif                                 /* RTW_HEADER_jitter_private_h_ */

/*
 * File trailer for Real-Time Workshop generated code.
 *
 * [EOF]
 */