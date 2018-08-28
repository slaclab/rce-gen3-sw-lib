/*
** ++
**  Package:
**	Tool
**
**  Abstract:
**	Non-inline functions for "Histogram.hh"
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - June 1,1998
**
**  Revision History:
**	None.
**
** --
*/

#include <stdio.h>
#include <string.h>


#include "histogram/Histogram.hh"
using namespace tool::histogram;


/*
** ++
**
**   constructor... initializes all bins to zero.
**
**   @param title A title for the histogram
**   @param nBins Number of bins in the histogram
**   @param binWidth Conversion constant to convert bin # to natural units
**   @param lowerBound Offset of the histogram from zero, in natural units
**
** --
*/

Histogram::Histogram(const char* title,
                     unsigned    nBins,
                     double      binWidth,
                     double      lowerBound)
  {
  _buffer     = new unsigned[nBins + 2];             // +2 for under/overflow
  _size       = nBins;
  _lowerBound = lowerBound;
  _binWidth   = binWidth;
  strncpy(_title, title, TITLE_LENGTH);
  _title[TITLE_LENGTH] = '\0';

  if (_buffer == 0) {
    printf("%s: No memory found for histogram buffer of %u bins\n", __func__,
           nBins);
    return;
  }

  reset();
  }


/*
** ++
**
**
** --
*/

void Histogram::reset()
  {
  unsigned  remaining = _size + 2;      // Include under/overflow bin
  unsigned* next      = _buffer;

  do *next++ = 0; while(remaining--);

  _totalWeight = 0.0;
  _totalCounts = 0.0;
  }

/*
** ++
**
**
** --
*/

void Histogram::sum()
  {
  unsigned  remaining   = _size;        // Exclude under/overflow bin
  unsigned* next        = &_buffer[remaining];
  double    totalWeight = 0;
  double    totalCounts = 0;

  remaining--;

  do
    {
    double counts = double(*--next);
    totalWeight += counts * double(remaining);
    totalCounts += counts;
    }
  while(remaining--);

  _totalWeight = totalWeight;
  _totalCounts = totalCounts;
  }

/*
** ++
**
**
** --
*/

void Histogram::dump(char* fileSpec)
  {
  unsigned  remaining  = _size + 1;
  unsigned* next       = &_buffer[remaining];
  unsigned  lowerBound = _lowerBound - 1; // Compensate for underflow bin

  remaining--;

  FILE* file = fopen (fileSpec, "w");
  if (!file)
    {
    printf ("%s: Couldn't open file %s for write\n", __func__, fileSpec);
    return;
    }

  do
    {
    unsigned counts = *--next;
    if (counts)
      {
      fprintf (file, "%f %u\n", (lowerBound + remaining) * _binWidth, counts);
      }
    }
  while (--remaining);
  unsigned counts = _buffer[_size + 1];
  if (counts)
    {
    fprintf (file, "%f %u\n", (lowerBound + _size + 1) * _binWidth, counts);
    }

  if (fclose(file) == -1)
    {
    printf ("%s: File %s didn't properly close\n", __func__, fileSpec);
    }
  }

/*
** ++
**
**
** --
*/

void Histogram::print()
  {
  // Dumb way of creating a string of length MAX_HEIGHT, with a null terminator
  unsigned const width = MAX_HEIGHT;
  char str[width + 1];
  memset(str, '*', width);
  str[width] = '\0';

  // Find the maximum bin of the histogram.  If this bin is
  // greater than the width of the above string, set a scale factor
  unsigned histoMax = 0;
  unsigned idx;
  double   scale    = 1.0;
  for (idx = 0; idx < _size; idx++)
    if (_buffer[idx] > histoMax)  histoMax = _buffer[idx];

  // JHP:  If we want to include the bins for under and overflow
  // in the scale factor, comment these two lines back in.
  //if (_buffer[_size + 1] > histoMax)  histoMax = _buffer[_size + 1]; // Underflows
  //if (_buffer[_size    ] > histoMax)  histoMax = _buffer[_size    ]; // Overflows
  if (histoMax > width)  scale = double(width) / double(histoMax);

  // Calculate the number of entries and weight of the histogram.
  sum();

  // Print the histogram.  Each line shows the edges of the
  // bin, followed by the decimal bin contents, followed by
  // an ascii bin picture (except for under/overflow, where)
  // the latter is skipped.  ( or ) means exclusive, while
  // [ or ] means inclusive.
  printf("Histogram \"%s\" - Total counts: %f, weight = %f\n",
         _title, counts(), weight());
  unsigned lowerBound = _lowerBound;
  printf("(    -INF, %8.3g):%10u %.*s\n",
         (  lowerBound                   ) * _binWidth,
                       _buffer[_size + 1],
         int(scale *   _buffer[_size + 1]), str);
  for (idx = 0; idx < _size; idx++) {
    printf("[%8.3g, %8.3g):%10u %.*s\n",
           (lowerBound +       idx       ) * _binWidth,
           (lowerBound +      (idx + 1)  ) * _binWidth,
                       _buffer[idx      ],
           int(scale * _buffer[idx      ]), str);
  }
  printf("[%8.3g,     +INF):%10u %.*s\n",
         (  lowerBound +       _size     ) * _binWidth,
                       _buffer[_size    ],
         int(scale *   _buffer[_size    ]), str);
  }

/*
** ++
**
**
** --
*/

Histogram::~Histogram()
  {
  if (_buffer)  delete[] _buffer;
  }
