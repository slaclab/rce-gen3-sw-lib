#include "fmb/FmStatsCfg.hh"

using namespace Bali;

FmStatsCfg& FmStatsCfg::operator=(const unsigned v)
{
  _data[0] = v;
  return *this;
}

FmStatsCfg::operator unsigned() const
{
  return _data[0];
}
