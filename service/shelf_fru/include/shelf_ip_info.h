
#ifndef SHELF_IP_INFO_H
#define SHELF_IP_INFO_H

typedef struct 
{
   char     dest[20];
   int      dest_type;
   uint32_t vlan;
   int      vlan_valid;
   uint32_t untagged;
   int      untagged_valid;
   uint32_t tagged;
   int      tagged_valid;
   uint32_t violation;
   int      violation_valid;
   uint32_t group_base;
   int      group_base_valid;
   uint32_t group_end;
   int      group_end_valid;
   uint32_t subnet_mask;
   int      subnet_mask_valid;
   uint32_t gateway;
   int      gateway_valid;
   int      dry_run;
   int      force;
} set_shelf_ip_info_args;

typedef struct
{
   char shelf[20];
   int  dry_run;
} delete_shelf_ip_info_args;

typedef struct
{
   char shelf[20];
   int  mr_area;
   int  raw;
} display_shelf_ip_info_args;

int set_shelf_ip_info(const set_shelf_ip_info_args* args);
int delete_shelf_ip_info(const delete_shelf_ip_info_args* args);
int display_shelf_ip_info(const display_shelf_ip_info_args* args);
void set_verbose(int verbose);

#endif
