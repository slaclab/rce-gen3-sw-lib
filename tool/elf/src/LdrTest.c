/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC <mehsys@slac.stanford.edu>
**
**  Creation Date:
**	000 - , 2007
**
**  Revision History:
**	None.
**
** --
*/
 
#include <stdio.h>

#include "Ldr.h"
#include "Sym.h"

//
//#include "ldr/Ldr.h"

static Ldr_status _Link(Ldr_elf*);
static Ldr_status _Install(Ldr_name, Ldr_image, Ldr_elf**);
static Ldr_status _Translate_Install(Ldr_name, Ldr_elf**);
static Ldr_status _Load(Ldr_image, Ldr_elf**);

static const char CNTRL_1[] = "Construction failed with status:%d\n";

static Ldr_status _Start();

int main(int argc, char** argv)
 {

 Ldr_status error = _Start();
 
 if(error) printf(CNTRL_1, error);
 
 return 0;
 }

/*
** ++
**
**
** --
*/

static const char Snt[] = LDR_NAMEOF_SNT;
static const char Ant[] = LDR_NAMEOF_ANT;
static const char Rti[] = LDR_NAMEOF_RTI;

static const char sntImage[] = "snt.so";
static const char antImage[] = "ant.so";
static const char rtiImage[] = "rti.so";

Ldr_status _Start()
 {
 
 Ldr_status error = Ldr_Install_Rtems();

 if(error) return error;

 // mount filesystem (s) here...
  
 Ldr_elf* elf;
 
 error = _Install(Snt, sntImage, &elf);
  
 if(error) return error;
  
 Sym_SetupSys(elf);
  
 error = _Install(Ant, antImage, &elf);
  
 if(error) return error;
  
 Sym_SetupApp(elf);
  
 error = _Install(Rti, rtiImage, &elf);
  
 return error;
 }

/*
** ++
**
**
** --
*/

Ldr_status _Install(Ldr_name name, Ldr_image image, Ldr_elf** returned_elf)
 {
 
 Ldr_elf* elf = Ldr_Lookup(name);
 
 if((*returned_elf = elf)) return LDR_SUCCESS;
 
 int error = Ldr_Install(name, image, returned_elf);
 
 if(error) return error;  

 return _Link(*returned_elf);
 }

/*
** ++
**
**
** --
*/

Ldr_status _Translate_Install(Ldr_name name, Ldr_elf** returned_elf)
 {
 
 Ldr_elf* elf = Ldr_Lookup(name);
 
 if((*returned_elf = elf)) return LDR_SUCCESS;
 
 Ldr_image image = (Ldr_image)Sym_TranslateApp(name);
 
 if(!image) return LDR_NO_TRANSLATION;
 
 int error = Ldr_Install(name, image, returned_elf);
 
 if(error) return error;  
  
 return _Link(*returned_elf);
 }

/*
** ++
**
**
** --
*/

Ldr_status _Load(Ldr_image image, Ldr_elf** returned_elf)
 {
 
 int error = Ldr_Load(image, returned_elf);
 
 if(error) return error;  
  
 return _Link(*returned_elf);
 }

/*
** ++
**
**
** --
*/

Ldr_status _Link(Ldr_elf* this)
 {
 
 // actual link would happen here...
 
 Ldr_Fixup(this); 
  
 return LDR_SUCCESS;
 }


