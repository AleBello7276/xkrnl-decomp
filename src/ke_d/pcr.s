KPCR struct
    paddd        db 24 dup (?)
    m_currentIrql db ?
    pad        db 87 dup (?)
    m_stackPtr dd ?
    pad2       db 140 dup (?)
    unk_0x100 dd ?
    pad3       db 8 dup (?)
    m_ProcessorNum db ?
KPCR ends
