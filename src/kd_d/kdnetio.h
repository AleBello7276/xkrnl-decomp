#pragma once

#include "kd.h"

// KdNetPhysicalPort
bool KdpNetPortInitialize(bool);
bool KdpNetRecivePacket(void*, void*, void*, void*);
void KdpNetSendPacket(void*, void*, void*);
