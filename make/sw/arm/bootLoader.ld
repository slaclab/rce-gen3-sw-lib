/*
 * Facility:     DAT
 * File:         bootLoader.ld
 * Abstract:     Link script for generating a bootable image
 * Author:       R. Claus <claus@slac.stanford.edu>
 * Created:      1/30/2012
 * Committed:  \$Date: 2012-04-29 12:21:29 -0700 (Sun, 29 Apr 2012) $ by \$Author: claus $
 * Revision:   \$Revision: 884 $
 * Repository: \$HeadURL: file:///afs/slac/g/cci/repositories/ctkrepo/workspace/trunk/make/sw/bootLoader.ld $
 * Credits:      SLAC
 *                              Copyright 2011
 *                                    by
 *                       The Board of Trustees of the
 *                    Leland Stanford Junior University.
 *                           All rights reserved.
*/

/*
 *  This script is used to build images that are booted using the bootloader,
 *  versus downloaded and started using some third party tool such as xmd
 *  or the shell and the dynamic linker.
*/

/*
ENTRY(_start)

PROVIDE_HIDDEN(__exeentry = _start);
*/

/* Include the common portion of the script */
INCLUDE common.ld
