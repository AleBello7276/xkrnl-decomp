#include "kdcomio.h"

KD_PHYSICAL_PORT KdSerialPhysicalPort = {
    KdpSerialPortInitialize,  // Initialize
    KdpSerialRecivePacket,    // RecivePacket
    KdpSerialSendPacket       // SendPacket
};
