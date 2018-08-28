#ifndef ChrDisplay_hh
#define ChrDisplay_hh

namespace cm {
  namespace svc {
    class ChannelMap;
  }
}


class ChrDisplay {
public:
  enum Transform { None, Rotate90, Rotate180, Rotate270, HFlip, VFlip };

  void write    (unsigned ,   Transform=None);
  void writeHex (unsigned ,   Transform=None);
  void writeBmp (unsigned*,   Transform=None);
  void writeStr (const char*, Transform=None);

  unsigned charToBmp(char);
  unsigned transform(unsigned, Transform=None);
public:
  static ChrDisplay* instance();

private:
  ChrDisplay();
  ~ChrDisplay();
private:
  cm::svc::ChannelMap* _rotate90;
  cm::svc::ChannelMap* _rotate180;
  cm::svc::ChannelMap* _rotate270;
  cm::svc::ChannelMap* _hflip ;
  cm::svc::ChannelMap* _vflip ;
};

#endif
