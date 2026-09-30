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

MMVAD_TREE* MiGetVadTreeFromBaseAddress(PKPROCESS Process, ULONG_PTR Base, DWORD Size) {
    MMVAD_TREE* Tree;

    if (Base >= Process->mTreeA.m_rangeStart && Base < Process->mTreeA.m_rangeEnd) {
        Tree = &Process->mTreeA;
    } else if (Base >= Process->mTreeB.m_rangeStart && Base < Process->mTreeB.m_rangeEnd) {
        Tree = &Process->mTreeB;
    } else {
        return NULL;
    }

    if (Size > Tree->m_rangeEnd - Base)
        return NULL;

    return Tree;
}

void MiInsertVad(MMVAD* Node, MMVAD_TREE* Tree) {
    MMVAD* prev;
    MMVAD* current;
    int counter;

    assert(Node->m_endVpn >= Node->m_startVpn);

    prev = Tree->m_reserved;
    Tree->m_hint = Node;
    if (prev != 0) {
        if ((uint32_t)prev->m_endVpn + 0x10 >= (uint32_t)Node->m_startVpn) {
            Tree->m_reserved = Node;
        }
    }

    counter = 0;
    current = Tree->m_root;
    Node->m_leftLeaf = 0;
    Node->m_rightLeaf = 0;

    if (current == 0) {
        Tree->m_root = Node;
        Node->m_parent = 0;
        return;
    }

    for (;;) {
        counter++;
        if (counter == 15)
            MiReorderTree((MMADDRESS_NODE*)current, (MMADDRESS_NODE**)&Tree->m_root);
        if (Node->m_startVpn < current->m_startVpn) {
            if (current->m_leftLeaf == 0) {
                current->m_leftLeaf = Node;
                break;
            }
            current = current->m_leftLeaf;
        } else {
            if (current->m_rightLeaf == 0) {
                current->m_rightLeaf = Node;
                break;
            }
            current = current->m_rightLeaf;
        }
    }
    Node->m_parent = current;
}

MMVAD* MiGetNextVad(MMVAD* vad) {
    MMVAD* Node = vad->m_rightLeaf;

    while (Node) {
        vad = Node;
        Node = vad->m_leftLeaf;
        if (Node == NULL)
            return NULL;
    }

    Node = vad->m_parent;

    while (Node) {
        if (Node->m_leftLeaf == vad)
            return vad->m_parent;
        vad = Node;
        Node = vad->m_parent;
    }

    return NULL;
}

MMVAD* MiGetPreviousVad(MMVAD* vad) {
    MMVAD* Node = vad->m_leftLeaf;
    if (Node) {
        do {
            vad = Node;
            Node = vad->m_rightLeaf;
        } while (Node);
        return vad;
    }

    Node = vad->m_parent;
    if (!Node)
        goto fail;
    do {
        if (Node->m_rightLeaf == vad)
            return vad->m_parent;
        vad = Node;
        Node = vad->m_parent;
    } while (Node);
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
