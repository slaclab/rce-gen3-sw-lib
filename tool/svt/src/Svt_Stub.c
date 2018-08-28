// -*-Mode: C;-*-
/**
@cond development
@file Svt_Stub.c

@brief The SVT stub file which must be linked to any SVT in order for
       it to function as an SVT.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/



#include "ldr/Ldr.h"

/* @brief Lookup context for any one SVT.
**
** This array holds the lookup context for any one SVT. Beware: Its
** size is fixed at 8 integers, but in actuality it must be at least
** equal in size to the structure "STB" defined in "SVT.c". If the
** size of that structure changes this array must be changed.  This
** somewhat dubious code practice is a trade between sleaze and saving
** the need to expose that structure through another header file.
**
*/
static const int _stb[8] = {0};
 
/*
**
** Forward declarations for (facility private) local functions...
**
*/
 
uint32_t Svt_Setup(uint32_t table, Ldr_elf*, void* stb);
 
uint32_t lnk_prelude(uint32_t table, Ldr_elf* elf)
 { 
 return Svt_Setup(table, elf, (void*)_stb);
 }
 
 
/** @endcond */
