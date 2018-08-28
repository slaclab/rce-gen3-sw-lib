// -*-Mode: C;-*-
/**
@cond development
@file Task_Config.c
@brief Document 
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

/** @endcond */

/**

   @def TASK_MAX_ARGS
   @brief Maximum number of parameters to be passed to a task.

   @struct Task_Attributes
   @brief Runtime attributes for a Task.  
   
   @var Task_Attributes::name
   @brief Task name (four characters)

   @var Task_Attributes::stack_size  
   @brief Task stack size (e.g. RTEMS_MINIMUM_STACK_SIZE)

   @var Task_Attributes::priority    
   @brief Task priority (255 (lowest) to 1 (highest)) 

   @var Task_Attributes::attributes  
   @brief Task attributes (e.g. RTEMS_DEFAULT_ATTRIBUTES)

   @var Task_Attributes::modes       
   @brief Task mode (e. g. RTEMS_DEFAULT_MODES)

   @var Task_Attributes::image       
   @brief Path specification for executable image

   @var Task_Attributes::argc        
   @brief Number of task parameters (MBZ (0) if none)

   @var Task_Attributes::argv        
   @brief Vector of task parameters...


*/
