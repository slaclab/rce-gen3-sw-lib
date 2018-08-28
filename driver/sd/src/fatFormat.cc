/**
* @file      fatFormat.cc
*
* @brief     Module for formatting SD cards with a FAT file system of
*            optimal performance.
*
*            Note that this program will destroy data on the SD card.
*
*            This is based on $RTEMS_ROOT/testsuites/fstests/fsdosfsformat01.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      September 14, 2013 -- Created
*
* $Revision: 3121 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <fcntl.h>
#include <inttypes.h>
#include <sys/statvfs.h>
#include <rtems/libio.h>
#include <rtems/blkdev.h>
#include <rtems/dosfs.h>
#include <errno.h>

#include <bsp.h>


#include "debug/print.hh"
#include "time/Time.hh"


#define MAX_PATH_LENGTH 100         /* Maximum number of characters per path */
#define SECTOR_SIZE 512             /* sector size (bytes) */
#define FAT16_MAX_CLN 65525      /* maximum + 1 number of clusters for FAT16 */
#define FAT16_DEFAULT_SECTORS_PER_CLUSTER 32 /* Default number of sectors per cluster for FAT16 */

#define stringify(x) #x
#define rtems_test_exit(x, y)  goto y
#define rtems_test_assert(__exp, __out)                          \
  do {                                                           \
    if (!(__exp)) {                                              \
      printf( "%s: %d %s\n", __FILE__, __LINE__, #__exp );       \
      rtems_test_exit(0, __out);                                 \
    }                                                            \
  } while (0)


namespace driver {

  namespace sd {

static void test_disk_params(
  const char     *dev_name,
  const char     *mount_dir,
  const blksize_t sector_size,
  const blksize_t cluster_size,
  const blkcnt_t  sectors_per_cluster )
{
  int          rv;
  int          fildes;
  struct stat  stat_buff;
  char         file_name[MAX_PATH_LENGTH + 1];
  ssize_t      num_bytes;
  unsigned int value = (unsigned int) -1;

  snprintf( file_name, MAX_PATH_LENGTH, "%s/file1.txt", mount_dir );
  memset( &stat_buff, 0, sizeof( stat_buff ) );

  rv = mkdir( mount_dir, S_IRWXU | S_IRWXG | S_IRWXO );
  rtems_test_assert ( ((0 == rv) || (EEXIST == errno)), out1 );

  rv = mount( dev_name,
              mount_dir,
              RTEMS_FILESYSTEM_TYPE_DOSFS,
              RTEMS_FILESYSTEM_READ_WRITE,
              NULL );
  rtems_test_assert( 0 == rv, out1 );

  fildes = open( file_name,
                 O_RDWR | O_CREAT | O_TRUNC,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  rtems_test_assert( -1 != fildes, out2 );

  num_bytes = write( fildes, &value, sizeof( value ) );
  rtems_test_assert( sizeof( value ) == num_bytes, out3 );

  rv = fstat( fildes, &stat_buff );
  printf("%s: ID of device containing file:    st_dev     = %08x\n", __func__, (unsigned)stat_buff.st_dev);
  printf("%s: Inode number:                    st_ino     = %08x\n", __func__, (unsigned)stat_buff.st_ino);
  printf("%s: Protection:                      st_mode    = %08x\n", __func__, (unsigned)stat_buff.st_mode);
  printf("%s: Number of hard links:            st_nlink   = %08x\n", __func__, (unsigned)stat_buff.st_nlink);
  printf("%s: User ID of owner:                st_uid     = %08x\n", __func__, (unsigned)stat_buff.st_uid);
  printf("%s: Group ID of owner:               st_gid     = %08x\n", __func__, (unsigned)stat_buff.st_gid);
  printf("%s: Device ID (if special file):     st_rdev    = %08x\n", __func__, (unsigned)stat_buff.st_rdev);
  printf("%s: Total size, in bytes:            st_size    = %08x\n", __func__, (unsigned)stat_buff.st_size);
  printf("%s: Blocksize for file system I/O:   st_blksize = %08x\n", __func__, (unsigned)stat_buff.st_blksize);
  printf("%s: Number of 512B blocks allocated: st_blocks  = %08x\n", __func__, (unsigned)stat_buff.st_blocks);
  printf("%s: Time of last access:             st_atime   = %08x\n", __func__, (unsigned)stat_buff.st_atime);
  printf("%s: Time of last modification:       st_mtime   = %08x\n", __func__, (unsigned)stat_buff.st_mtime);
  printf("%s: Time of last status change:      st_ctime   = %08x\n", __func__, (unsigned)stat_buff.st_ctime);
  rtems_test_assert( 0 == rv, out3 );
  rtems_test_assert( S_ISREG( stat_buff.st_mode ), out3 );
  rtems_test_assert( sizeof( value ) == stat_buff.st_size, out3 );
  rtems_test_assert( cluster_size == stat_buff.st_blksize, out3 );
  rtems_test_assert( sectors_per_cluster
                     == ( stat_buff.st_blocks * sector_size / 512 ), out3 );
  rtems_test_assert( ( ( ( stat_buff.st_size + cluster_size
                           - 1 ) / cluster_size ) * cluster_size / 512 )
                     == stat_buff.st_blocks, out3 );
 out3:
  rv = close( fildes );
  rtems_test_assert( 0 == rv, out2 );

 out2:
  rv = unmount( mount_dir );
  rtems_test_assert( 0 == rv, out1 );

 out1:
  return;
}


static void format_partition(const char* dev_name, const char* label)
{
  rtems_status_code            sc;
  int                          rv;
  const char                   mount_dir[] = "/mnt/sd";
  msdos_format_request_param_t rqdata;

  memset( &rqdata, 0, sizeof( rqdata ) );

  /* Optimized for read/write speed.  Below, 0 => Default */
  rqdata.OEMName             = NULL;
  rqdata.VolLabel            = label;
  rqdata.sectors_per_cluster = 64;      /* bps(512) * spc(64) = 32K is max */
  rqdata.fat_num             = 0;
  rqdata.files_per_root_dir  = 0;
  rqdata.media               = 0;       /* Media code */
  rqdata.quick_format        = true;
  rqdata.skip_alignment      = false;
  rqdata.info_level          = MSDOS_FMT_INFO_LEVEL_DEBUG;

  sc = rtems_disk_io_initialize();
  rtems_test_assert( sc == RTEMS_SUCCESSFUL, out );

  rv = msdos_format( dev_name, &rqdata );
  if ( rv != 0 ) {
    printf("%s: msdos_format failed with return value %08x\n", __func__, rv);
    goto out;
  }

  test_disk_params( dev_name,
                    mount_dir,
                    SECTOR_SIZE,
                    SECTOR_SIZE * rqdata.sectors_per_cluster,
                    rqdata.sectors_per_cluster );
out:
  return;
}


extern "C" void rce_appmain()
{
  //const char dev_name[] = "/dev/sdA1";
  //const char label[]    = "BOOT";
  const char dev_name[] = "/dev/sdA2";
  const char label[]    = "USER";
  //const char dev_name[] = "/dev/sdA4";
  //const char label[]    = "RTEMS";
  //const char dev_name[] = "/dev/sdA5";
  //const char label[]    = "LINUX";

  uint64_t t0 = tool::time::lticks();

  format_partition(dev_name, label);

  static const uint32_t fClk = CPU_CLOCK_FREQUENCY / 2;
  uint64_t dT    = tool::time::lticks() - t0;
  uint32_t tSecs = dT / fClk;
  uint32_t mSecs = (1000 * dT) / fClk - 1000 * tSecs;

  tool::debug::printv("fatFormat: Exiting; Run time: %u.%03u seconds\n", tSecs, mSecs);
}

  } // sd

} // driver
