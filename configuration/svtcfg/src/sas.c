// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definitions and prototypes for the SAS symbol value table.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility: SAS
configuration

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/06/29

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC

*/

#include <stdint.h>
#include <string.h>

#include "memory/mem.h"
#include "sas/Sas.h"
#include "sas/Sas_Config.h"

/*
** ++
**
** SAS socket plugin attributes.
**
** Structures for configuring socket plugin attributes.
**
** Each structure represents the SAS_Attributes used to
** configure the memory resources available to a plugin
** and to bind the plugin name and bitfile to the socket..
**
** --
*/

const char PPI0_NAME[]    = "Pgp0";
const char PPI1_NAME[]    = "Pgp1";
const char PPI2_NAME[]    = "Pgp2";
const char PPI3_NAME[]    = "Xaui";

/* SEMTODO: remove workaround for fw partial reconfiguration */

const char PPI0_BITFILE[] = "0";
const char PPI1_BITFILE[] = "1";
const char PPI2_BITFILE[] = "2";
const char PPI3_BITFILE[] = "3";

/* Example PPI0 socket attributes */

SAS_Attributes const PPI0_ATTRS = {
  .moh     = 32,
  .obFd    = 42,
  .mib     = 32,
  .ibFd    = 42,
  };

/* Example PPI1 socket attributes */

SAS_Attributes const PPI1_ATTRS = {
  .moh     = 32,
  .obFd    = 42,
  .mib     = 32,
  .ibFd    = 42,
  };

/* Example PPI2 socket attributes */

SAS_Attributes const PPI2_ATTRS = {
  .moh     = 32,
  .obFd    = 42,
  .mib     = 32,
  .ibFd    = 42,
  };

/* Example PPI3 socket attributes */

SAS_Attributes const PPI3_ATTRS = {
  .moh     = 16,
  .obFd    = 504,
  .mib     = 16,
  .ibFd    = 504,
  };

/* Example PPI0 plugin preferences */

SAS_Preferences const PPI0_PREFS = {
  .attrs   = &PPI0_ATTRS,
  .region  =  MEM_REGION_USERDEV,
  .offset  =  0,
  .name    =  PPI0_NAME,
  .bitfile =  PPI0_BITFILE
  };

/* Example PPI1 plugin preferences */

SAS_Preferences const PPI1_PREFS = {
  .attrs   = &PPI1_ATTRS,
  .region  =  MEM_REGION_USERDEV,
  .offset  =  0x10000,
  .name    =  PPI1_NAME,
  .bitfile =  PPI1_BITFILE
  };

/* Example PPI2 plugin preferences */

SAS_Preferences const PPI2_PREFS = {
  .attrs   = &PPI2_ATTRS,
  .region  =  MEM_REGION_USERDEV,
  .offset  =  0x20000,
  .name    =  PPI2_NAME,
  .bitfile =  PPI2_BITFILE
  };

/* Example PPI3 plugin preferences */

SAS_Preferences const PPI3_PREFS = {
  .attrs   = &PPI3_ATTRS,
  .region  =  MEM_REGION_USERPPI,
  .offset  =  0,
  .name    =  PPI3_NAME,
  .bitfile =  PPI3_BITFILE
  };

/*
** ++
**
** SAS plugin configuration preferences.
**
** Structure for enabling plugins.
**
** Each entry corresponds to a plugin socket attributes pointer.
** This table supports 4 plugins (0-3).
**
** --
*/
    
SAS_Preferences const *SAS_PLUGIN_PREFS[] = {
  &PPI0_PREFS,
  &PPI1_PREFS,
  &PPI2_PREFS,
  &PPI3_PREFS,
  NULL
  };

/*
** ++
**
** SAS Application fifo preferences.
**
** Structures for binding names to application fifo slots.
**
** Each structure represents the SAS_AppAttributes used to
** configure the application specific fifo.
**
** --
*/

const char APP0_NAME[] = "app0";
const char APP1_NAME[] = "app1";

/* Example application fifo 0 attributes */
/* SEMTODO: remove workaround for missing fw app fifo interface */
SAS_AppPreferences const APP0_PREFS = {
  .slot    = 0,
  .offset  = 0x10080108,
  .name    = APP0_NAME,
  };

/* Example application fifo 1 attributes */
/* SEMTODO: remove workaround for missing fw app fifo interface */
SAS_AppPreferences const APP1_PREFS = {
  .slot    = 1,
  .offset  = 0x1008010C,
  .name    = APP1_NAME,
  };

/*
** ++
**
** SAS application fifo configuration preferences.
**
** Structure for enabling application specific fifos.
**
** Each entry corresponds to an application attributes pointer. 
** This table supports up to 8 application fifos (0-7).
**
** --
*/
  
SAS_AppPreferences const *SAS_APP_PREFS[] = {
//&APP0_PREFS,
//&APP1_PREFS,
  NULL
  };

/*
** ++
**
** SAS Interrupt configuration table.
**
** Structure for binding interrupt sources to 
** group source inputs.
**
** An interrupt group corresponds to a single
** interrupt IRQ line.
**
** Each entry index in the table corresponds 
** to the interrupt source ID.
**
** Each entry in the table specifies the interrupt 
** group and source input to which it is assigned.
**
** --
*/

const unsigned SAS_INTR_MAP[SAS_INTR_COUNT] = {
/* SAS_INTR_MAP_ENABLE(group,input)         // interrupt IRQ */
  SAS_INTR_MAP_ENABLE(0,0),                 // SAS_INTR_PPI0_IBPEND      0
  SAS_INTR_MAP_ENABLE(0,1),                 // SAS_INTR_PPI1_IBPEND      1
  SAS_INTR_MAP_ENABLE(0,2),                 // SAS_INTR_PPI2_IBPEND      2
  SAS_INTR_MAP_ENABLE(0,3),                 // SAS_INTR_PPI3_IBPEND      3
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,0),   // SAS_INTR_PPI0_IBFULL      4
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,3),   // SAS_INTR_PPI1_IBFULL      5
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,6),   // SAS_INTR_PPI2_IBFULL      6
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,9),   // SAS_INTR_PPI3_IBFULL      7
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,1),   // SAS_INTR_PPI0_OBEMPTY     8
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,4),   // SAS_INTR_PPI1_OBEMPTY     9
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,7),   // SAS_INTR_PPI2_OBEMPTY     10
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,9),   // SAS_INTR_PPI3_OBEMPTY     11
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,2),   // SAS_INTR_PPI0_OBFULL      12
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,5),   // SAS_INTR_PPI1_OBFULL      13
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,8),   // SAS_INTR_PPI2_OBFULL      14
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,11),  // SAS_INTR_PPI3_OBFULL      15
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL0_PEND       16
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL1_PEND       17
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL2_PEND       18
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL3_PEND       19
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,16),  // SAS_INTR_UTIL0_EMPTY      20
  SAS_INTR_MAP_ENABLE(SAS_IRQ_METRICS,17),  // SAS_INTR_UTIL1_EMPTY      21
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL2_EMPTY      22
  SAS_INTR_MAP_DISABLE,                     // SAS_INTR_UTIL3_EMPTY      23
  SAS_INTR_MAP_ENABLE(5,0),                 // SAS_INTR_RUNDWN0_PEND     24
  SAS_INTR_MAP_ENABLE(5,1),                 // SAS_INTR_RUNDWN1_PEND     25
  SAS_INTR_MAP_ENABLE(5,2),                 // SAS_INTR_RUNDWN2_PEND     26
  SAS_INTR_MAP_ENABLE(5,3),                 // SAS_INTR_RUNDWN3_PEND     27
  SAS_INTR_MAP_ENABLE(5,4),                 // SAS_INTR_RUNDWN4_PEND     28
  SAS_INTR_MAP_ENABLE(5,5),                 // SAS_INTR_RUNDWN5_PEND     29
  SAS_INTR_MAP_ENABLE(5,6),                 // SAS_INTR_RUNDWN6_PEND     30
  SAS_INTR_MAP_ENABLE(5,7),                 // SAS_INTR_RUNDWN7_PEND     31
  SAS_INTR_MAP_ENABLE(5,8),                 // SAS_INTR_RUNDWN8_PEND     32
  SAS_INTR_MAP_ENABLE(5,9),                 // SAS_INTR_RUNDWN9_PEND     33
  SAS_INTR_MAP_ENABLE(5,10),                // SAS_INTR_RUNDWN10_PEND    34
  SAS_INTR_MAP_ENABLE(5,11),                // SAS_INTR_RUNDWN11_PEND    35
  SAS_INTR_MAP_ENABLE(5,12),                // SAS_INTR_RUNDWN12_PEND    36
  SAS_INTR_MAP_ENABLE(5,13),                // SAS_INTR_RUNDWN13_PEND    37
  SAS_INTR_MAP_ENABLE(5,14),                // SAS_INTR_RUNDWN14_PEND    38
  SAS_INTR_MAP_ENABLE(5,15),                // SAS_INTR_RUNDWN15_PEND    39
  SAS_INTR_MAP_ENABLE(5,16),                // SAS_INTR_RUNDWN16_PEND    40
  SAS_INTR_MAP_ENABLE(5,17),                // SAS_INTR_RUNDWN17_PEND    41
  SAS_INTR_MAP_ENABLE(5,18),                // SAS_INTR_RUNDWN18_PEND    42
  SAS_INTR_MAP_ENABLE(5,19),                // SAS_INTR_RUNDWN19_PEND    43
  SAS_INTR_MAP_ENABLE(5,20),                // SAS_INTR_RUNDWN20_PEND    44
  SAS_INTR_MAP_ENABLE(5,21),                // SAS_INTR_RUNDWN21_PEND    45
  SAS_INTR_MAP_ENABLE(5,22),                // SAS_INTR_RUNDWN22_PEND    46
  SAS_INTR_MAP_ENABLE(5,23),                // SAS_INTR_RUNDWN23_PEND    47
  SAS_INTR_MAP_ENABLE(5,24),                // SAS_INTR_RUNDWN24_PEND    48
  SAS_INTR_MAP_ENABLE(5,25),                // SAS_INTR_RUNDWN25_PEND    49
  SAS_INTR_MAP_ENABLE(5,26),                // SAS_INTR_RUNDWN26_PEND    50
  SAS_INTR_MAP_ENABLE(5,27),                // SAS_INTR_RUNDWN27_PEND    51
  SAS_INTR_MAP_ENABLE(5,28),                // SAS_INTR_RUNDWN28_PEND    52
  SAS_INTR_MAP_ENABLE(5,29),                // SAS_INTR_RUNDWN29_PEND    53
  SAS_INTR_MAP_ENABLE(5,30),                // SAS_INTR_RUNDWN30_PEND    54
  SAS_INTR_MAP_ENABLE(SAS_IRQ_FAULTS,0),    // SAS_INTR_RUNDWNERR_PEND   55 
  SAS_INTR_MAP_ENABLE(0,4),                 // SAS_INTR_APP0             56
  SAS_INTR_MAP_ENABLE(0,5),                 // SAS_INTR_APP1             57
  SAS_INTR_MAP_ENABLE(0,6),                 // SAS_INTR_APP2             58
  SAS_INTR_MAP_ENABLE(0,7),                 // SAS_INTR_APP3             59
  SAS_INTR_MAP_ENABLE(0,8),                 // SAS_INTR_APP4             60
  SAS_INTR_MAP_ENABLE(0,9),                 // SAS_INTR_APP5             61
  SAS_INTR_MAP_ENABLE(0,10),                // SAS_INTR_APP6             62
  SAS_INTR_MAP_ENABLE(0,11),                // SAS_INTR_APP7             63
  };    

