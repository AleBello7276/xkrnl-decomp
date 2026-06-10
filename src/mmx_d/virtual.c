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

            if (idk == p_Var1) {
                p_Var1 = idk->m_rightLeaf;
                parent->m_leftLeaf = p_Var1;
                if (p_Var1 != nullptr) {
                    p_Var1->m_parent = parent;
                }
                idk->m_rightLeaf = parent;
            } else {
                p_Var1 = idk->m_leftLeaf;
                parent->m_rightLeaf = p_Var1;
                if (p_Var1 != nullptr) {
                    p_Var1->m_parent = parent;
                }
                idk->m_leftLeaf = parent;
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
MMVAD_TREE* MiGetVadTreeFromBaseAddress(KPROCESS* process, uint32_t baseAddr, uint32_t size) {
    MMVAD_TREE* tree;

    if (baseAddr >= process->m_vadTree1.m_rangeStart && baseAddr < process->m_vadTree1.m_rangeEnd) {
        tree = &process->m_vadTree1;
    } else if (baseAddr >= process->m_vadTree2.m_rangeStart && baseAddr < process->m_vadTree2.m_rangeEnd) {
        tree = &process->m_vadTree2;
    } else {
        goto fail;
    }
    if (size > tree->m_rangeEnd - baseAddr)
        goto fail;
    return tree;
fail:
    return 0;
}

void MiInsertVad(MMVAD* node, MMVAD_TREE* tree) {
    MMVAD* prev;
    MMVAD* current;
    int counter;

    assert(node->m_endVpn >= node->m_startVpn);

    prev = tree->m_reserved;
    tree->m_hint = node;
    if (prev != 0) {
        if ((uint32_t)prev->m_endVpn + 0x10 >= (uint32_t)node->m_startVpn) {
            tree->m_reserved = node;
        }
    }

    counter = 0;
    current = tree->m_root;
    node->m_leftLeaf = 0;
    node->m_rightLeaf = 0;

    if (current == 0) {
        tree->m_root = node;
        node->m_parent = 0;
        return;
    }

    for (;;) {
        counter++;
        if (counter == 15)
            MiReorderTree((MMADDRESS_NODE*)current, (MMADDRESS_NODE**)&tree->m_root);
        if (node->m_startVpn < current->m_startVpn) {
            if (current->m_leftLeaf == 0) {
                current->m_leftLeaf = node;
                break;
            }
            current = current->m_leftLeaf;
        } else {
            if (current->m_rightLeaf == 0) {
                current->m_rightLeaf = node;
                break;
            }
            current = current->m_rightLeaf;
        }
    }
    node->m_parent = current;
}

MMVAD* MiGetNextVad(MMVAD* vad) {
    MMVAD* node = vad->m_rightLeaf;
    if (node) {
        do {
            vad = node;
            node = vad->m_leftLeaf;
        } while (node);
        return vad;
    }

    node = vad->m_parent;
    if (!node)
        goto fail;
    do {
        if (node->m_leftLeaf == vad)
            return vad->m_parent;
        vad = node;
        node = vad->m_parent;
    } while (node);
fail:
    return 0;
}

MMVAD* MiGetPreviousVad(MMVAD* vad) {
    MMVAD* node = vad->m_leftLeaf;
    if (node) {
        do {
            vad = node;
            node = vad->m_rightLeaf;
        } while (node);
        return vad;
    }

    node = vad->m_parent;
    if (!node)
        goto fail;
    do {
        if (node->m_rightLeaf == vad)
            return vad->m_parent;
        vad = node;
        node = vad->m_parent;
    } while (node);
fail:
    return 0;
}

void HvxEncryptedAllocationReserve() {
    __asm {
        li r0, 0x47;
        sc;
    }
}

void HvxEncryptedAllocationMap() {
    __asm {
        li r0, 0x48;
        sc;
    }
}

void HvxEncryptedAllocationUnmap() {
    __asm {
        li r0, 0x49;
        sc;
    }
}

void HvxEncryptedAllocationRelease() {
    __asm {
        li r0, 0x4a;
        sc;
    }
}
