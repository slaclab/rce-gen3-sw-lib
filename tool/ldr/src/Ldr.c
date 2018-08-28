// -*-Mode: C;-*-
/**
@cond development
@file Ldr.c
@brief Implementation of ELF loader utilities.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
  Author:
      Michael Huffer, SLAC (mehsys@slac.stanford.edu)

*/

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>   // printf...


#include "kvt/Kvt.h"
#include "memory/mem.h"
#include "ldr/Ldr.h"

/*
**
** This constant specifies the maximum number of loaded images that can be managed. If 
** this value is exceeded "Ldr_Load" will return an error. Note that this value must be 
** expressed as power of two... 
**
*/

#define MAX_IMAGES 8 // As a power of 2, i.e., 8 = 256 entries 

/*
**
** This constant specifies the maximum number of loaded images that can be managed. If 
** this value is exceeded "Ldr_Load" will return an error. Note that this value must be 
** expressed as power of two... 
**
*/

#define MAX_NAME_SPACES 9 // As a power of 2, i.e., 8 = 512 entries 

/*
**
** This constant defines the maximum number of segments that any one image may contain.
** This constant is needed as the program segments are carried on the stack. 
** The particular value chosen as a good trade towards minimizing stack used as a typical 
** images contains about four program segments. If an input image exceeds this number the 
** "Ldr_Load" function will fail, returning an error value of "LDR_NOT_ELF_FILE". 
**
*/

#define MAX_SEGMENTS 64

/*
**
** Although the loader is targeted to the ARM 32-bit architecture, in fact ELF supports
** either 32-bit or 64-bit architectures. These #defines serve to serve as at least a 
** start of porting these functions should there be a move to a 64-bit architecture.  
**
*/

#define CLASS ELFCLASS32    
#define DATA  ELFDATA2MSB   // LSB = little-endian MSB = big endian
#define OSABI ELFOSABI_NONE

typedef Elf32_Sym Symbol;
typedef Elf32_Word Word;

typedef Elf32_Phdr  Segment;
typedef Elf32_Word  SegmentTag;  
typedef Elf32_Dyn   Element; 
typedef Elf32_Sword DynamicTag;

/*
**
** POSIX return codes for file I/O operations...
**
*/
 
#define READ_ERROR -1
#define INVALID    -1

/*
** ++
**
**
** --
*/

static KvTable _images_table    = (KvTable)0;
static KvTable _namespace_table = (KvTable)0;

/*
**
** Forward declarations for local functions...
**
*/
static const char* _parse(const Ldr_name, char* namespace);
static KvTable     _construct(uint32_t size2);
static uint32_t    _isElf(Ldr_elf*);
static Ldr_status  _validate(Ldr_elf*, Ldr_name);
static Segment*    _set(Segment*, uint8_t* offset);
static Ldr_elf*    _load(int fd, Ldr_name, Ldr_status*);
static uint32_t    _seek_size(Segment* base, Segment* end);
static Segment*    _pageIn(Segment*, int fd, Ldr_status*);
static Ldr_status  _populate(Ldr_elf*, Segment* base, Segment* end, int fd);
static int         _zero(uint8_t* buffer, uint32_t bufsiz);
static int         _read(int fd, uint32_t offset, uint8_t* vaddr, uint32_t filesz);
static Segment*    _iterate(Segment* first, Segment* last);
static Segment*    _segment(SegmentTag, Ldr_elf*);
static Element*    _element(DynamicTag, Element* vector);

/*
** ++
**
**  initialize the loader facility. This function must be called once and only once over 
**  the lifetime of the system. Its function is to. Basically it allocates memory from the 
**  RTS region to hold its corresponding. This function 
**
** --
*/
 
#define RTEMS_REGION MEM_REGION_RTS

Ldr_status Ldr_LoadRtems(Ldr_name ourname)
 {

 Ldr_elf* ourself = (Ldr_elf*)mem_Region_startAddress(RTEMS_REGION);
  
 Ldr_status error = _validate(ourself, ourname);
 
 if(error) return error;
 
 uint32_t size = Ldr_Sizeof(ourself);
   
 if(!size) return LDR_NOT_ELF_IMAGE;
 
 if(!mem_Region_alloc(RTEMS_REGION, size))             return LDR_INSFMEM;
 
 if(!(_images_table = _construct(MAX_IMAGES)))         return LDR_INSFMEM;
 
 if(!(_namespace_table = _construct(MAX_NAME_SPACES))) return LDR_INSFMEM;

 Ldr_Fixup(ourself);
 
 return Ldr_Install(ourself);
 }

/*
** ++
**
**
** --
*/

#define KVT_REGION MEM_REGION_WORKSPACE

static KvTable _construct(uint32_t size2)
 {
 
 char* buffer = mem_Region_alloc(KVT_REGION, KvtSizeof(size2));

 if(!buffer) return (KvTable)0;

 return KvtConstruct(size2, buffer);
 } 

/*
** ++
**
**
** --
*/

Ldr_path Ldr_Assign(Ldr_namespace this, Ldr_path path)
 {

 return (Ldr_path)KvtInsert(KvtHash(this), (void*)path, _namespace_table);
 }

/*
** ++
**
**
** --
*/

Ldr_path Ldr_Rename(Ldr_namespace this, Ldr_path path) 
 {
 
 return (Ldr_path)KvtRename(KvtHash(this), (void*)path, _namespace_table); 
 }

/*
** ++
**
**
** --
*/

Ldr_path Ldr_Remove(Ldr_namespace this) 
 {
 
 return (Ldr_path)KvtRemove(KvtHash(this), _namespace_table); 
 }
    
/*
** ++
**
**
** --
*/

Ldr_path Ldr_Map(const Ldr_name this, char* buffer)
 {

 char namespace[LDR_MAX_PATH];
 
 const char* name = _parse(this, namespace);
 
 if(!name)  return LDR_MAP_ERROR; // NIL string..

 const char* prefix = KvtLookup(KvtHash(namespace), _namespace_table);

 if(!prefix) return LDR_MAP_ERROR;

 if((strlen(prefix) + strlen(name) + 1) > LDR_MAX_PATH) return LDR_MAP_ERROR;

 return strcat(strcpy(buffer, prefix), name);
 }
 
/*
** ++
**
**
** --
*/
 
#define TERMINATE ':'
 
static const char* _parse(const Ldr_name this, char* namespace)
 {
 
 const char* src  = this;
 char        next = *src++;
 
 if(!next)             return LDR_MAP_ERROR;
 if(next == TERMINATE) return LDR_MAP_ERROR;

 char* dst       = namespace;
 int   remaining = LDR_MAX_PATH - 1;
 
 do
   {
   if(!remaining--) return LDR_MAP_ERROR;
   if(next == TERMINATE) break;
   *dst++ = next;
   next = *src++;
   }
 while(next);
     
 *dst = 0;
 
 return next == TERMINATE ? src : LDR_MAP_ERROR;
 }
  
/*
** ++
**
**
** --
*/

Ldr_elf* Ldr_Lookup(Ldr_name this)
 {
 return (Ldr_elf*)KvtLookup(KvtHash(this), _images_table);
 }

/*
** ++
**
**
** --
*/

void Ldr_Fixup(Ldr_elf* this)
 {

 uint8_t* offset = (uint8_t*)this;

 uint32_t phnum = this->e_phnum;
 
 if(!phnum) return;
 
 Segment* first  = (Segment*)((uint8_t*)this + (uint32_t)this->e_phoff);
 Segment* last   =  &first[phnum];
 Segment* next   = _iterate(first, last);
   
 while(next != last) next = _iterate(_set(next, offset), last);  
 
 return;
 }

/*
** ++
**
** It's assumed that the initial permissions of the image's memory are such that the 
** permission changes we make here won't require any cache manipulation. 
**
** --
*/

static Segment* _set(Segment* this, uint8_t* offset)
 {

 uint8_t* low  = offset + this->p_vaddr;
 uint8_t* high = low + this->p_memsz;
   
 unsigned flags  = this->p_flags;   
 unsigned access = MEM_CACHED;
 
 if(flags & PF_X) access |= MEM_EXEC;
 if(flags & PF_R) access |= MEM_READ;
 if(flags & PF_W) access |= MEM_WRITE;
 
 mem_setFlags((uintptr_t)low, (uintptr_t)high, access);
  
 return ++this;
 }

/*
** ++
**
**
** --
*/

static Ldr_status _validate(Ldr_elf* this, Ldr_name name)
 {

 Ldr_name thisName = Ldr_Name(this);  
 
 KvtKey this_key = KvtHash(thisName);
 KvtKey name_key = KvtHash(name);
 
 return this_key == name_key ? LDR_SUCCESS : LDR_IMAGE_NO_MATCH;
 }
                 
/*
** ++
**
**
** --
*/

uint32_t Ldr_Sizeof(Ldr_elf* this)
 {
 
 uint32_t phnum = this->e_phnum;

 if(!phnum) return 0;
 
 Segment* first = (Segment*)((uint8_t*)this + (uint32_t)this->e_phoff);
 Segment* last  =  &first[phnum];
 
 first = _iterate(first, last);
 
 return first != last ? _seek_size(first, last) : 0;
 }

/*
** ++
**
**
** --
*/

static uint32_t _isElf(Ldr_elf* this)
 {
 
 if(this->e_ident[EI_MAG0]    != ELFMAG0)    return 0;
 if(this->e_ident[EI_MAG1]    != ELFMAG1)    return 0;
 if(this->e_ident[EI_MAG2]    != ELFMAG2)    return 0;
 if(this->e_ident[EI_MAG3]    != ELFMAG3)    return 0;
 if(this->e_ident[EI_CLASS]   != CLASS)      return 0;;
 if(this->e_ident[EI_VERSION] != EV_CURRENT) return 0;
 
 if(this->e_version    != EV_CURRENT)      return 0;
 if(this->e_ehsize     != sizeof(Ldr_elf)) return 0;
 if(this->e_phentsize  != sizeof(Segment)) return 0;

 if(!this->e_phoff) return 0; 
 
 uint32_t phnum = (uint32_t)this->e_phnum; // # of program headers...

 return phnum <= MAX_SEGMENTS ? phnum : 0;
 }

/*
** ++
**
**
** --
*/

Ldr_name Ldr_Name(Ldr_elf* this)
 {

 uint8_t* offset = (uint8_t*)this;
  
 Segment* segment = _segment(PT_DYNAMIC, this);
 
 if(!segment) return (Ldr_name)0;
 
 Element* vector  = (Element*)(offset + segment->p_vaddr);
 Element* element = _element(DT_STRTAB, vector);
 const char* strings = (char*)(offset + element->d_un.d_ptr);
   
 element = _element(DT_SONAME, vector);

 if(!element) return (Ldr_name)0;
  
 return &strings[element->d_un.d_val];  
 }
                
/*
** ++
**
**  This function sums the size (in bytes) over all the loadable segments in an ELF 
**  object and returns that value. It has two arguments: 
**  First, a pointer to the object's base segment descriptor and second, a pointer which 
**  points past just past the last segment descriptor in the the ELF's corresponding 
**  program table. This argument is used as a sentinal value to determine end of list.      
**  This function returns the total amount of storage (in bytes) required of all the 
**  loadable segments in the corresponding ELF image.
**  
** --
*/

static uint32_t _seek_size(Segment* base, Segment* end)
 {
 
 Segment* next = base;
 Segment* last = next;
  
 next = _iterate(++next, end);
 
 while(next != end)
  {
  last = next;
  next = _iterate(++next, end);
  }
 
 uint32_t size = (uint32_t)last->p_vaddr - (uint32_t)base->p_vaddr;
  
 size += (uint32_t)last->p_memsz;
 
 return size;
 }

/*
** ++
**
** This function iterates over the vector of program headers For a range of program 
** segments this function returns the first loadable program segment within that 
** (inclusive) range. The function takes two arguments: The first is a pointer to the 
** first segment within the range to be searched and the second points just past the last  
** a pointer to the last segment of the range. Typically, the second argument will 
** point to the  
**
** --
*/

static Segment* _iterate(Segment* first, Segment* last)
 {
 
 Segment* segment = first; 

 do {if(segment->p_type == PT_LOAD) break; segment++;} while(segment != last); 
   
 return segment;
 }

/*
** ++
**
**
** --
*/

Ldr_status Ldr_Install(Ldr_elf* this)
 {
 
 Ldr_name name = Ldr_Name(this);

 if(!name) return LDR_IMAGE_WITHOUT_NAME;
 
 KvtKey elf_key = KvtHash(name);

 return KvtInsert(elf_key, (void*)this, _images_table) ? LDR_SUCCESS : LDR_IMAGE_INSTALLED;
 }

/*
** ++
**
**
** --
*/

Ldr_elf* Ldr_Load(Ldr_name this, Ldr_status* status)
 {
 
 *status = LDR_NO_SUCH_FILE;

 char string[LDR_MAX_PATH];

 const Ldr_path path = Ldr_Map(this, string);
 
 if(!path) return LDR_LOAD_ERROR; 
    
 int fd = open(path, O_RDONLY);
 
 if(fd == INVALID) return LDR_LOAD_ERROR;
 
 Ldr_elf* elf = _load(fd, this, status);
    
 close(fd);

 Ldr_status error = *status;

 if(!error) return elf; // success!
 
 if(!elf)   return elf; // error, but doesn't need deallocation...
  
 mem_Region_undoAlloc(MEM_REGION_RTS, (char*)elf);
 
 return LDR_LOAD_ERROR;
 }
 
/*
** ++
**
**
** --
*/

static Ldr_elf* _load(int fd, Ldr_name name, Ldr_status* status)
 {
 
 Segment segments[MAX_SEGMENTS];
 
 Segment* end = _pageIn(segments, fd, status); // validate header and fetch program segments
 
 *status = LDR_NOT_ELF_FILE;
 
 if(!end) return LDR_LOAD_ERROR; 

 Segment* base = _iterate(segments, end); // establish first loadable segment
 
 *status = LDR_NOT_LOADABLE;
  
 if(base == end)   return LDR_LOAD_ERROR; 
 if(base->p_vaddr) return LDR_LOAD_ERROR; 
 
 int32_t size = _seek_size(base, end); // establish how much memory is needed
 
 if(!size) return LDR_LOAD_ERROR;

 Ldr_elf* this = (Ldr_elf*)mem_Region_alloc(MEM_REGION_RTS, size);

 *status = LDR_INSFMEM;

 if(!this) return LDR_LOAD_ERROR;
 
 Ldr_status error = _populate(this, base, end, fd); // copy all loable segments to allocated memory

 if(!error) error = _validate(this, name);

 *status = error;
 
 return this;
 }

/*
** ++
**
**
** --
*/

static Segment* _pageIn(Segment* segments, int fd, Ldr_status* status)
 {
 
 Ldr_elf header;
 
 if(_read(fd, 0, (uint8_t*)&header, sizeof(Ldr_elf)) == READ_ERROR) return (Segment*)0;
 
 uint32_t phnum = _isElf(&header);

 if(!phnum)                  return (Segment*)0;
 if(header.e_type != ET_DYN) return (Segment*)0;
 
 int error = _read(fd, (uint32_t)header.e_phoff, (uint8_t*)segments, sizeof(Segment) * phnum);
 
 *status = LDR_NOT_ELF_FILE;
 
 return error != READ_ERROR ? &segments[phnum] : (Segment*)0;
 }

/*
** ++
**
**
** --
*/

static Ldr_status _populate(Ldr_elf* elf, Segment* base, Segment* end, int fd)
 {
 
 Segment* next = base; 
 
 uint32_t filesz     = next->p_filesz;
 uint32_t nzero      = next->p_memsz - filesz;
 uint32_t offset     = next->p_offset;
 uint8_t* base_vaddr = (uint8_t*)elf;
 uint8_t* vaddr      = base_vaddr + (uint32_t)next->p_vaddr;
 
 int status = _read(fd, offset, vaddr, filesz);
 
 if(status == READ_ERROR) return LDR_FILEIO_ERROR;

 if (nzero) _zero(vaddr + filesz, nzero);

 while((next = _iterate(++next, end)) != end)
   {
    
   filesz = next->p_filesz;
   nzero  = next->p_memsz - filesz;
   offset = next->p_offset;
   vaddr  = base_vaddr + (uint32_t)next->p_vaddr;
   
   status  = _read(fd, offset, vaddr, filesz);
   
   if(status == READ_ERROR) return LDR_FILEIO_ERROR;

   if (nzero) _zero(vaddr + filesz, nzero);
   }
   
 return LDR_SUCCESS;
 } 

/*
** ++
**
**
** --
*/

static int _read(int fd, uint32_t offset, uint8_t* buffer, uint32_t bufsiz)
 {
 
 int error = lseek(fd, offset, SEEK_SET);
 
 if(error == READ_ERROR) return error;

 return read(fd, (void*)buffer, bufsiz);
 }

/*
** ++
**
**
** --
*/

//static const char CNTRL_1[] = "zero %d bytes @ %x from %s.\n";

static int _zero(uint8_t* buffer, uint32_t bufsiz)
 {

//printf(CNTRL_1, (int)bufsiz, (int)buffer, filename);

 memset((void*)buffer, '\0', bufsiz);
 
 return 0;
 }

/*
** ++
**
**
** --
*/

static Segment* _segment(SegmentTag tag, Ldr_elf* elf)
 {
 
 Segment* segment   = (Segment*)((uint8_t*)elf + (uint32_t)elf->e_phoff); 
 uint32_t remaining = (uint32_t)elf->e_phnum;

 while(remaining--) {if(segment->p_type == tag) return segment; segment++;}  

 return (Segment*)0;
 }
 
/*
** ++
**
**
** --
*/

static Element* _element(DynamicTag tag, Element* vector)
 {
 
 Element* entry = vector;

 while(entry->d_tag != DT_NULL) {if(entry->d_tag == tag) return entry; entry++;}  

 return (Element*)0;  
 }


/** @endcond */

// Documentation

// #defines
/**
   
   @def LDR
   @brief Facility number.  See configuration/system

   @def LDR_MAX_PATH
   @brief Maximum size (in number of characters), including NULL termination of a
   path name returned by ::Ldr_Map

   @def LDR_MAP_ERROR
   @brief Error value indicating that mapping the name of a sharable
   image to its path has failed.  Returned by ::Ldr_Load
   or ::Ldr_LoadRtems.

   @def LDR_LOAD_ERROR
   @brief Error value indicating that a sharable has failed to load
   for some reason.  Returned by ::Ldr_Load or
   ::Ldr_LoadRtems.
*/

//  typedefs
/**
   
   @typedef Ldr_elf
   @brief An ELF file header (32-bit ARM only)
   
   @typedef Ldr_name
   @brief The name of a sharable image (as a C string).
   
   @typedef Ldr_namespace
   @brief The name of an image \ref namespace "namespace" (as a C string).
   
   @typedef Ldr_path
   @brief The path name for an image (as a C string).
*/

// enums
/**
   
   @enum Ldr_status
   @brief The possible status values returned by ::Ldr_LoadRtems and ::Ldr_Install.
   
   @var LDR_SUCCESS
   @brief Zero is always success, non-zero an error...
   
   @var LDR_NO_TRANSLATION
   @brief Cannot map SO to FILE name

   @var LDR_NO_SUCH_FILE
   @brief File cannot be opened (not found or access violation)

   @var LDR_NOT_ELF_FILE
   @brief File is not an ELF file

   @var LDR_NOT_LOADABLE
   @brief File does not contain any loadable segments

   @var LDR_INSFMEM
   @brief Region does not contain enough free space for image 

   @var LDR_FILEIO_ERROR
   @brief File read function function retuned with error

   @var LDR_IMAGE_NO_MATCH
   @brief ELF image SO name did not match requested SO name

   @var LDR_NOT_ELF_IMAGE
   @brief Image is not an ELF image 

   @var LDR_IMAGE_WITHOUT_NAME
   @brief Specified ELF image did not define an SO name

   @var LDR_IMAGE_INSTALLED
   @brief Loaded image had previously been installed

   @var LDR_NO_DYN_SEGMENT
   @brief Image does not contain a dynamic segment

   @var LDR_NO_STRING_TABLE
   @brief Image does not contain a string table

   @var LDR_NO_SYMBOL_TABLE
   @brief Image does not contain a symbol table

   @var LDR_NO_HASH_TABLE
   @brief Image does not contain a hash table
*/


/**
   @fn Ldr_path Ldr_Assign(Ldr_namespace namespace, Ldr_path path)
   @brief   Assign a file path to a given \ref namespace "namespace".
   @param[in] namespace   A \ref namespace "namespace" specification.
   @param[in] path   A file path string.
   @return A pointer to the path string, or NIL if the function failed
   due to exceeding the maximum number of allowed namespaces.   
   @note The path argument should be considered a STATIC string which
   must not be dynamically allocated.


   @fn Ldr_path Ldr_Map(Ldr_name name, char* buffer)
   @brief Return the file path for the image corresponding to a specified \ref image_name "name". 
   @param[in] name  Pointer to a C string which describes the unique name of an image.
   @param[in] buffer  Pointer to a buffer which will contain the resultant path.
   This buffer must be at least ::LDR_MAX_PATH in size.
   @return A pointer to the buffer with the NULL terminated path, or
   NIL if the lookup has failed for one of the following reasons:
   -# The \ref namespace "namespace" has not been assigned a path (see ::Ldr_Assign)
   -# Resulting path specification is larger than ::LDR_MAX_PATH
      (including NULL termination)
   -# String is empty, composed of only NULL
   -# String contains only the single delimiter ":".


   @fn Ldr_path Ldr_Rename(Ldr_namespace namespace, Ldr_path path)
   @brief   Rename the assigned path for a specified \ref namespace "namespace".
   @param[in] namespace   A \ref namespace "namespace" specification.
   @param[in] path   A file path string.
   @return
   @note The path argument should be considered a STATIC string which
   must not be dynamically allocated.

   
   @fn Ldr_path Ldr_Remove(Ldr_namespace namespace)
   @brief Remove (delete) the specified \ref namespace "namespace".
   @param[in] namespace   A \ref namespace "namespace" specification.
   @return The string corresponding to the current assignment, or NIL (0)
   if no path was assigned to the supplied namespace.


   @fn Ldr_elf* Ldr_Load(Ldr_name name, Ldr_status* status)
   @brief Load an ELF image corresponding to its name. 
   @param[in] name  A string specifying the \ref image_name "name" of the ELF image to be loaded.
   @param[out] status  A pointer to the location where the status
   information is to be returned. The status values which may be returned are
   enumerated by ::Ldr_status.  A value of ::LDR_SUCCESS (or zero) indicates
   the function completed successfully. Any non-zero value indicates an error
   whose exact value specifies which type of error. 
   @return A pointer to the loaded, memory-resident ELF image, or NIL(0) if
   there was a failure (in which case see the value of status).
   @note The image's corresponding FILE is discovered by translating the
   specified name using ::Ldr_Map.


   @fn Ldr_elf* Ldr_Lookup(Ldr_name name)
   @brief Search the installed database for an image (previously installed
   by ::Ldr_Install) corresponding to a name.
   @param[in] name   A string specifying the \ref image_name "name" of the ELF image to be looked up
   @return A pointer to the loaded ELF image, or NIL (0) if the image was not found.


   @fn void Ldr_Fixup(Ldr_elf* elf)
   @brief Set the appropriate page attributes for the memory corresponding to
   all the loadable segments of a specified ELF image. 
   @param[in] elf A pointer to the loaded ELF image.
   
   This function must be called \b once and \b only \b once for any
   unique loaded image. For this reason usage of this function is
   typically reserved for the dynamic linker when it both loads and
   links an image. In such a case the memory occupied by the image
   immediately after being loaded is set such that can be modified as
   necessary, for example by the linker in performing its necessary
   relocation as well as fix-up.


   @fn Ldr_status Ldr_LoadRtems(Ldr_name name)
   @brief Set up and initializes the loader (Ldr) facility.
   @param[in] name The input System image \ref image_name "name" (i.e. \c system:rtems.so).
   @return LDR_SUCCESS if successful.  Non-Zero if not (see ::Ldr_status).
   @note This function must be called once and only once in the
   lifetime of a booted system. It must also be (successfully) called
   before using any of the other functions in this package.


   @fn uint8_t* Ldr_Offset(Ldr_elf* elf)
   @brief Returns the relocation offset to be added to any address contained within 
   a specified elf image. 
   @param[in] elf A pointer to the ELF image.
   @return A pointer corresponding to the offset.
   
   This offset can be added to any ELF address to return its absolute
   location. Note that this function returns a legitimate value
   independent of where the image was loaded or how it was linked,
   either statically or dynamically.

   @note It is safe to call this function BEFORE the loader facility is initialized 
   (i.e. before calling ::Ldr_LoadRtems).


   @fn uint8_t* Ldr_Entry(Ldr_elf* elf)
   @brief Return the (relocated) value of an specified ELF header "entry" vector. 
   @param[in] elf A pointer to the ELF image.
   @return The relocated value of the entry point.


   @fn uint32_t Ldr_Sizeof(Ldr_elf* elf)
   @brief Return the amount of memory occupied by a specified elf image. 
   @param[in] elf A pointer to the ELF image.
   @return The amount of memory occupied by a specified ELF image (in bytes), or
   zero (0) if the image cannot be parsed.
   
   @note It is safe to call this function BEFORE the loader facility is initialized 
   (i.e.before calling ::Ldr_LoadRtems).   


   @fn Ldr_name Ldr_Name(Ldr_elf* elf)
   @brief Return the name of a specified ELF image.
   @param[in] elf  A pointer to the ELF image.
   @return A pointer to the \ref image_name "name".  If the specified
   image cannot be correctly parsed or an SO name was not assigned to
   the image, a NIL (zero) pointer will be returned.


   @fn uint8_t* Ldr_Offset(Ldr_elf* elf)
   @brief Return the relocation offset to be added to any address contained within 
   a specified ELF image. 
   @param[in] elf  A pointer to the ELF image.
   @return A pointer corresponding to that offset.

   This offset can be added to any ELF address to return its
   absolute location. Note that this function returns a legitimate value independent of 
   where the image was loaded or how it was linked, either statically or dynamically.

   @note It is safe to call this function BEFORE the loader facility is initialized 
   (i.e. before calling ::Ldr_Install).   


   @fn Ldr_status Ldr_Install(Ldr_elf* elf)
   @brief Install a specified ELF image.
   @param[in] elf A pointer to the ELF image.
   @return LDR_SUCCESS (0) if successful.  Non-zero values indicate an
   error (see ::Ldr_status).

*/
