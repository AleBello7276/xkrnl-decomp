#pragma once

#include "kd.h"

// KdSerialPhysicalPort
bool KdpSerialPortInitialize(bool);
bool KdpSerialRecivePacket(void*, void*, void*, void*);
void KdpSerialSendPacket(void*, void*, void*);
