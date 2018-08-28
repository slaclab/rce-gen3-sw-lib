#include <stdio.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/ramdisk.h>
#include <rtems/dosfs.h>
#include <rtems/fsmount.h>

extern "C"
{

rtems_status_code create_ramdisk(
  const char *disk_name_path,
  uint32_t block_size,
  rtems_blkdev_bnum block_count
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_device_major_number major = 0;
  ramdisk *rd = NULL;
  dev_t dev = 0;

  /*
   * Register the RAM Disk driver.
   */
  printf ("Register RAM Disk Driver\n");
  sc = rtems_io_register_driver(0, &ramdisk_ops, &major);
  if (sc != RTEMS_SUCCESSFUL) {
    printf ("Register failed\n");
    return RTEMS_UNSATISFIED;
  }

  printf ("Allocate RAM Disk Driver\n");
  rd = ramdisk_allocate(NULL, block_size, block_count, false);
  if (rd == NULL) {
    rtems_io_unregister_driver(major);

    return RTEMS_UNSATISFIED;
    printf ("Allocate failed\n");
  }

  dev = rtems_filesystem_make_dev_t(major, 0);

  printf ("Create Disk\n");
  sc = rtems_disk_create_phys(
    dev,
    block_size,
    block_count,
    ramdisk_ioctl,
    rd,
    disk_name_path
  );
  if (sc != RTEMS_SUCCESSFUL) {
    ramdisk_free(rd);
    rtems_io_unregister_driver(major);

    printf ("Create disk failed: %u\n", sc);
    return RTEMS_UNSATISFIED;
  }

  return RTEMS_SUCCESSFUL;
}


/**
 * The RAM Disk configuration.
 */
rtems_ramdisk_config rtems_ramdisk_configuration[] =
{
  {
    512,   /* block size */
    32768, /* block_num */
    NULL   /* location */
  }
};

/**
 * The number of RAM Disk configurations.
 */
size_t rtems_ramdisk_configuration_size = 1;

}


void ramDisk()
{
  rtems_status_code sc;
  rtems_device_major_number major = 0;
  const char* dir = "/dev/rda";

#if 0
  printf("Initializing disk IO\n");
  sc = rtems_disk_io_initialize();
  printf("Disk IO initialize returned %u\n", sc);

  printf("Creating ramdisk at %s\n", dir);
  sc = create_ramdisk(dir, 512, 32768);
  printf("create_ramdisk returned %u\n", sc);
#else
  printf ("Register RAM Disk Driver\n");
  sc = rtems_io_register_driver(0, &ramdisk_ops, &major);
  if (sc != RTEMS_SUCCESSFUL) {
    printf ("Register returned %u\n", sc);
  }

  rtems_ramdisk_configuration[0].location = (void*)0x6f000000; //new uint8_t[rtems_ramdisk_configuration[0].block_size * rtems_ramdisk_configuration[0].block_num];
  printf("Initializing ramdisk at %p\n", rtems_ramdisk_configuration[0].location);
  sc = ramdisk_initialize(major, 0, 0);
  printf("ramdisk_initialize returned %u\n", sc);
#endif

  msdos_format_request_param_t rqdata;

  memset(&rqdata,0,sizeof(rqdata));
  rqdata.OEMName="Rtems";
  rqdata.VolLabel="MyRTEMSDisk";
//  rqdata.fattype=MSDOS_FMT_FAT32;
  rqdata.quick_format=TRUE;

  printf("Formatting %s\n", dir);
  int rc = msdos_format(dir, &rqdata);
  printf("Format returned %d\n", rc);

  fstab_t fs_mnt_table[] = {
    {
      dir, "/mnt/ramdisk",
      "dosfs", RTEMS_FILESYSTEM_READ_WRITE,
      RTEMS_FSTAB_ANY,
      RTEMS_FSTAB_NONE
    }
  };

  rtems_fsmount(fs_mnt_table,
                sizeof(fs_mnt_table)/sizeof(fs_mnt_table[0]),
                NULL);
}
