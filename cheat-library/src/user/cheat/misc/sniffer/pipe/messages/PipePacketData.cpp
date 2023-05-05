#include "pch-il2cpp.h"
#include "PipePacketData.h"

void PipePacketData::Write(PipeTransfer* transfer)
{
	PipeMessage::Write(transfer);

	transfer->Write(manipulationEnabled);
	transfer->Write(direction);
	transfer->Write(messageID);
	transfer->Write(head);
	transfer->Write(content);
}

void PipePacketData::Read(PipeTransfer* transfer)
{
	transfer->Read(manipulationEnabled);
	transfer->Read(direction);
	transfer->Read(messageID);
	transfer->Read(head);
	transfer->Read(content);
}