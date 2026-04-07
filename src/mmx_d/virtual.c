#include "virtual.h"

#include "krnl.h"
void MiReorderTree(MMADDRESS_NODE* node, MMADDRESS_NODE** rootVad) {
    MMADDRESS_NODE* p_Var1;
    MMADDRESS_NODE* parent;
    MMADDRESS_NODE* p_Var3;
    MMADDRESS_NODE* grandParent;
    MMADDRESS_NODE* p_Var5;
    MMADDRESS_NODE* idk;
    idk = node;

    while (idk != *rootVad) {
        p_Var1 = *rootVad;
        parent = idk->m_parent;

        if (parent == p_Var1) {
            p_Var1 = parent->m_leftLeaf;
            *rootVad = idk;
            idk->m_parent = nullptr;
            parent->m_parent = idk;

            if (idk != p_Var1) {
                p_Var1 = idk->m_leftLeaf;
                parent->m_rightLeaf = p_Var1;
                if (p_Var1 != nullptr) {
                    p_Var1->m_parent = parent;
                }
                idk->m_leftLeaf = parent;
            } else {
                p_Var1 = idk->m_rightLeaf;
                parent->m_leftLeaf = p_Var1;
                if (p_Var1 != nullptr) {
                    p_Var1->m_parent = parent;
                }
                idk->m_rightLeaf = parent;
            }
            break;
        } else {
            p_Var3 = parent->m_leftLeaf;
            grandParent = parent->m_parent;
            if ((idk == parent->m_leftLeaf) && (parent == grandParent->m_leftLeaf)) {
                if (grandParent == p_Var1) {
                    *rootVad = parent;
                    parent->m_parent = nullptr;
                } else {
                    p_Var1 = grandParent->m_parent;
                    p_Var3 = p_Var1->m_leftLeaf;
                    parent->m_parent = p_Var1;
                    if (grandParent == p_Var3) {
                        p_Var1->m_leftLeaf = parent;
                    } else {
                        p_Var1->m_rightLeaf = parent;
                    }
                }
                p_Var1 = parent->m_rightLeaf;
                grandParent->m_leftLeaf = p_Var1;
                if (p_Var1 != nullptr) {
                    p_Var1->m_parent = grandParent;
                }
                grandParent->m_parent = parent;
                parent->m_rightLeaf = grandParent;
                idk = parent;
            } else if ((idk == parent->m_rightLeaf) && (parent == grandParent->m_rightLeaf)) {
                if (grandParent == p_Var1) {
                    *rootVad = parent;
                    parent->m_parent = nullptr;
                } else {
                    p_Var1 = grandParent->m_parent;
                    p_Var5 = p_Var1->m_leftLeaf;
                    parent->m_parent = p_Var1;
                    if (grandParent == p_Var5) {
                        p_Var1->m_leftLeaf = parent;
                    } else {
                        p_Var1->m_rightLeaf = parent;
                    }
                }
                grandParent->m_rightLeaf = p_Var3;
                if (p_Var3 != nullptr) {
                    p_Var3->m_parent = grandParent;
                }
                grandParent->m_parent = parent;
                parent->m_leftLeaf = grandParent;
                idk = parent;
            } else {
                if ((idk == p_Var3) && (parent == grandParent->m_rightLeaf)) {
                    if (grandParent == p_Var1) {
                        *rootVad = idk;
                        idk->m_parent = nullptr;
                    } else {
                        p_Var1 = grandParent->m_parent;
                        p_Var3 = p_Var1->m_leftLeaf;
                        idk->m_parent = p_Var1;
                        if (grandParent == p_Var3) {
                            p_Var1->m_leftLeaf = idk;
                        } else {
                            p_Var1->m_rightLeaf = idk;
                        }
                    }
                    p_Var1 = idk->m_rightLeaf;
                    parent->m_leftLeaf = p_Var1;
                    if (p_Var1 != nullptr) {
                        p_Var1->m_parent = parent;
                    }
                    p_Var1 = idk->m_leftLeaf;
                    grandParent->m_rightLeaf = p_Var1;
                    if (p_Var1 != nullptr) {
                        p_Var1->m_parent = grandParent;
                    }
                    idk->m_leftLeaf = grandParent;
                    idk->m_rightLeaf = parent;
                } else {
                    if (grandParent == p_Var1) {
                        *rootVad = idk;
                        idk->m_parent = nullptr;
                    } else {
                        p_Var1 = grandParent->m_parent;
                        p_Var3 = p_Var1->m_leftLeaf;
                        idk->m_parent = p_Var1;
                        if (grandParent == p_Var3) {
                            p_Var1->m_leftLeaf = idk;
                        } else {
                            p_Var1->m_rightLeaf = idk;
                        }
                    }
                    p_Var1 = idk->m_leftLeaf;
                    parent->m_rightLeaf = p_Var1;
                    if (p_Var1 != nullptr) {
                        p_Var1->m_parent = parent;
                    }
                    p_Var1 = idk->m_rightLeaf;
                    grandParent->m_leftLeaf = p_Var1;
                    if (p_Var1 != nullptr) {
                        p_Var1->m_parent = grandParent;
                    }
                    idk->m_leftLeaf = parent;
                    idk->m_rightLeaf = grandParent;
                }
                grandParent->m_parent = idk;
                parent->m_parent = idk;
            }
        }
    }
    return;
}
uint32_t MiGetVadTreeFromBaseAddress() {}

void MiInsertVad(MMVAD* node, MMVAD* rootVad) {
    assert(node->m_endVpn >= node->m_startVpn);

    rootVad->m_endVpn = node;
}

MMVAD* MiGetNextVad(MMVAD* vad) {
    MMVAD* node = vad->m_rightLeaf;
    MMVAD* cur;
    MMVAD* parent;
    if (node != nullptr) {
        while (node->m_leftLeaf != nullptr)
            node = node->m_leftLeaf;
        return node;
    }

    cur = vad;
    parent = cur->m_parent;
    while (parent != nullptr) {
        if (parent->m_leftLeaf == cur)
            return cur->m_parent;
        cur = parent;
        parent = cur->m_parent;
    }
    return nullptr;
}
