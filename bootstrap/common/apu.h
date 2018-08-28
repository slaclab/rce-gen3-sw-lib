// -*-Mode: C;-*-
/*!@file     apu.h
*
* @brief     udiNfcm instruction instantiation
*
*            This code creates a bunch of inline functions to provide access
*            to the udiNfcm instructions from C.  The functions are named
*            APU_UDI<N>, where N is a value from 0 to 15, inclusive.
*
*            A udifcm instructions has either read or write semantics, thus
*            requiring two different interfaces.  In addition, the read form
*            is expected to set the Condition Register.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 15, 2012 -- Created
*
* $Revision: 1384 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _APU_H_
#define _APU_H_

typedef struct
{
  unsigned long v[4];
} uint128_t;


extern inline void APU_Enable()
{
  register unsigned ccr0;
  __asm__ volatile("mfccr0 %0": "=r"(ccr0));
  ccr0 &= ~0x00100000;                  /* DBAPU */
  __asm__ volatile("eieio     \n\t"
                   "mtccr0 %0 \n\t"
                   "isync     \n\t"
                   :
                   : "r"(ccr0)
                   );
}


#define UDIFCM_RD(N, t, a, b)                                           \
  extern inline __attribute__((__always_inline__))                      \
  unsigned APU_udi##N##fcm(unsigned*      t,                            \
                           const unsigned a,                            \
                           const unsigned b)                            \
  {                                                                     \
    register unsigned cc;                                               \
    __asm__ volatile("udi%[n]fcm. %[T],%[A],%[B] \n\t"                  \
                     "mfcr        %[CC]          \n\t"                  \
                     : [T]"=r"(*t), [CC]"=r"(cc)                        \
                     : [A]"r"(a), [B]"r"(b), [n]"i"(N)                  \
                     : "cc", "memory"                                   \
                     );                                                 \
    return cc;                                                          \
  }                                                                     \
                                                                        \
  extern inline __attribute__((__always_inline__))                      \
  void APU_udi##N##fcmW(unsigned*      t,                               \
                        const unsigned a,                               \
                        const unsigned b)                               \
  {                                                                     \
    __asm__ volatile("1: udi%[n]fcm. %[T],%[A],%[B] \n\t"               \
                     "   bt          3,1b           \n\t"               \
                     : [T]"=r"(*t)                                      \
                     : [A]"r"(a), [B]"r"(b), [n]"i"(N)                  \
                     : "cc", "memory"                                   \
                     );                                                 \
  }

#define UDIFCM_WR(N, t, a, b)                                           \
  extern inline __attribute__((__always_inline__))                      \
  void APU_udi##N##fcm(const unsigned t,                                \
                       const unsigned a,                                \
                       const unsigned b)                                \
  {                                                                     \
    __asm__ volatile("udi%[n]fcm %[T],%[A],%[B]  \n\t"                  \
                     :                                                  \
                     : [T]"r"(t), [A]"r"(a), [B]"r"(b), [n]"i"(N)       \
                     : "memory"                                         \
                     );                                                 \
  }

UDIFCM_RD( 0, t, a, b)
UDIFCM_WR( 1, t, a, b)
UDIFCM_RD( 2, t, a, b)
UDIFCM_WR( 3, t, a, b)
UDIFCM_RD( 4, t, a, b)
UDIFCM_WR( 5, t, a, b)
UDIFCM_RD( 6, t, a, b)
UDIFCM_WR( 7, t, a, b)
UDIFCM_RD( 8, t, a, b)
UDIFCM_WR( 9, t, a, b)
UDIFCM_RD(10, t, a, b)
UDIFCM_WR(11, t, a, b)
UDIFCM_RD(12, t, a, b)
UDIFCM_WR(13, t, a, b)
UDIFCM_RD(14, t, a, b)
UDIFCM_WR(15, t, a, b)

#undef UDIFCM_RD
#undef UDIFCM_WR

#define APU_INSN(insn, type, T, a, b)                         \
  extern inline __attribute__((__always_inline__))            \
  void APU_##insn##_##T(type*          a,                     \
                        const unsigned b)                     \
  {                                                           \
    __asm__ volatile(#insn" %[t],%[A],%[B]"                   \
                     :                                        \
                     : [t]"i"(T), [A]"b"(*a), [B]"r"(b)       \
                     : "memory"                               \
                     );                                       \
  }

#define APU_INSN_U(insn, type, T, a, b)                       \
  extern inline __attribute__((__always_inline__))            \
  void APU_##insn##_##T(type*          a,                     \
                        const unsigned b)                     \
  {                                                           \
    __asm__ volatile(#insn" %[t],%[A],%[B]"                   \
                     : [A]"+b"(*a)                            \
                     : [t]"i"(T), [B]"r"(b)                   \
                     : "memory"                               \
                     );                                       \
  }

#define APU_FCM(insn, type, a, b)               \
  APU_INSN(insn, type,  0, a, b)                \
  APU_INSN(insn, type,  1, a, b)                \
  APU_INSN(insn, type,  2, a, b)                \
  APU_INSN(insn, type,  3, a, b)                \
  APU_INSN(insn, type,  4, a, b)                \
  APU_INSN(insn, type,  5, a, b)                \
  APU_INSN(insn, type,  6, a, b)                \
  APU_INSN(insn, type,  7, a, b)                \
  APU_INSN(insn, type,  8, a, b)                \
  APU_INSN(insn, type,  9, a, b)                \
  APU_INSN(insn, type, 10, a, b)                \
  APU_INSN(insn, type, 11, a, b)                \
  APU_INSN(insn, type, 12, a, b)                \
  APU_INSN(insn, type, 13, a, b)                \
  APU_INSN(insn, type, 14, a, b)                \
  APU_INSN(insn, type, 15, a, b)                \
  APU_INSN(insn, type, 16, a, b)                \
  APU_INSN(insn, type, 17, a, b)                \
  APU_INSN(insn, type, 18, a, b)                \
  APU_INSN(insn, type, 19, a, b)                \
  APU_INSN(insn, type, 20, a, b)                \
  APU_INSN(insn, type, 21, a, b)                \
  APU_INSN(insn, type, 22, a, b)                \
  APU_INSN(insn, type, 23, a, b)                \
  APU_INSN(insn, type, 24, a, b)                \
  APU_INSN(insn, type, 25, a, b)                \
  APU_INSN(insn, type, 26, a, b)                \
  APU_INSN(insn, type, 27, a, b)                \
  APU_INSN(insn, type, 28, a, b)                \
  APU_INSN(insn, type, 29, a, b)                \
  APU_INSN(insn, type, 30, a, b)                \
  APU_INSN(insn, type, 31, a, b)

#define APU_FCM_U(insn, type, a, b)             \
  APU_INSN_U(insn, type,  0, a, b)              \
  APU_INSN_U(insn, type,  1, a, b)              \
  APU_INSN_U(insn, type,  2, a, b)              \
  APU_INSN_U(insn, type,  3, a, b)              \
  APU_INSN_U(insn, type,  4, a, b)              \
  APU_INSN_U(insn, type,  5, a, b)              \
  APU_INSN_U(insn, type,  6, a, b)              \
  APU_INSN_U(insn, type,  7, a, b)              \
  APU_INSN_U(insn, type,  8, a, b)              \
  APU_INSN_U(insn, type,  9, a, b)              \
  APU_INSN_U(insn, type, 10, a, b)              \
  APU_INSN_U(insn, type, 11, a, b)              \
  APU_INSN_U(insn, type, 12, a, b)              \
  APU_INSN_U(insn, type, 13, a, b)              \
  APU_INSN_U(insn, type, 14, a, b)              \
  APU_INSN_U(insn, type, 15, a, b)              \
  APU_INSN_U(insn, type, 16, a, b)              \
  APU_INSN_U(insn, type, 17, a, b)              \
  APU_INSN_U(insn, type, 18, a, b)              \
  APU_INSN_U(insn, type, 19, a, b)              \
  APU_INSN_U(insn, type, 20, a, b)              \
  APU_INSN_U(insn, type, 21, a, b)              \
  APU_INSN_U(insn, type, 22, a, b)              \
  APU_INSN_U(insn, type, 23, a, b)              \
  APU_INSN_U(insn, type, 24, a, b)              \
  APU_INSN_U(insn, type, 25, a, b)              \
  APU_INSN_U(insn, type, 26, a, b)              \
  APU_INSN_U(insn, type, 27, a, b)              \
  APU_INSN_U(insn, type, 28, a, b)              \
  APU_INSN_U(insn, type, 29, a, b)              \
  APU_INSN_U(insn, type, 30, a, b)              \
  APU_INSN_U(insn, type, 31, a, b)

APU_FCM(lbfcmx, unsigned char*,        a, b)
APU_FCM(lhfcmx, unsigned short*,       a, b)
APU_FCM(lwfcmx, unsigned long*,        a, b)
APU_FCM(lqfcmx, uint128_t*,            a, b)
APU_FCM(ldfcmx, unsigned long long*,   a, b)

APU_FCM(stbfcmx, unsigned char*,       a, b)
APU_FCM(sthfcmx, unsigned short*,      a, b)
APU_FCM(stwfcmx, unsigned long*,       a, b)
APU_FCM(stqfcmx, uint128_t*,           a, b)
APU_FCM(stdfcmx, unsigned long long*,  a, b)

APU_FCM_U(lbfcmux, unsigned char*,       a, b)
APU_FCM_U(lhfcmux, unsigned short*,      a, b)
APU_FCM_U(lwfcmux, unsigned long*,       a, b)
APU_FCM_U(lqfcmux, uint128_t*,           a, b)
APU_FCM_U(ldfcmux, unsigned long long*,  a, b)

APU_FCM_U(stbfcmux, unsigned char*,      a, b)
APU_FCM_U(sthfcmux, unsigned short*,     a, b)
APU_FCM_U(stwfcmux, unsigned long*,      a, b)
APU_FCM_U(stqfcmux, uint128_t*,          a, b)
APU_FCM_U(stdfcmux, unsigned long long*, a, b)

#undef APU_INSN
#undef APU_FCM
#undef APU_INSN_U
#undef APU_FCM_U

#endif
