#
# To add a new HW type to the database, add the Class and Type (page title)
# to the classToType dictionary.
#

# The space where the HW Pages exist
HW_PAGE_SPACE = "airtrack"

## @brief Map device class to a list of valid hardware description pages.
#
# Valid titles for hardware description pages. 
classToType = dict(
    COB   = ["PC_248_101_01_C05", 
             "PC_248_101_01_C06", 
             "PC_248_101_01_C07", 
             "PC_248_101_01_C08"],


    RTM   = ["PC_248_101_20_C00", 
             "PC_248_101_20_C01", 
             "PC_248_101_20_C02", 
             "PC_248_101_20_C03", 
             "PC_248_101_20_C04", 
             "PC_248_101_20_C05", 
             "PC_248_101_28_C00", 
             "PC_248_101_28_C01", 
             "PC_248_101_28_C02", 
             "PC_248_101_20_C03", 
             "PC_248_101_20_C04", 
             "PC_256_100_84_C01", 
             "PC_256_100_85_C01",
             "LCA-11566-A"],

    LBNE_RTM = ["PC_264_810_02_C00",
             "PC_264_810_02_C01"],

    RMB   = ["PC_256_100_86_C01"],

    AMC_CARRIER = [
             "PC_379_396_01_C00",
             "PC_379_396_01_C01",
             "PC_379_396_01_C02",
             "PC_379_396_01_C03",
             "PC_379_396_01_C04"],
            

    FLASH = ["PC_248_101_11_C00"],

    DTM   = ["PC_248_101_19_C00", 
             "PC_248_101_19_C01", 
             "PC_248_101_19_C02"],

    DPM   = ["PC_248_101_18_C00", 
             "PC_248_101_18_C01", 
             "PC_248_101_18_C02"],

    RTB   = ["LCA-10955-B"],

    OTM   = ["LCA-11545", "LCA-11545-B"],

    SCI   = ["LCA-11726-A"],
    
    HSIO  = ["PC_256_100_83_C00",
             "PC_256_100_83_C01",
             "PC_256_100_83_C02",
             "PC_256_100_83_C03"],

    HSIO_TTC = ["PC_256_100_89_C00","PC_256_100_89_C01"],

    POHANG_CAVITY_BPM = ["PC_144_163_01_C00"],

    uTCA_RTM_PI = ["PC_714_102_14_C00"],

    IPM_V2_BOX = ["SA_261_201_54_C00"],

    HSIO_RJ45_RTM = ["PC_256_100_88_C00"]
    )

################################################################################
################################################################################
################################################################################

CLASSES = classToType.keys()
CLASSES.sort()

## @brief Map device page title to device class.
typeToClass = dict()
for _class, _pages in classToType.iteritems():
    for _p in _pages:
        typeToClass[_p] = _class
