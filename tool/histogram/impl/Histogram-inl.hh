/*
** ++
**  Package:
**	Tool
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC, (650) 926-4269
**
**  Creation Date:
**	000 - October 27,1999
**
**  Revision History:
**	None.
**
** --
*/

namespace tool {
  namespace histogram {

/*
** ++
**
**
** --
*/

inline double Histogram::units() const
  {
  return _binWidth;
  }

/*
** ++
**
**
** --
*/

inline double Histogram::lowerBound() const
  {
  return _lowerBound * _binWidth;
  }

/*
** ++
**
**
** --
*/

inline double Histogram::upperBound() const
  {
    return (_lowerBound + _size) * _binWidth;
  }

/*
** ++
**
**
** --
*/

inline double Histogram::counts() const
  {
  return _totalCounts;
  }

/*
** ++
**
**
** --
*/

inline double Histogram::weight() const
  {
  return _totalWeight;
  }

/*
** ++
**
**
** --
*/

inline unsigned Histogram::underflows() const
  {
  return _buffer[_size + 1];
  }

/*
** ++
**
**
** --
*/

inline unsigned Histogram::overflows() const
  {
  return _buffer[_size];
  }

/*
** ++
**
**    Accumulate statistics by bin index
**
**    @param index Bin index, i.e.: (datum - offset) / _binWidth
**
** --
*/

inline void Histogram::bump(int index)
  {
  unsigned* buffer = _buffer;
  unsigned  size   = _size;

  if (index < 0)          buffer[size + 1]++; // Underflows
  if (index < (int)size)  buffer[index   ]++;
  else                    buffer[size    ]++; // Overflows
  }

/*
** ++
**
**    Accumulate statistics by data value
**
** --
*/

inline void Histogram::bump(double datum)
  {
  bump(int(((datum - _lowerBound) / _binWidth)));
  }


  }; // tool
}; // histogram
