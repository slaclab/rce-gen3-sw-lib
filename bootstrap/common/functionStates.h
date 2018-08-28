// -*-Mode: gas;-*-
/*!@file   functionStates.h
*
* @brief   Definition of function states
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    November 29, 2011 -- Created
*
* $Revision: 1358 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _FUNCTION_STATES_H_
#define _FUNCTION_STATES_H_

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, display.h)

/* State values, containing a function identifier and a state number,
 * are meant to fit in an immediate value, i.e. 16 bits. The values
 * signify the various states in a function and may be used to show
 * progress through the function by updating an external display, for
 * example.  The values in and of themselves have no meaning and so can
 * be changed to whatever is convenient.  Similarly, more functions and
 * states may be defined when desired.
 */

/* Function number */
#define FN_0   0
#define FN_1   (FN_0  + 1)
#define FN_2   (FN_1  + 1)
#define FN_3   (FN_2  + 1)
#define FN_4   (FN_3  + 1)
#define FN_5   (FN_4  + 1)
#define FN_6   (FN_5  + 1)
#define FN_7   (FN_6  + 1)
#define FN_8   (FN_7  + 1)
#define FN_9   (FN_8  + 1)
#define FN_10  (FN_9  + 1)
#define FN_11  (FN_10 + 1)
#define FN_12  (FN_11 + 1)
#define FN_13  (FN_12 + 1)
#define FN_14  (FN_13 + 1)
#define FN_15  (FN_14 + 1)
#define FN_16  (FN_15 + 1)

/* State number */
#define FS_0   0
#define FS_1   (FS_0  + 1)
#define FS_2   (FS_1  + 1)
#define FS_3   (FS_2  + 1)
#define FS_4   (FS_3  + 1)
#define FS_5   (FS_4  + 1)
#define FS_6   (FS_5  + 1)
#define FS_7   (FS_6  + 1)
#define FS_8   (FS_7  + 1)
#define FS_9   (FS_8  + 1)
#define FS_10  (FS_9  + 1)
#define FS_11  (FS_10 + 1)
#define FS_12  (FS_11 + 1)
#define FS_13  (FS_12 + 1)
#define FS_14  (FS_13 + 1)
#define FS_15  (FS_14 + 1)
#define FS_16  (FS_15 + 1)
#define FS_17  (FS_16 + 1)
#define FS_18  (FS_17 + 1)
#define FS_19  (FS_18 + 1)
#define FS_20  (FS_19 + 1)
#define FS_21  (FS_20 + 1)
#define FS_22  (FS_21 + 1)
#define FS_23  (FS_22 + 1)
#define FS_24  (FS_23 + 1)
#define FS_25  (FS_24 + 1)
#define FS_26  (FS_25 + 1)
#define FS_27  (FS_26 + 1)
#define FS_28  (FS_27 + 1)
#define FS_29  (FS_28 + 1)
#define FS_30  (FS_29 + 1)
#define FS_31  (FS_30 + 1)

/* The following macro defines how function states are declared.  The function
 * number and state value are combined to form a 16 bit immediate value that
 * is meant to uniquely identify a location in a given program.  Currently
 * the upper nibble gives the function number and the lower 3 nibbles give
 * the state number.
 * This value is used to update an external display so that it shows the
 * last location where the program was executing prior to getting stuck (hung,
 * crashed, or bugchecked).  How the display displays the state value is left
 * to the display code and subsystem.
 */
#if !tgt_os_eabi
        //! Declare a function state
        .macro  State fn:req,value:req,reg=r30
        li      \reg,((\fn<<12)+\value)
        SetDpy  \reg
        .endm
#else   // Some syntactically correct nonesense that allows things to compile
#define State   inslwi r30,r30,
#endif
#endif
