#ifndef CimNetIGMP_hh
#define CimNetIGMP_hh

class IGMPMessage {
public:
  unsigned char  type;
  unsigned char  code;
  unsigned short checksum;

  bool isV3MembershipReport() const { return type==0x22; }
};

class IGMPGroupRecord {
public:
  enum { MODE_IS_INCLUDE = 1,
	 MODE_IS_EXCLUDE = 2,
	 CHANGE_TO_INCLUDE_MODE = 3,
	 CHANGE_TO_EXCLUDE_MODE = 4,
	 ALLOW_NEW_SOURCES = 5,
	 BLOCK_OLD_SOURCES = 6 };
  unsigned char    type;
  unsigned char    auxlen;
  unsigned short   numberOfSources;
  unsigned         mcastAddress;
  unsigned         sourceAddr(unsigned i) const
  { return *reinterpret_cast<const unsigned*>(reinterpret_cast<const unsigned*>(this+1)+i); }

  const IGMPGroupRecord* next() const 
  { return reinterpret_cast<const IGMPGroupRecord*>(reinterpret_cast<const unsigned*>(this+1)+numberOfSources); }
};

class IGMPv3Report : public IGMPMessage {
public:
  unsigned short   reserved;
  unsigned short   numberOfGroupRecords;
  IGMPGroupRecord  groupRecords;
};

#endif
