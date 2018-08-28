// -*-Mode: C++;-*-
/**
@file
@brief Document Linux-specific parts of class Once.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

/**
   @def PTH_CALL
   @brief Call a Pth function, examine the return code and throw Error if needed.
 */

namespace tool {

  namespace concurrency {

    /** @brief For implementation that must appear in Once headers. */
    namespace Once_private {

      /**
         @struct ArgRec
         @brief An adapter used by function onearg() to call a no-argument
         function.
         @details Once::run() calls a no-argument function but a pth_once object
         must refer to a one-argument function. This struct serves
         as the required adapter along with the onearg() function.

         @var ArgRec::func
         @brief Holds a pointer to a no-argument function.

         @fn onearg(void* arg)
         @brief This function is called by the pth_once object.
         @param[in] arg This pointer is cast to an ArgRec* and
         the function pointer to by the ArgRec is called.
       */
    }
  }

}
