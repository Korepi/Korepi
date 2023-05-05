#include "pch-il2cpp.h"
#include "PipeIO.h"

void PipeIO::Send(PipeMessage& data)
{
	if (!IsConnected())
		return;

	m_Pipe->WriteObject(data);
}

PipeMessage* PipeIO::ReceiveMessage()
{
	if (!IsConnected())
		return nullptr;

	PipeMessage header;
	m_Pipe->ReadObject(header);

	MessageIDs messageID = static_cast<MessageIDs>(header.packetID());

#define MESSAGE_CASE(mid, type)\
					case mid:                                       		  \
					{										        		  \
						auto data = new type();	            		          \
						m_Pipe->ReadObject(*data);			        		  \
						data->SetMessage(header);                             \
						CallHandlers(*data);					    		  \
						return dynamic_cast<PipeMessage*>(data);          \
					}													  

	switch (messageID)
	{
		MESSAGE_CASE(MessageIDs::PACKET_DATA, PipePacketData);
		MESSAGE_CASE(MessageIDs::MODIFY_DATA, PipeModifyData);
	default:
		break;
	}
	return nullptr;
}

void PipeIO::ProcessMessage()
{
	auto message = ReceiveMessage();
	delete message;
}
