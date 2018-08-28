/* Default linker script, for normal executables */

INCLUDE memory.ld
INCLUDE bsp.ld

/* Use RTEMS' character output routine */
outbyte       = BSP_output_char;
