add_library(cob_ipmc
  src/COB_cmds.c
  src/COB_util.c
  src/COB_ipmc.c
  src/COB_dump_bsi.c
)
target_link_libraries(cob_ipmc dslutil conversion freeipmi cmb)
target_include_directories(cob_ipmc PUBLIC .)

add_executable(cob_rce_reset src/cob_rce_reset.c)
target_link_libraries(cob_rce_reset cob_ipmc)
add_executable(cob_refresh_shelf_info  src/cob_refresh_shelf_info.c)
target_link_libraries(cob_refresh_shelf_info  cob_ipmc)
add_executable(cob_rtm_init  src/cob_rtm_init.c)
target_link_libraries(cob_rtm_init  cob_ipmc)
add_executable(cob_fru_init  src/cob_fru_init.c)
target_link_libraries(cob_fru_init  cob_ipmc)
add_executable(cob_raw_cmd  src/cob_raw_cmd.c)
target_link_libraries(cob_raw_cmd  cob_ipmc)
add_executable(cob_rmb_init  src/cob_rmb_init.c)
target_link_libraries(cob_rmb_init  cob_ipmc)
add_executable(cob_cold_data_reset src/cob_cold_data_reset.c)
target_link_libraries(cob_cold_data_reset cob_ipmc)
add_executable(cob_rce_set_zombie src/cob_rce_set_zombie.c)
target_link_libraries(cob_rce_set_zombie cob_ipmc)
add_executable(cob_rce_clr_zombie src/cob_rce_clr_zombie.c)
target_link_libraries(cob_rce_clr_zombie cob_ipmc)
add_executable(cob_dump  src/COB_dump.c src/cob_dump.c)
target_link_libraries(cob_dump cob_ipmc)
add_executable(cob_dump_bsi  src/COB_dump_bsi.c src/cob_dump_bsi.c)
target_link_libraries(cob_dump_bsi cob_ipmc)

install(TARGETS
	cob_rce_reset
        cob_refresh_shelf_info
        cob_rtm_init
        cob_fru_init
        cob_raw_cmd
        cob_rmb_init
        cob_cold_data_reset
        cob_rce_set_zombie
        cob_rce_clr_zombie
        cob_dump
        cob_dump_bsi
	DESTINATION bin)
