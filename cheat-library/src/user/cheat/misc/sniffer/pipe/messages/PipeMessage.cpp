#include "pch-il2cpp.h"
#include "PipeMessage.h"

PipeMessage::PipeMessage(uint32_t packetID) :
	m_PipePacketID(packetID), m_Timestamp(util::GetCurrentTimeMillisec())
{ }

PipeMessage::PipeMessage() : m_PipePacketID(0), m_Timestamp(0)
{ }

uint32_t PipeMessage::packetID() const
{
	return m_PipePacketID;
}

int64_t PipeMessage::timestamp() const
{
	return m_Timestamp;
}

void PipeMessage::Write(PipeTransfer* transfer)
{
	transfer->Write(m_PipePacketID);
	transfer->Write(util::GetCurrentTimeMillisec());
}

void PipeMessage::Read(PipeTransfer* transfer)
{
	transfer->Read(m_PipePacketID);
	transfer->Read(m_Timestamp);
}

void PipeMessage::SetMessage(const PipeMessage& other)
{
	m_PipePacketID = other.m_PipePacketID;
	m_Timestamp = other.m_Timestamp;
}