#pragma once
#include <atomic>
#include <cheat-base/PipeTransfer.h>

class PipeMessage : public PipeSerializedObject
{
public:
	PipeMessage();
	PipeMessage(uint32_t packetID);
	uint32_t packetID() const;
	int64_t timestamp() const;

	void SetMessage(const PipeMessage& other);
	void Write(PipeTransfer* transfer) override;
	void Read(PipeTransfer* transfer) override;

private:

	uint32_t m_PipePacketID;
	int64_t m_Timestamp;
};