#ifndef SwmDebug_hh
#define SwmDebug_hh

class AbsConsole;
class SwmGlobalMibCounters;
class SwmPortMibCounters;
class SwmPortMscCounters;
class SwmEplRegs;
class SwmPacket;
class SwmMacTable;
class SwmMacTableEntry;
class SwmVlanTableEntry;
class SwmFidTableEntry;
class SwmStreamStatus;
class SwmMibCounter;
class FmConfig;

AbsConsole& operator<<(AbsConsole& o,const SwmGlobalMibCounters& c);
AbsConsole& operator<<(AbsConsole& o,const SwmPortMibCounters& c);
AbsConsole& operator<<(AbsConsole& o,const SwmPortMscCounters& c);
AbsConsole& operator<<(AbsConsole& o,const SwmEplRegs& c);
AbsConsole& operator<<(AbsConsole& o,const SwmPacket& c);
AbsConsole& operator<<(AbsConsole& o,const SwmMacTable& c);
AbsConsole& operator<<(AbsConsole& o,const SwmMacTableEntry& c);
AbsConsole& operator<<(AbsConsole& o,const SwmVlanTableEntry& c);
AbsConsole& operator<<(AbsConsole& o,const SwmFidTableEntry& c);
AbsConsole& operator<<(AbsConsole& o,const FmConfig& c);
AbsConsole& operator<<(AbsConsole& o,const SwmStreamStatus& c);
AbsConsole& operator<<(AbsConsole& o,const SwmMibCounter& c);

#endif
