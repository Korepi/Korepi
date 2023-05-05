#pragma once
#include <cheat-base/PipeTransfer.h>
#include "PipeMessage.h"

enum class ModifyType
{
	Unchanged, Blocked, Modified
};

class PipeModifyData : public PipeMessage
{
public:
	using PipeMessage::PipeMessage;

	ModifyType modifyType;
	uint32_t messageID;
	std::vector<uint8_t> head;
	std::vector<uint8_t> content;

	PipeModifyData();
	~PipeModifyData() {}

	// Inherited via PipeSerializedObject
	virtual void Write(PipeTransfer* transfer) final;
	virtual void Read(PipeTransfer* transfer) final;
};