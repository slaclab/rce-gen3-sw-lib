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

#ifndef TOOL_HISTOGRAM_HISTOGRAM_HH
#define TOOL_HISTOGRAM_HISTOGRAM_HH

namespace tool {
  namespace histogram {

class Histogram
  {
    // Create an in-memory, constant-width binned histogram
    //
    // @param title:        Title of the histogram and units, separated by a ';'
    // @param nBins:        Log 2 of the number of bins in the histogram
    //                      (including under and overflow)
    // @param binWidth:     Width of histogram bins
    // @param lowerBound:   Offset from zero of the first bin (natural units)
    //
    // bins:  underflow:  (-inf, lowerBound),
    //                    [           lowerBound,   binWidth + lowerBound),
    //                    [binWidth + lowerBound, 2*binWidth + lowerBound),
    //                    ...
    //        overflow:   [nBins * binWidth + lowerBound, +inf)
    //

  public:
    // The title has to be word aligned when the EOL character is included in REAL_TITLE_LENGTH.
    enum { TITLE_LENGTH = 22 * sizeof(unsigned) - 1};
    enum { MAX_HEIGHT = 80};            // Scale to the usual screen width

  public:
    Histogram(char const* title, unsigned size, double unitsCvt, double lowerBound);
   ~Histogram();
  public:
    void        sum();
    void        dump(char* filesSpec);
    void        print();
    char const* title()      const;
    double      lowerBound() const;
    double      upperBound() const;
    double      units()      const;
    double      weight()     const;
    double      counts()     const;
    unsigned    underflows() const;
    unsigned    overflows()  const;
    void        bump(int index);
    void        bump(double data);
    void        reset();
  private:
    unsigned* _buffer;                  // Histogram buffer
    unsigned  _size;                    // Number of entries = upper bound
    double    _lowerBound;              // Lowest bin offset from zero
    double    _totalCounts;             // # of times histogram incrmented
    double    _totalWeight;             // # of times histogram incrmented
    double    _binWidth;                // Natural units conversion constant
    char      _title[TITLE_LENGTH + 1]; // Histogram title (+1 for '\0')

  };

  }; // tool
}; // histogram


#include "histogram/impl/Histogram-inl.hh"

#endif // TOOL_HISTOGRAM_HISTOGRAM_HH
