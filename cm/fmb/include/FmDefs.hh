#ifndef Bali_FmDefs_hh
#define Bali_FmDefs_hh

//
//  Helper macros for the crazy register structure
//

#define ADD_STAT_PORTCOUNTERS                           \
  ADD_STAT(0, RxUcstPktsNonIp, RxBcstPktsNonIp);        \
  ADD_STAT(1, RxMcstPktsNonIp, RxUcstPktsIPv4);         \
  ADD_STAT(2, RxBcstPktsIPv4,  RxMcstPktsIPv4);         \
  ADD_STAT(3, RxUcstPktsIPv6,  RxBcstPktsIPv6);         \
  ADD_STAT(4, RxMcstPktsIPv6,  RxPausePkts);            \
  ADD_STAT(5, RxCBPausePkts,   RxSymbolErrors);         \
  ADD_STAT(6, RxFcsErrors,     RxFrameSizeErrors);      \
  ADD_STAT(7, RxP0, RxP1);                              \
  ADD_STAT(8, RxP2, RxP3);                              \
  ADD_STAT(9, RxP4, RxP5);                              \
  ADD_STAT(A, RxP6, RxP7);                              \
  ADD_STAT(B, TxUcstPkts, TxBcstPkts);                  \
  ADD_STAT(C, TxMcstPkts, TxTimeoutDrop);               \
  ADD_STAT(D, TxErrorDrop, TxLoopbackDrop);             \
  ADD_RSRV(90620,0x91000-0x90620);                      \
                                                        \
  ADD_STAT(E, RxMinto63,     Rx64Pkts);                 \
  ADD_STAT(F, Rx65to127,     Rx128to255);               \
  ADD_STAT(G, Rx256to511,    Rx512to1023);              \
  ADD_STAT(H, Rx1024to1522,  Rx1523to2047);             \
  ADD_STAT(I, Rx2048to4095,  Rx4096to8191);             \
  ADD_STAT(J, Rx8192to10239, Rx10240toMax);             \
  ADD_STAT(K, RxFragments,   RxUndersized);             \
  ADD_STAT(L, RxOversized,   ReservedL);                \
  ADD_STAT(M, RxOctetsNonIP, RxOctetsIPv4);             \
  ADD_STAT(N, RxOctetsIPv6,  RxOctetsError);            \
  ADD_STAT(O, TxMinto63,     Tx64Pkts);                 \
  ADD_STAT(P, Tx65to127,     Tx128to255);               \
  ADD_STAT(Q, Tx256to511,    Tx512to1023);              \
  ADD_STAT(R, Tx1024to1522,  Tx1523to2047);             \
  ADD_STAT(S, Tx2048to4095,  Tx4096to8191);             \
  ADD_STAT(T, Tx8192to10239, Tx10240toMax);             \
  ADD_STAT(U, TxErrors,      ReservedU);                \
  ADD_RSRV(91770,0x92000-0x91770);                      \
                                                        \
  ADD_STAT(V, FIDForwarded, FloodForwarded);            \
  ADD_STAT(W, SpeciallyHandled, ParseErrDrops);         \
  ADD_STAT(X, ParityError, Trapped);                    \
  ADD_STAT(Y, PauseDrops, STPDrops);                    \
  ADD_STAT(Z, SecurityViolations, VlanTagDrops);        \
  ADD_STAT(a, VlanIngressDrops, VlanEgressDrops);       \
  ADD_STAT(b, GlortMissDrops, FFUDrops);                \
  ADD_STAT(c, TriggerDrops, PolicerDrops);              \
  ADD_STAT(d, TTLDrops, CMPrivDrops);                   \
  ADD_STAT(e, SMP0Drops, SMP1Drops);                    \
  ADD_STAT(f, RxHog0Drops, RxHog1Drops);                \
  ADD_STAT(g, TxHog0Drops, TxHog1Drops);                \
  ADD_STAT(h, RateLimit0Drops, RateLimit1Drops);        \
  ADD_STAT(i, BadSMPDrops, TriggerRedirects);           \
  ADD_STAT(j, Others, Reservedj);                       \
  ADD_RSRV(92690,0x92700-0x92690);                      \
                                                        \
  ADD_STAT(k, RxOctetsP0, RxOctetsP1);                  \
  ADD_STAT(l, RxOctetsP2, RxOctetsP3);                  \
  ADD_STAT(m, RxOctetsP4, RxOctetsP5);          \
  ADD_STAT(n, RxOctetsP6, RxOctetsP7);          \
  ADD_STAT(o, TxOctets, TxErrorOctets);         \
  ADD_RSRV(92930,0xc0000-0x92930);

#endif
