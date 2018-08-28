#ifndef ChannelMap_hh
#define ChannelMap_hh

namespace cm {
  namespace svc {
    class ChannelMap {
    public:
      ChannelMap(unsigned rangeA,
                 unsigned rangeB);
      ~ChannelMap();

      unsigned a(unsigned b) const { return _a[b]; }
      unsigned b(unsigned a) const { return _b[a]; }

      unsigned aMask(unsigned bMask) const;
      unsigned bMask(unsigned aMask) const;

      void add(unsigned aChan, unsigned bChan);
    private:
      unsigned* _a;
      unsigned* _b;
    };
  }
}

#endif
