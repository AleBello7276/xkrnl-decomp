#include "kdnetio.h"

KD_PHYSICAL_PORT KdNetPhysicalPort = {
    KdpNetPortInitialize,  // Initialize
    KdpNetRecivePacket,    // RecivePacket
    KdpNetSendPacket       // SendPacket
};
