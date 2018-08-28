// -*-Mode: C;-*-
/**
@file
@brief Document the public "mem" library API.
@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/



/** 
    @name Virtual address queries
    @brief Functions that look up physical addresses and memory properties.

    Naming conventions:
    - "Phys", looks up the physical address.
    - "Flags", looks up a subset of memory property flags.
    - "Read", memory at the virtual address must be readable.
    - "Write", memory at the virtual address must be writable
       and is assumed to be readable.

    The functions that return a physical address return 0 to indicate
    a translation failure, returning an undefined flags result, if
    any. Functions that return only flags will return 0 to indicate a
    translation failure.

    @note For ARMv7: Only the MEM_READ, MEM_WRITE, MEM_CACHED,
    MEM_DEVICE, MEM_HANDSHAKE and MEM_SHARED flags will ever be set.
    MEM_WRITE is not set by functions that assume just readability.
    MEM_READ is always set by routines that assume writability except
    when address translation fails. Ordinary Device memory will have
    MEM_DEVICE set while Strongly Ordered memory will have both
    MEM_DEVICE and MEM_HANDSHAKE set.
    @{

    @fn uint32_t mem_getPhysRead        (uint32_t virt)
    @param[in] virt the virtual address
    @return the physical address

    @fn uint32_t mem_getPhysWrite       (uint32_t virt)
    @param[in] virt the virtual address
    @return the physical address

    @fn int      mem_getFlagsRead       (uint32_t virt)
    @param[in] virt the virtual address
    @return the property flags for the virtual address

    @fn int      mem_getFlagsWrite      (uint32_t virt)
    @param[in] virt the virtual address
    @return the property flags for the virtual address

    @fn uint32_t mem_getPhysFlagsRead   (uint32_t virt, int* flags)
    @param[in] virt the virtual address
    @param[out] flags the property flags for the virtual address
    @return the physical address

    @fn uint32_t mem_getPhysFlagsWrite  (uint32_t virt, int* flags)
    @param[in] virt the virtual address
    @param[out] flags the property flags for the virtual address
    @return the physical address

*/

/** @} */ // Virtual address queries




/** @name Memory properties
    @{
*/

/**
   @enum mem_AttributeFlag
   @brief Primitive memory properties expressed as 1-bit flags

   @var MEM_READ
   @brief Readable

   @var MEM_WRITE
   @brief Writeable

   @var MEM_EXEC
   @brief Executable

   @var MEM_CACHED
   @brief Cache enabled

   @var MEM_DEVICE
   @brief Mapped I/O device

   @var MEM_HANDSHAKE
   @brief Each transaction requires a handshake

   @var MEM_SHARED
   @brief (Multiprocessing) Shared memory

*/

/**
   @enum  mem_UseType
   @brief Useful combinations of the primitive property flags of mem_AttributeFlag

   @var MEM_RESERVED
   @brief Inaccessible

   @var MEM_NORMAL
   @brief Generic read/write RAM

   @var MEM_TEXT
   @brief Instructions and perhaps RO data

   @var MEM_RWDATA
   @brief Writeable data

   @var MEM_RODATA
   @brief Read-only data

   @var MEM_UNCACHED
   @brief Buffers used by external agents (DMA buffers)

   @var MEM_MMU
   @brief MMU tables

   @var MEM_REGISTERS
   @brief  Mapped I/O registers

   @var MEM_ROREGISTERS
   @brief Read-only mapped I/O registers

*/

/**
    @fn void mem_setFlags(uint32_t beginVirt, uint32_t endVirt, int flags)
    @brief Set memory properties. All pages overlapping the range
    [beginVirt, endVirt) will be affected.
    @param[in] beginVirt the lower virtual address limit (inclusive)
    @param[in] endVirt   the upper virtual address limit (exclusive)
    @param[in] flags the desired memory attribute flags

    The caller must ensure that none of the following preconditions
    are violated:

    Caching: Before changing a region of address space from cached
    to uncached there must be no valid cache lines for that
    region. There must never be cache hits for uncached regions.
    @code
    mem_storeDataCacheRange             (A, B, mem_mapL2Controller());
    mem_invalidateDataCacheRange        (A, B, mem_mapL2Controller());
    mem_invalidateInstructionCacheRange (A, B);
    mem_setFlags                        (A, B, MEM_UNCACHED);
    @endcode

    Write access: Before changing a region of address space from
    read/write to read-only there must be no dirty cache lines nor any
    uncompleted writes pending for that region. The caches and CPU
    cores must never try to write to a read-only region.
    @code
    mem_storeDataCacheRange (A, B, mem_mapL2Controller());
    mem_setFlags            (A, B, MEM_RODATA);
    @endcode

    Executablility: Before changing a region of address space from
    executable to non-executable there must be no valid instruction
    cache lines or branch predictions for that region, nor should any
    instructions from the region be in a CPU core's execution
    pipeline. A CPU core must never try to execute instructions
    originating from a non-executable region.
    @code
    mem_invalidateInstructionCacheRange (A, B);
    mem_setFlags                        (A, B, MEM_NORMAL & ~MEM_EXEC);
    @endcode

*/

/** @fn int mem_explode(uint32_t virtBegin, uint32_t virtEnd)
    @brief Alter the MMU tables so that the given virtual address range
    has 4KB pages (does nothing if that's already the case)
    @param[in] virtBegin the first virtual address (inclusive).
    @param[in] virtEnd   the ending virtual address (exclusive).
    @retval 1 for success
    @retval 0 for failure (no room for new page table entries)
*/


/** @if development
    @fn int mem_setupMmuTable()
    @brief Create and activate a two-level translation table at the location and
    with the properties specified by the global data structure
    mem_RegionTable.
    @return 1 on success and 0 on failure.

    When calling this function the MMU and the data caches must be
    disabled, though the instruction cache may be active. When this
    function returns the caches and MMU will be active. The
    fault-on-unaligned transfer bit in the system control register
    will be cleared.

    Setting up the MMU tables can fail if an attempt to allocate
    storage for a second-level table (a.k.a page table) fails.
    @endif
*/

/** @if development
    @fn void mem_printMmuTable(const uint32_t* tableStart, void (*printFunction)(const char* fmt, ...))
    @brief Print the MMU translation table using the given print function.
    @param[in] tableStart The address of the full 4096-entry first-level
    translation table.
    @param[in] printFunction A function like printf() except that it returns void.
    @endif
*/
/** @} */ // Memory properties





/** @name Cache management
    @brief All cache management functions assume that virtual address == real address.
    @{
    @fn void mem_storeDataCacheRange(uint32_t beginVirt, uint32_t endVirt, uint32_t l2Base)
    @brief Store (make clean) but don't invalidate all cache lines
    that contain data for any virtual addresses in the given range
    [beginVirt, endVirt). Storing is from all data or unified caches
    from L1 all the way down to main memory.
    @param[in] beginVirt the lower virtual address limit (inclusive)
    @param[in] endVirt   the upper virtual address limit (exclusive)
    @param[in] l2Base the VIRTUAL base address of the L2 cache
    controller's memory-mapped registers
*/

/** @fn void mem_storeDataCache(uint32_t l2Base)
    @brief Store (make clean) but don't invalidate all cache lines
    that contain data. Both L1 data and L2 caches are affected.
    @param[in] l2Base the VIRTUAL base address of the L2 cache
    controller's memory-mapped registers.
*/

/** @fn void mem_invalidateDataCacheRange(uint32_t beginVirt, uint32_t endVirt, uint32_t l2Base)
    @brief Invalidate all cache lines that contain data for any
    virtual addresses in the given range [beginVirt, endVirt). Invalidation
    is performed for both the L1 data and L2 caches.
    main memory.
    @param[in] beginVirt the lower virtual address limit (inclusive)
    @param[in] endVirt   the upper virtual address limit (exclusive)
    @param[in] l2Base the VIRTUAL base address of the L2 cache
    controller's memory-mapped registers. See mem_mapL2Controller().
*/

/** @fn void mem_invalidateDataCache(uint32_t l2Base)
    @brief Invalidate the entire L1 data and L2 caches.
    @param[in] l2Base The VIRTUAL base address of the L2 cache
    controller's memory-mapped registers. See mem_mapL2Controller().
*/

/** @fn void mem_invalidateInstructionCacheRange(uint32_t beginVirt, uint32_t endVirt)
    @brief Invalidate all instruction cache lines and branch
    predictions for any virtual addresses in the given range
    [beginVirt, endVirt). Only L1 instruction caches are affected; to
    invalidate L2 use mem_invalidateDataCacheRange().
    @param[in] beginVirt the lower virtual address limit (inclusive)
    @param[in] endVirt   the upper virtual address limit (exclusive)
*/

/** @fn void mem_invalidateInstructionCache(void)
    @brief Invalidate the entire instruction cache and all branch
    predictions. Only L1 instruction caches are affected; to
    invalidate the entire L2 cache use mem_invalidateDataCache().
*/

/** @} */ // Cache management



/** @name Access to device registers
    @{
*/

/** @if development
    @def MEM_L2CC_PHYSICAL_BASE
    @brief The physical base address of the L2 cache controller
    @endif
*/

/** @fn uint32_t mem_mapL2Controller(void)
    @brief Provide a good virtual base address for the L2 Cache Controller
    @return the virtual base address
*/

/** @fn void    mem_unmapL2Controller(void)
    @brief Release any resources allocated by mem_mapL2Controller()
*/


/** @if development
    @def MEM_SLCR_PHYSICAL_BASE
    @brief The physical base address of the System-Level Control Registers
    @endif
*/

/** @fn mem_mapSlcr(void)
    @brief Provide a good base address for the System-Level Control Registers.
    @return the virtual base address
*/

/** @fn void mem_unmapSlcr(void)
    @brief Release any resources allocated by mem_mapSlcr(). */

/** @} */ // Access to device registers



/** @name Memory region management 
    @{
*/
/** @enum mem_RegionName
    @brief Names of regions in the system's address space

    A Region is a contiguous range of the address space with the same
    default memory attributes, page size and in the most cases is used
    for similar purposes.


    @var MEM_REGION_SYSLOG
    @brief Saved output

    @var MEM_REGION_MMU
    @brief MMU translation tables

    @var MEM_REGION_RTS
    @brief Run-Time Support, dynamic loading area

    @var MEM_REGION_WORKSPACE
    @brief OS workspace (if any) + application workspace.

    @var MEM_REGION_UNCACHED
    @brief Device & protocol-plugin buffers (DMA buffers in general).

    @var MEM_REGION_SOCKET
    @brief Firmware sockets

    @var MEM_REGION_AXI0_TEST
    @brief AXI port 0 test area

    @var MEM_REGION_FW_VER_CTL
    @brief Firmware version info, some control register

    @var MEM_REGION_BSI
    @brief Bootstrap information

    @var MEM_REGION_AXI1_TEST
    @brief AXI port 1 test area

    @var MEM_REGION_USERDEV
    @brief User-defined non-PPI firmware registers

    @var MEM_REGION_USERPPI
    @brief User-defined PPI firmware registers

    @var MEM_REGION_IOP
    @brief IOP registers

    @var MEM_REGION_STATIC
    @brief Static memories

    @var MEM_REGION_HIGHREG
    @brief PS, CPU private registers, quad-SPI

    @var MEM_REGION_OCM
    @brief High-mapped OCM

*/

/** @fn char* mem_Region_alloc(int name, uint32_t numBytes)
    @brief Allocate memory within a given Region.
    @param[in] name The Region name (from mem_RegionName).
    @param[in] numBytes The number of bytes to allocate. This will be rounded up to
    an integral number of pages.
    @return a pointer to the allocated memory, which is always page-aligned.
*/

/** @fn void  mem_Region_undoAlloc(int name, char* allocPtr)
    @brief Undo the last allocation performed in the given Region
    @param[in] name the Region name (from mem_RegionName)
    @param[in] allocPtr the pointer returned by the most recent call to
    mem_Region_alloc() for the given Region
*/

/** @fn char* mem_Region_startAddress(int name)
    @brief Return the start address of the given Region.
    @param[in] name the Region name (from mem_RegionName)
    @return the start address
*/

/** @fn uint32_t mem_Region_remaining(int name)
    @brief Return the number of free bytes remaining to be allocated.
    @param[in] name the Region name (from mem_RegionName)
    @return the number of free bytes
*/

/** @fn uint32_t mem_Region_size(int name)
    @brief Return the total number of bytes in the Region, allocated or not.
    @param[in] name the Region name (from mem_RegionName)
    @return the total region capacity in bytes
*/

/** @fn uint32_t mem_Region_pageSize(int name)
    @brief Return the page size of a Region in bytes.
    @param[in] name the Region name (from mem_RegionName)
    @return the region's page size
*/

/** @fn uint32_t mem_Region_workspaceHoldback(void)
    @brief Return how many bytes of the WORKSPACE Region are held back from the OS, that is,
    are reserved for users.
    @return the number of bytes held back
*/

/** @fn void mem_Region_printConfig(void (*printFunction)(const char* fmt, ...))
    @brief Print the Region names, sizes, locations and default memory attributes
    using the given printing function
    @param[in] printFunction A function like printf() except that it returns void.
*/

/** @} */ // Memory region management
