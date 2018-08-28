
#include "control/ChrDisplay.hh"

#include "cm_svc/ChannelMap.hh"

static const unsigned _b_____ = 0;
static const unsigned _b____X = 1;
static const unsigned _b___X_ = 2;
static const unsigned _b___XX = 3;
static const unsigned _b__X__ = 4;
static const unsigned _b__X_X = 5;
static const unsigned _b__XX_ = 6;
static const unsigned _b__XXX = 7;
static const unsigned _b_X___ = 8;
static const unsigned _b_X__X = 9;
static const unsigned _b_X_X_ = 10;
static const unsigned _b_X_XX = 11;
static const unsigned _b_XX__ = 12;
static const unsigned _b_XX_X = 13;
static const unsigned _b_XXX_ = 14;
static const unsigned _b_XXXX = 15;
static const unsigned _bX____ = 16;
static const unsigned _bX___X = 17;
static const unsigned _bX__X_ = 18;
static const unsigned _bX__XX = 19;
static const unsigned _bX_X__ = 20;
static const unsigned _bX_X_X = 21;
static const unsigned _bX_XX_ = 22;
static const unsigned _bX_XXX = 23;
static const unsigned _bXX___ = 24;
static const unsigned _bXX__X = 25;
static const unsigned _bXX_X_ = 26;
static const unsigned _bXX_XX = 27;
static const unsigned _bXXX__ = 28;
static const unsigned _bXXX_X = 29;
static const unsigned _bXXXX_ = 30;
static const unsigned _bXXXXX = 31;

static unsigned _hexchar[] = { (_b_XXX_ <<  0) |
			       (_bX__XX <<  5) |
			       (_bX_X_X << 10) |
			       (_bXX__X << 15) |
			       (_b_XXX_ << 20),

			       (_b__X__ <<  0) |
			       (_b_XX__ <<  5) |
			       (_b__X__ << 10) |
			       (_b__X__ << 15) |
			       (_b_XXX_ << 20),

			       (_b_XXXX <<  0) |
			       (_b____X <<  5) |
			       (_b_XXXX << 10) |
			       (_b_X___ << 15) |
			       (_b_XXXX << 20),

			       (_b_XXXX <<  0) |
			       (_b____X <<  5) |
			       (_b__XXX << 10) |
			       (_b____X << 15) |
			       (_b_XXXX << 20),

			       (_b__XX_ <<  0) |
			       (_b_X_X_ <<  5) |
			       (_bXXXXX << 10) |
			       (_b___X_ << 15) |
			       (_b___X_ << 20),

			       (_bXXXXX <<  0) |
			       (_bX____ <<  5) |
			       (_bXXXX_ << 10) |
			       (_b____X << 15) |
			       (_bXXXX_ << 20),

			       (_b__X__ <<  0) |
			       (_b_X___ <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX___X << 15) |
			       (_b_XXX_ << 20),

			       (_bXXXXX <<  0) |
			       (_b___X_ <<  5) |
			       (_b__X__ << 10) |
			       (_b_X___ << 15) |
			       (_b_X___ << 20),

			       (_b_XXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_b_XXX_ << 10) |
			       (_bX___X << 15) |
			       (_b_XXX_ << 20),

			       (_b_XXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_b_XXXX << 10) |
			       (_b___X_ << 15) |
			       (_b__X__ << 20),

			       (_b__X__ <<  0) |
			       (_b_X_X_ <<  5) |
			       (_bXXXXX << 10) |
			       (_bX___X << 15) |
			       (_bX___X << 20),

			       (_bXXXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX___X << 15) |
			       (_bXXXX_ << 20),

			       (_b_XXXX <<  0) |
			       (_bX____ <<  5) |
			       (_bX____ << 10) |
			       (_bX____ << 15) |
			       (_b_XXXX << 20),

			       (_bXXXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bX___X << 10) |
			       (_bX___X << 15) |
			       (_bXXXX_ << 20),

			       (_bXXXXX <<  0) |
			       (_bX____ <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX____ << 15) |
			       (_bXXXXX << 20),

			       (_bXXXXX <<  0) |
			       (_bX____ <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX____ << 15) |
			       (_bX____ << 20),

			       (_b_XXX_ <<  0) |
			       (_bX____ <<  5) |
			       (_bX_XXX << 10) |
			       (_bX___X << 15) |
			       (_b_XXX_ << 20),

			       (_bX___X <<  0) |
			       (_bX___X <<  5) |
			       (_bXXXXX << 10) |
			       (_bX___X << 15) |
			       (_bX___X << 20),

			       (_b_XXX_ <<  0) |
			       (_b__X__ <<  5) |
			       (_b__X__ << 10) |
			       (_b__X__ << 15) |
			       (_b_XXX_ << 20),

			       (_b_XXXX <<  0) |
			       (_b___X_ <<  5) |
			       (_b___X_ << 10) |
			       (_b_X_X_ << 15) |
			       (_b_XX__ << 20),

			       (_bX___X <<  0) |
			       (_bX__X_ <<  5) |
			       (_bXXX__ << 10) |
			       (_bX__X_ << 15) |
			       (_bX___X << 20),

			       (_bX____ <<  0) |
			       (_bX____ <<  5) |
			       (_bX____ << 10) |
			       (_bX____ << 15) |
			       (_bXXXXX << 20),

			       (_bX___X <<  0) |
			       (_bXX_XX <<  5) |
			       (_bX_X_X << 10) |
			       (_bX___X << 15) |
			       (_bX___X << 20),

			       (_bX___X <<  0) |
			       (_bXX__X <<  5) |
			       (_bX_X_X << 10) |
			       (_bX__XX << 15) |
			       (_bX___X << 20),

			       (_b_XXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bX___X << 10) |
			       (_bX___X << 15) |
			       (_b_XXX_ << 20),

			       (_bXXXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX____ << 15) |
			       (_bX____ << 20),

			       (_b_XXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bX___X << 10) |
			       (_bX_X_X << 15) |
			       (_b_XXXX << 20),

			       (_bXXXX_ <<  0) |
			       (_bX___X <<  5) |
			       (_bXXXX_ << 10) |
			       (_bX__X_ << 15) |
			       (_bX___X << 20),

			       (_b_XXXX <<  0) |
			       (_bX____ <<  5) |
			       (_b_XXX_ << 10) |
			       (_b____X << 15) |
			       (_bXXXX_ << 20),

			       (_bXXXXX <<  0) |
			       (_b__X__ <<  5) |
			       (_b__X__ << 10) |
			       (_b__X__ << 15) |
			       (_b__X__ << 20),

			       (_bX___X <<  0) |
			       (_bX___X <<  5) |
			       (_bX___X << 10) |
			       (_bX___X << 15) |
			       (_b_XXX_ << 20),

			       (_bX___X <<  0) |
			       (_bX___X <<  5) |
			       (_bX___X << 10) |
			       (_b_X_X_ << 15) |
			       (_b__X__ << 20),

			       (_bX___X <<  0) |
			       (_bX___X <<  5) |
			       (_bX_X_X << 10) |
			       (_bX_X_X << 15) |
			       (_b_X_X_ << 20),

			       (_bX___X <<  0) |
			       (_b_X_X_ <<  5) |
			       (_b__X__ << 10) |
			       (_b_X_X_ << 15) |
			       (_bX___X << 20),

			       (_bX___X <<  0) |
			       (_b_X_X_ <<  5) |
			       (_b__X__ << 10) |
			       (_b__X__ << 15) |
			       (_b__X__ << 20),

			       (_bXXXXX <<  0) |
			       (_b___X_ <<  5) |
			       (_b__X__ << 10) |
			       (_b_X___ << 15) |
			       (_bXXXXX << 20)

};

static inline void write_display(unsigned v)
{
  asm volatile("mtdcr %0,%1" : : "i"(0x2f7), "r"(v));
}


unsigned ChrDisplay::transform(unsigned v, Transform t)
{
  cm::svc::ChannelMap* map = 0;
  switch(t) {
  case Rotate90 : map = _rotate90 ; break;
  case Rotate180: map = _rotate180; break;
  case Rotate270: map = _rotate270; break;
  case HFlip    : map = _hflip    ; break;
  case VFlip    : map = _vflip    ; break;
  default: break;
  }
  return map ? (map->bMask(v) | (v&0x30000000)) : v;
}

void ChrDisplay::write(unsigned v, Transform t)
{
  write_display( transform(v,t) );
}

void ChrDisplay::writeHex(unsigned v, Transform t)
{
  for(unsigned k=0; k<8; k++) {
    write(_hexchar[v&0xf] | (k<<28),t);
    v >>= 4;
  }
}

void ChrDisplay::writeBmp(unsigned* v, Transform t)
{
  for(unsigned k=0; k<4; k++)
    write((v[k]&0x1ffffff) | (k<<28),t);
}

void ChrDisplay::writeStr(const char* s, Transform t)
{
  unsigned bmp[4];
  for(unsigned k=0; k<4; k++) {
    bmp[k^3] = charToBmp(s[k]);
  }
  writeBmp(bmp,t);
}

unsigned ChrDisplay::charToBmp(char s)
{
  unsigned bmp;
  if      (s>='0' && s<='9')
    bmp = _hexchar[s-'0'];
  else if (s>='A' && s<='Z')
    bmp = _hexchar[s-'A'+10];
  else if (s>='a' && s<='z')
    bmp = _hexchar[s-'a'+10];
  else
    bmp = 0;
  return bmp;
}

static ChrDisplay* _instance = 0;

ChrDisplay* ChrDisplay::instance()
{
  if (!_instance)
    _instance = new ChrDisplay;

  return _instance;
}


ChrDisplay::ChrDisplay() :
  _rotate90 (new cm::svc::ChannelMap(25,25)),
  _rotate180(new cm::svc::ChannelMap(25,25)),
  _rotate270(new cm::svc::ChannelMap(25,25)),
  _hflip    (new cm::svc::ChannelMap(25,25)),
  _vflip    (new cm::svc::ChannelMap(25,25))
{
  for(unsigned k=0; k<25; k++) {
    unsigned row = k/5;
    unsigned col = k%5;
    _rotate90 ->add(k,(4-col)*5+row);
    _rotate180->add(k,24-k);
    _rotate270->add(k,col*5+4-row);
    _hflip    ->add(k,row*5+4-col);
    _vflip    ->add(k,(4-row)*5+col);
  }
}

ChrDisplay::~ChrDisplay()
{
  delete _rotate90 ;
  delete _rotate180;
  delete _rotate270;
  delete _hflip    ;
  delete _vflip    ;
}

/*
** fast hflip
**
    bmp[k] |= (w&(0x108421<<0))<<4;
    bmp[k] |= (w&(0x108421<<1))<<2;
    bmp[k] |= (w&(0x108421<<2))<<0;
    bmp[k] |= (w&(0x108421<<3))>>2;
    bmp[k] |= (w&(0x108421<<4))>>4;
 */

/*
** fast vflip
**
    bmp[k] |= (w&(0x00001F<< 0))<<20;
    bmp[k] |= (w&(0x00001F<< 5))<<10;
    bmp[k] |= (w&(0x00001F<<10))<<0;
    bmp[k] |= (w&(0x00001F<<15))>>10;
    bmp[k] |= (w&(0x00001F<<20))>>20;
*/
