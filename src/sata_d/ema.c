#include "ema.h"

NTSTATUS EmaExecuteSingle(PVOID Param1) {
    const u32 IOCTL_CODE__EG = 0x4D014;

    SATA_EMA_BLOCK Block;
    SATA_EXTENSION* ChannelExtension;
    NTSTATUS Status;

    ChannelExtension = SataCdRomChannelExtension.ChannelExtension;

    if ((ChannelExtension->Flags & 0x10) != 0)
        return STATUS_SUCCESS;

    memset(&Block, 0, sizeof(Block));
    Block.Size = sizeof(Block);
    Block.Flags = 1;
    Block.TransferLength = 0x800;
    Block.Buffer = SataCdRomAP21ScratchBuffer;

    Status = ExExpansionCall(EMA_MAGIC, 0x10001, Param1, &Block, 0);

    if (Status != STATUS_PENDING)
        return Status;

    Status = IoSynchronousDeviceIoControlRequest(IOCTL_CODE__EG, ChannelExtension, &Block, sizeof(Block),
                                                 NULL, 0, NULL);

    return ExExpansionCall(EMA_MAGIC, 0x10002, Param1, &Block, Status);
}

NTSTATUS EmaExecute(PVOID Param1) {
    NTSTATUS status;

    while ((status = EmaExecuteSingle(Param1)) == STATUS_PENDING)
        ;

    if (status == STATUS_NOT_IMPLEMENTED)
        return STATUS_SUCCESS;

    return status;
}
