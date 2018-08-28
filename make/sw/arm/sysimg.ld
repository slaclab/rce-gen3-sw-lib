/*
 *  This file contains directives for the GNU linker which are specific
 *  to the RCE.  Specifically, it is used to build images that are loaded
 *  using a third party tool such as xmd.
 */

/* Provide default definitions for some symbols. Note that PROVIDE and PROVIDE_HIDDEN
   have no effect unless both the following conditions hold:
   1) No other definition is found for the symbol provided.
   2) Some bit of program code refers to the symbol provided. A reference
      in the linker script is not enough.
To satisfy (2) we put references in the rcework section (see rcework.s).
*/

/*
PROVIDE_HIDDEN(__exeentry = _start);

ENTRY(_start)
*/

/* Include the common portion of the script */
INCLUDE common.ld
