#pragma once
#include "messages/PipeMessage.h"
#include "messages/PipeModifyData.h"
#include "messages/PipePacketData.h"

#include <vector>
#include <optional>


enum class MessageIDs : uint32_t
{
	NONE = 0,
	PACKET_DATA = 1,
	MODIFY_DATA = 2
};

namespace internal
{
	template<class TMessage> struct message_to_id;
	template<> struct message_to_id<PipePacketData> : std::integral_constant<uint32_t, static_cast<uint32_t>(MessageIDs::PACKET_DATA)> {};
	template<> struct message_to_id<PipeModifyData> : std::integral_constant<uint32_t, static_cast<uint32_t>(MessageIDs::MODIFY_DATA)> {};
}

class PipeIO
{
public:
	template<class T>
	using CallbackFunction = void(*)(const T& packetData);

	template<class T>
	void AddHandler(CallbackFunction<T> callback)
	{
		static_assert(std::is_base_of<PipeMessage, T>::value, "Should be derived of MessageBase.");

		constexpr auto messageID = internal::message_to_id<T>::value;

		m_Handlers.push_back({
			messageID,
			reinterpret_cast<UniqueCallbackFunction>(callback)
			});
	}

	template<class TMessage>
	TMessage CreateMessage(uint32_t reqID = 0, uint32_t seqID = 0)
	{
		static_assert(std::is_base_of<PipeMessage, TMessage>::value, "Should be derived of MessageBase.");

		return TMessage(internal::message_to_id<TMessage>::value);
	}

	template<class TMessage>
	std::optional<TMessage> WaitFor()
	{
		static_assert(std::is_base_of<PipeMessage, TMessage>::value, "Should be derived of MessageBase.");

		if (!IsConnected())
			return {};

		while (true)
		{
			auto messagePtr = ReceiveMessage();
			if (messagePtr == nullptr)
				return {};

			if (internal::message_to_id<TMessage>::value != messagePtr->packetID())
			{
				delete messagePtr;
				continue;
			}

			// Copying message
			TMessage message = *reinterpret_cast<TMessage*>(messagePtr);
			delete messagePtr;

			return message;
		}
	}

	void Send(PipeMessage& data);

	PipeMessage* ReceiveMessage();

	void ProcessMessage();

	virtual bool IsConnected() = 0;

protected:
	PipeIO() : m_Pipe(nullptr) {};
	virtual ~PipeIO() = default;
	PipeTransfer* m_Pipe;

private:
	using UniqueCallbackFunction = void(*)(const PipeMessage& pipeMessage);
	std::vector<std::pair<uint32_t, UniqueCallbackFunction>> m_Handlers;

	template<class T>
	void CallHandlers(const T& message)
	{
		for (auto& [id, handler] : m_Handlers)
		{
			if (id == internal::message_to_id<T>::value)
				handler(message);
		}
	}

public:
	PipeIO(PipeIO& other) = delete;
	PipeIO(PipeIO&& other) = delete;
	PipeIO& operator=(PipeIO& other) = delete;
	PipeIO& operator=(PipeIO&& other) = delete;
};