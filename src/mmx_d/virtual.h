#pragma once

#include <types.h>

typedef struct _MMVAD {
    void* m_startVpn;
    void* m_endVpn;
    struct _MMVAD* m_parent;
    struct _MMVAD* m_leftLeaf;
    struct _MMVAD* m_rightLeaf;
} MMVAD;
