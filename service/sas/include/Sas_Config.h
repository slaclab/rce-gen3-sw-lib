// -*-Mode: C;-*-
/**

This is the PUBLIC interface to the Socket Abstraction Services (SAS) facility.  
This file provides configuration level access to the SAS facility.


                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.


Facility:
SAS

Author
S. Maldonado, SLAC (smaldona@slac.stanford.edu)

Date created:
2014/06/11

Credits:
SLAC
*/

#ifndef SAS_CONFIG_H
#define SAS_CONFIG_H

/*
**
** These constants specify the SAS SVT table IDs. 
**
*/

#define SVT_SAS_TABLE_NUM           24
#define SVT_SAS_TABLE               (1 << SVT_SAS_TABLE_NUM)


/* This value defines the count of SAS interrupt sources. */

#define SAS_INTR_COUNT              64

/*
**
** These are the SAS interrupt source IDs.
**
*/

#define SAS_INTR_PPI_IBPEND_BASE    0
#define SAS_INTR_PPI0_IBPEND        0
#define SAS_INTR_PPI1_IBPEND        1
#define SAS_INTR_PPI2_IBPEND        2
#define SAS_INTR_PPI3_IBPEND        3

#define SAS_INTR_PPI_IBFULL_BASE    4
#define SAS_INTR_PPI1_IBFULL        5
#define SAS_INTR_PPI2_IBFULL        6
#define SAS_INTR_PPI3_IBFULL        7

#define SAS_INTR_PPI_OBEMPTY_BASE   8
#define SAS_INTR_PPI0_OBEMPTY       8
#define SAS_INTR_PPI1_OBEMPTY       9
#define SAS_INTR_PPI2_OBEMPTY       10
#define SAS_INTR_PPI3_OBEMPTY       11

#define SAS_INTR_PPI_OBFULL_BASE    12
#define SAS_INTR_PPI0_OBFULL        12
#define SAS_INTR_PPI1_OBFULL        13
#define SAS_INTR_PPI2_OBFULL        14
#define SAS_INTR_PPI3_OBFULL        15

#define SAS_INTR_UTIL_PEND_BASE     16
#define SAS_INTR_UTIL0_PEND         16
#define SAS_INTR_UTIL1_PEND         17
#define SAS_INTR_UTIL2_PEND         18
#define SAS_INTR_UTIL3_PEND         19

#define SAS_INTR_UTIL_EMPTY_BASE    20
#define SAS_INTR_UTIL0_EMPTY        20
#define SAS_INTR_UTIL1_EMPTY        21
#define SAS_INTR_UTIL2_EMPTY        22
#define SAS_INTR_UTIL3_EMPTY        23

#define SAS_INTR_RUNDWN_BASE        24
#define SAS_INTR_RUNDWN0_PEND       24
#define SAS_INTR_RUNDWN1_PEND       25
#define SAS_INTR_RUNDWN2_PEND       26
#define SAS_INTR_RUNDWN3_PEND       27
#define SAS_INTR_RUNDWN4_PEND       28
#define SAS_INTR_RUNDWN5_PEND       29
#define SAS_INTR_RUNDWN6_PEND       30
#define SAS_INTR_RUNDWN7_PEND       31
#define SAS_INTR_RUNDWN8_PEND       32
#define SAS_INTR_RUNDWN9_PEND       33
#define SAS_INTR_RUNDWN10_PEND      34
#define SAS_INTR_RUNDWN11_PEND      35
#define SAS_INTR_RUNDWN12_PEND      36
#define SAS_INTR_RUNDWN13_PEND      37
#define SAS_INTR_RUNDWN14_PEND      38
#define SAS_INTR_RUNDWN15_PEND      39
#define SAS_INTR_RUNDWN16_PEND      40
#define SAS_INTR_RUNDWN17_PEND      41
#define SAS_INTR_RUNDWN18_PEND      42
#define SAS_INTR_RUNDWN19_PEND      43
#define SAS_INTR_RUNDWN20_PEND      44
#define SAS_INTR_RUNDWN21_PEND      45
#define SAS_INTR_RUNDWN22_PEND      46
#define SAS_INTR_RUNDWN23_PEND      47
#define SAS_INTR_RUNDWN24_PEND      48
#define SAS_INTR_RUNDWN25_PEND      49
#define SAS_INTR_RUNDWN26_PEND      50
#define SAS_INTR_RUNDWN27_PEND      51
#define SAS_INTR_RUNDWN28_PEND      52
#define SAS_INTR_RUNDWN29_PEND      53
#define SAS_INTR_RUNDWN30_PEND      54

#define SAS_INTR_RUNDWNERR_PEND     55

#define SAS_INTR_APP_BASE           56
#define SAS_INTR_APP0               56
#define SAS_INTR_APP1               57
#define SAS_INTR_APP2               58
#define SAS_INTR_APP3               59
#define SAS_INTR_APP4               60
#define SAS_INTR_APP5               61
#define SAS_INTR_APP6               62
#define SAS_INTR_APP7               63

/*
**
** This value defines the reserved interrupt line assigned to metrics.
** The metrics irq must not be used in any other source map.
**
*/

#define SAS_IRQ_METRICS             2

/*
**
** This value defines the reserved interrupt line assigned to faults.
** The faults irq must not be used in any other source map.
**
*/

#define SAS_IRQ_FAULTS              3

/*
**
** These constants define the offsets for fields
** in a 32-bit interrupt map configuration value.
**
*/

#define SAS_INTR_MAP_INPUT_OFFSET   0x00000000
#define SAS_INTR_MAP_GROUP_OFFSET   0x00000008
#define SAS_INTR_MAP_ENABLE_OFFSET  0x00000010

#define SAS_INTR_MAP_INPUT_MASK     0x0000001F
#define SAS_INTR_MAP_GROUP_MASK     0x0000000F

/*
**
** This macro encodes an interrupt map configuration value
**
*/

#define SAS_INTR_MAP_ENABLE(g,i) \
        ((unsigned)((i & SAS_INTR_MAP_INPUT_MASK) << SAS_INTR_MAP_INPUT_OFFSET) |\
         (unsigned)((g & SAS_INTR_MAP_GROUP_MASK) << SAS_INTR_MAP_GROUP_OFFSET) |\
         0x1 << SAS_INTR_MAP_ENABLE_OFFSET)

/*
**
** This constant specifies a disabled interrupt map value.
**
*/

#define SAS_INTR_MAP_DISABLE        0x00000000

/** 

This is the socket attributes structure.

*/

typedef struct {
  uint32_t    moh;     /*!< Maximum outbound header length (in double words) */
  uint32_t    obFd;    /*!< Number of outbound frame descriptors for free-list */
  uint32_t    mib;     /*!< Maximum inbound header length (in double words) */
  uint32_t    ibFd;    /*!< Number of inbound frame descriptors for free-list */
  } SAS_Attributes;

/** 

This is the plugin preferences structure.

*/

typedef struct {
  const SAS_Attributes *attrs;   /*!< Pointer to plugin attributes */
  uint32_t              region;  /*!< Region of firmware I/O registers */
  uint32_t              offset;  /*!< Offset of firmware I/O registers */
  const char           *name;    /*!< Plugin name */
  const char           *bitfile; /*!< Plugin firmware bitfile */
  } SAS_Preferences;

/** 

This is the application fifo preferences structure.

*/

/* SEMTODO: remove workaround for missing fw app fifo interface */

typedef struct {
  uint32_t    slot;    /*!< Application fifo slot number */
  uint32_t    offset;  /*!< Application fifo axi offset */  
  const char *name;    /*!< Application fifo name */
  } SAS_AppPreferences;

#endif /* SAS_CONFIG_H */
