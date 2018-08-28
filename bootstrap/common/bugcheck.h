// -*-Mode: gas;-*-
/*!@file    bugcheck.h
*
*  @brief   Macro used to bugcheck the system
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    November 9, 2011 -- Created
*
*  $Revision: 1358 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/
#ifndef _BUGCHECK_H_
#define _BUGCHECK_H_

        .extern bugcheck

        // Handle bugchecks
        .macro  BugChk cond=bl,cr=cr0
        // @brief A macro for bugchecking the system based on condition codes
        //        This macro calls the bugcheck function when the condition
        //        provided in the first argument is true.  The bugcheck
        //        function doesn't return when called.
        //        Note that the bcl and bcla instructions (and their extended
        //        mnemonics) update the link register regardless of whether
        //        the branch is taken, so the caller must take care to preserve
        //        it for the case when the branch is not taken.
        // @param cond - One of the Branch Conditional mnemonics bcl, bcla
        // @param cr   - One of the 8 Condition Register fields cr0-cr7
        \cond   \cr,bugcheck      // Branch to bugchecking code
        .endm

#endif
