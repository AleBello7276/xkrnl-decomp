include pcr.s ; PCR struct

extern KiInitializeKernelStackBase:byte
extern KiInitializeKernel:near
extern KiInitializeKernel2:near
extern KiInitializeNonBootProcessor:near
extern KiIdleLoop:near

.code

; kernel entry point
KiProcessorStartup proc
    li      r0, 0   ; clear R0
    mtlr    r0      ; clear LR

    lbz     r0, PCR.m_threadNum(r13)    ; get m_threadNum, r13 points to current hardware thread PCR
    cmpwi   r0, 0                       ; is thread 0 ? (boot processor)
    bne     nonBootProcessorsPath       ; jump if *Not thread 0*

    ; boot processor
    lis     r31, offset KiInitializeKernelStackBase        ; ;
    addi    r31, r31, offset KiInitializeKernelStackBase   ; load stack base
    subi    r1, r31, 0f0h          ; some space 
    bl      KiInitializeKernel     ; krnl init 1

    subi    r1, r3, 0f0h           ; ;
    bl      KiInitializeKernel2    ; krnl init 2

    b      KiIdleLoop             ; loop

nonBootProcessorsPath:
    lwz     r1, PCR.m_stackPtr(r13)    ; get thread stack
    subi    r1, r1, 0f0h
    bl      KiInitializeNonBootProcessor

    b      KiIdleLoop             ; loop

KiProcessorStartup endp

end
