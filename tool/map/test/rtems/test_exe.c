/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - October 24, 2013
**
**  Revision History:
**	    None.
**
** --
*/



#include "task/Task.h"

extern int main(int argc, char **argv);

extern void task_exit(void);

/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
  {
  main(argc,(char **)argv);
  }
  
/*
** ++
**
**
** --
*/

void Task_Rundown()
  {
  return;
  }
  
