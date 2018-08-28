# 
	.text
	.align	2
	.global	mem_rsetAlloc
        .type   mem_rsetAlloc,%function
mem_rsetAlloc:
        push    {r4}
1:
	mov     ip,#0x80000000  @ Used to make the bic mask
        ldrexd  r2,r3,[r0]      @ Load the bitset
        clz     r4,r2           @ Search the low word for a 1-bit
        clz     r1,r3           @ Search the high word for a 1-bit
        cmp     r4,#32          @ Bit found in low word?
        bicne   r2,r2,ip,lsr r4 @ Clear bit found (if any) in low word
        movne   r1,#32          @ Inhibit bit clear in high word if low search succeeded
        cmp     r1,#32          @ Bit found in high word?
        bicne   r3,r3,ip,lsr r1 @ Clear bit found (if any) in high word
        strexd  ip,r2,r3,[r0]   @ Save the altered bitset
        cmp     ip,#0
        bne     1b              @ Retry if the save failed

        # We don't need a memorybarrier at this point because the resource
        # address depends on calculations made inside the loop. Any
        # speculative results are discarded if the branch is taken.

        cmp     r4,#32          @ Low search failed?
        addeq   r4,r4,r1        @ If so, position is 32 + high search position
        cmp     r4,#64          @ Both searches failed (both 32)?

# If the combined search failed then the function result is a null pointer
        moveq   r0,#0

# If the combined search succeeded then the result = base + count << stride
        ldrned  r0,[r0, #24]    @ base in r0, stride in r1
        addne   r0,r0,r4,lsl r1

# Return
        pop     {r4}
        bx      lr
	.size	mem_rsetAlloc,.-mem_rsetAlloc



        
        .global mem_rsetFree
        .type   mem_rsetFree,%function
mem_rsetFree:
        # r0 == rmap address, r1 == address of resource to free
        push    {r4}
        mov     r4,#0x80000000  @ Will be shifted to modify the bitset

        # Calulate the position of the bit to modify in the resource map's bitset
	ldrd    r2,[r0, #24]    @ r2 <- base, r3 <- stride
        sub     r1,r1,r2        @ Subtract base from resource address
        mov     r1,r1,lsr r3    @ Shift right by stride

        # Make sure that all reads and writes to the
        # resource about to be freed have been completed
        # and posted to other CPUs. See JIRA CCI-159.
        dmb

1:      
        # r0 == rmap address, r1 == position of bit to set (0..63)
        mov     ip,r1
        cmp     ip,#32          @ positions >= 32 are in the high bitset word
        subge   ip,#32          @ ip <- position within high word or low word
        ldrexd  r2,r3,[r0]      @ r2 <- bitset low, r3 <- bitset high
        orrlt   r2,r2,r4,lsr ip
        orrge   r3,r3,r4,lsr ip
        strexd  ip,r2,r3,[r0]   @ Store the modified bitset
        cmp     ip,#0           @ Did the store succeed?
        bne     1b              @ If not then retry

        pop     {r4}
        bx      lr
        .size   mem_rsetFree,.-mem_rsetFree





        .global mem_rsetSetup
        .type   mem_rsetSetup,%function
mem_rsetSetup:
        # r0 == rmap address, r1 == no. of resources (0..64), r2 == base, r3 == stride
        strd    r2,[r0,#24]     @ Store the base and stride

        mov     ip,#0x80000000  @ Will be shifted to construct the bitset
        mov     r2,#0           @ Low word of the bitmask under construction
        mov     r3,#0           @ High word
        subs    r1,r1,#1        @ More than zero resources?
        movge   r2,ip,asr r1    @ Set low word
        subs    r1,r1,#32       @ More than 32 resources?
        movge   r3,ip,asr r1    @ Set high word

        # r0 == map address, r2,r3 == bitset
        # Store the constructed bitset
        strd    r2,[r0]
        bx      lr
        .size   mem_rsetSetup,.-mem_rsetSetup
