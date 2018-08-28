// -*-Mode: C;-*-
/*!
*
* @brief   Functions for setting the boot mode
*
* @author  S. Maldonado - (smaldona@slac.stanford.edu)
*
* @date    April 22, 2014 -- Created
*
* $Revision: $
*
* @verbatim
*                               Copyright 2015
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rtems.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <zlib.h>
#include <rtems/libio.h>

#include "shell/bootmode.h"
#include "system/fs.h"
#include "debug/print.h"

#define BLOCKSIZE 256
#define CRCSIZE sizeof(uint32_t)
#define UBOOT_ENV_SIZE 131072

#define UBOOTENV SD_MOUNT_POINT_BOOT "/uboot.env"

static void _mount_boot(const char* t);

/*
** ++
**
**
** --
*/

const char _mount_error1[] = "SD mount point create failure 0x%x errno 0x%x: %s\n";
const char _mount_error2[] = "SD mount failure 0x%x: %s to %s\n";

static void _mount_boot(const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(SD_PARTITION_BOOT, name, SD_PARTITION_TYPE_BOOT, RTEMS_FILESYSTEM_READ_WRITE, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, SD_PARTITION_BOOT, name);
  }

/*
** ++
**
**
** --
*/


int set_bootmode(const char *modeboot, const char *loadbit)
  {
  uint32_t crc = crc32(0L, Z_NULL, 0);
  int fd = -1;
  int size;
  int bufsize = 0;

  unsigned char *buffer = malloc(UBOOT_ENV_SIZE);
  unsigned char *bufptr = buffer;

  unsigned char *filebuf = buffer+CRCSIZE;
  int fp,ep,lp;
  
  if(!modeboot && !loadbit) return 0;
  
  if(!bufptr)
    {
    printf("%s: error allocating file buffer\n",__func__);
    return -1;     
    }

  /* mount the boot partition */
  _mount_boot(SD_MOUNT_POINT_BOOT);

  fd = open(UBOOTENV,O_RDWR);
  if(fd == -1)
    {
    printf("%s: error opening file %s\n",__func__,UBOOTENV);
    return -1;
    }
    
  while(1)
    {
    size = read(fd,bufptr,BLOCKSIZE);
    if(size > 0)
      {  
      bufsize += size;
      bufptr += size;
      }
    else
      break;
      
  }
    
  if(loadbit)
    {
    for (fp = 0, ep = 0, lp = 0 ; fp < bufsize-CRCSIZE && ep < bufsize- CRCSIZE - 1; fp++) {
        if(filebuf[fp] == '\0')
          {
          if(loadbit && (!strcmp(&filebuf[lp],LOADBIT0) || !strcmp(&filebuf[lp],LOADBIT1)))
            {
            //printf("lp %d fp %d %s\n",lp,fp,&filebuf[lp]);
            strncpy(&filebuf[lp], loadbit, strlen(loadbit));
            //printf("updated lp %d fp %d %s\n",lp,fp,&filebuf[lp]);
            break;
            }
          if(fp)
            lp = fp+1;
          }
       if(filebuf[fp] == '\0' && filebuf[fp+1] == '\0')
         {
         if(loadbit)
           strncpy(&filebuf[fp+1],LOADBIT1,strlen(LOADBIT1));
         else
           strncpy(&filebuf[fp+1],LOADBIT0,strlen(LOADBIT0));
        // printf("added lp %d fp %d %s\n",lp,fp,&filebuf[fp+1]);        
         break;               
         }
      }  
    }

  if(modeboot)
    {
    filebuf = buffer+CRCSIZE;
    for (fp = 0, ep = 0, lp = 0 ; fp < bufsize-CRCSIZE && ep < bufsize -CRCSIZE - 1; fp++) {
        if(filebuf[fp] == '\0')
          {
          if(modeboot && (!strcmp(&filebuf[lp],LINUX) || !strcmp(&filebuf[lp],RTEMS) || !strcmp(&filebuf[lp],RAMDISK)))
            {
            //printf("lp %d fp %d %s\n",lp,fp,&filebuf[lp]);
            strncpy(&filebuf[lp], modeboot, strlen(modeboot));
            //printf("updated lp %d fp %d %s\n",lp,fp,&filebuf[lp]);
            break;
            }
          if(fp)
            lp = fp+1;
          }
      }  
    }
  
  crc = crc32(0L, Z_NULL, 0); 
  crc = crc32(crc, buffer+CRCSIZE, bufsize-CRCSIZE);  
  *(uint32_t*)buffer = crc;
 
  bufptr = buffer;
  lseek(fd,0,SEEK_SET);
  
  while(bufsize)
    {
    write(fd,bufptr,BLOCKSIZE);
    bufptr += BLOCKSIZE;
    bufsize -= BLOCKSIZE;
    }

  close(fd); 
  free(buffer);
  
  /* unmount the boot partition */
  unmount(SD_MOUNT_POINT_BOOT);
  
  return 0;
  
  }

int main(int argc, char** argv)
  {
  return 0;
  }
