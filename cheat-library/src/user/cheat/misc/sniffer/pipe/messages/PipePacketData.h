#pragma once
#include "PipeMessage.h"

enum class NetIODirection
{
	Send, Receive
};

class PipePacketData : public PipeMessage
{
public:
	using PipeMessage::PipeMessage;

	bool manipulationEnabled;

	NetIODirection direction;
	uint16_t messageID;
	std::vector<byte> head;
	std::vector<byte> content;

	// Inherited via PipeSerializedObject
	void Write(PipeTransfer* transfer) final;
	void Read(PipeTransfer* transfer) final;
};
