#pragma once

#include <types.h>

typedef struct _MMVAD {
    void* m_startVpn;
    void* m_endVpn;
    struct _MMVAD* m_parent;
    struct _MMVAD* m_leftLeaf;
    struct _MMVAD* m_rightLeaf;
} MMVAD;

typedef struct _MMVAD_TREE {
    MMVAD* m_root;          // +0x00
    MMVAD* m_hint;          // +0x04
    MMVAD* m_reserved;      // +0x08
    uint32_t m_unkC;        // +0x0C
    uint32_t m_rangeStart;  // +0x10
    uint32_t m_rangeEnd;    // +0x14
    uint32_t m_unk18;       // +0x18
} MMVAD_TREE;

typedef struct _KPROCESS {
    char m_pad0[0x70];
    MMVAD_TREE m_vadTree1;  // +0x70 (size 0x1C)
    MMVAD_TREE m_vadTree2;  // +0x8C
} KPROCESS;
