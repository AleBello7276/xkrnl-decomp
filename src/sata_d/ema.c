#include "ema.h"

extern NTSTATUS ExExpansionCall(u32 SelectorId, u32 SomeCodeIDX, void* Param1, void* Block, u32);

extern NTSTATUS IoSynchronousDeviceIoControlRequest(uint64_t IoControlCode, void* DeviceObject,
                                                    PVOID InputBuffer, uint64_t InputBufferLength,
                                                    PVOID OutputBuffer, uint64_t OutputBufferLength,
                                                    uint64_t* BytesReturned);

NTSTATUS EmaExecuteSingle(PVOID Param1) {
    const u32 IOCTL_CODE__EG = 0x4D014;

    SATA_EMA_BLOCK Block;
    SataExtension* ChannelExtension;
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

    return ExExpansionCall(0x454D4120, 0x10002, Param1, &Block, Status);
}

NTSTATUS EmaExecute(PVOID Param1) {
    NTSTATUS status;

    while ((status = EmaExecuteSingle(Param1)) == STATUS_PENDING) {
    }

    if (status == STATUS_NOT_IMPLEMENTED)
        return STATUS_SUCCESS;
    else
        return status;
}
