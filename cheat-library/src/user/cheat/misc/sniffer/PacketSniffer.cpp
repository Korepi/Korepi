#include "pch-il2cpp.h"
#include "PacketSniffer.h"

#ifdef _PACKET_SNIFFER
#include "SnifferWindow.h"
#endif

#include <fstream>
#include <helpers.h>

namespace cheat::feature 
{
	PacketSniffer::PacketSniffer() : Feature(),
		NF(f_CaptureEnabled, "PacketSniffer", false),
		NF(f_ManipulationEnabled, "PacketSniffer", false),
		NF(f_PipeName, "PacketSniffer", "genshin_packet_pipe")

	{
		client.Connect(f_PipeName.value());

		HookManager::install(app::Kcp_KcpNative_kcp_client_send_packet, KcpNative_kcp_client_send_packet_Hook);
		HookManager::install(app::MoleMole_KcpClient_TryDequeueEvent, KcpClient_TryDequeueEvent_Hook);
	}

	const FeatureGUIInfo& PacketSniffer::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Packet Sniffer", "Settings");
		return info;
	}

	void PacketSniffer::DrawMain()
	{
		ImGui::Text(_TR("Dev: for working needs server for named pipe with specified name.\nCheck 'packet-handler' project like example."));
		ConfigWidget(_TR("Pipe name"), f_PipeName, _TR("Pipe name for connecting. Changes will apply after next game launch."));
		ConfigWidget(_TR("Capturing"), f_CaptureEnabled, _TR("Enable capturing of packet info and sending to pipe, if it exists."));
		ConfigWidget(_TR("Manipulation"), f_ManipulationEnabled, _TR("Enable blocking and modifying packets by sniffer, can cause network lags."));
	}
	
	PacketSniffer& PacketSniffer::GetInstance()
	{
		static PacketSniffer instance;
		return instance;
	}

	bool PacketSniffer::ProcessModifiedData(app::KcpPacket_1*& packet)
	{
		auto modify_data = client.WaitFor<PipeModifyData>();
		if (!modify_data)
			return false;

		switch (modify_data->modifyType)
		{
		case ModifyType::Blocked:
			return true;

		case ModifyType::Modified:
		{
			const uint32_t data_size = static_cast<uint32_t>(
				modify_data->head.size() + modify_data->content.size() + 12
			);
			char* data = new char[data_size];

			auto head_size = static_cast<uint16_t>(modify_data->head.size());
			auto message_size = static_cast<uint32_t>(modify_data->content.size());

			util::WriteMapped(data, 0, static_cast<uint16_t>(0x4567)); // Magic number
			util::WriteMapped(data, 2, static_cast<uint16_t>(modify_data->messageID)); // Message id
			util::WriteMapped(data, 4, head_size); // Head size
			util::WriteMapped(data, 6, message_size); // Message size

			// Fill content
			char* ptr_head_content = data + 10;
			memcpy_s(ptr_head_content, head_size, modify_data->head.data(), head_size);

			char* ptr_message_content = ptr_head_content + modify_data->head.size();
			memcpy_s(ptr_message_content, message_size, modify_data->content.data(), message_size);
			
			util::WriteMapped(ptr_message_content, static_cast<int>(message_size), static_cast<uint16_t>(0x89AB));

			EncryptXor(data, data_size);

			// Can be memory leak.
			auto new_packet = app::Kcp_KcpNative_kcp_packet_create(reinterpret_cast<uint8_t*>(data), static_cast<int32_t>(data_size), nullptr);
			delete[] data;

			// Will be deleted by KcpNative_kcp_client_network_thread
			// app::Kcp_KcpNative_kcp_packet_destroy(packet, nullptr);
			packet = new_packet;
		}
		break;
		case ModifyType::Unchanged:
		default:
			break;
		}
		return false;
	}

	bool PacketSniffer::OnPacketIO(app::KcpPacket_1*& packet, NetIODirection direction)
	{
		if (!client.IsConnected())
			return true;

		if (!f_CaptureEnabled)
			return true;

		auto pipeData = client.CreateMessage<PipePacketData>();
		bool parsed = ParseRawPacketData((char*)packet->data, packet->dataLen, pipeData);
		if (!parsed)
			return true;

		pipeData.direction = direction;
		pipeData.manipulationEnabled = f_ManipulationEnabled;
		client.Send(pipeData);

		bool canceled = f_ManipulationEnabled && ProcessModifiedData(packet);

		return !canceled;
	}

	void PacketSniffer::EncryptXor(void* content, uint32_t length)
	{
		auto byteArray = reinterpret_cast<app::Byte__Array*>(new char[length + 0x20]);
		byteArray->max_length = length;
		memcpy_s(byteArray->vector, length, content, length);

		app::MoleMole_Packet_XorEncrypt(&byteArray, length, nullptr);

		memcpy_s(content, length, byteArray->vector, length);
		delete[] byteArray;
	}

	bool PacketSniffer::ParseRawPacketData(char* encryptedData, uint32_t length, PipePacketData& dataOut)
	{
		// Packet structure
		// * Magic word (0x4567) [2 bytes]
		// * message_id [2 bytes] 
		// * head_size [2 bytes]
		// * message_size [4 bytes]
		// * head_content [<head_size> bytes]
		// * message_content [<message_size> bytes]
		// * Magic word (0x89AB) [2 bytes]

		// Header size - 12 bytes
		
		// Decrypting packetData
		auto data = new char[length];
		memcpy_s(data, length, encryptedData, length);
		EncryptXor(data, length);

		uint16_t magicHead = util::ReadMapped<uint16_t>(data, 0);

		if (magicHead != 0x4567)
		{
			LOG_ERROR("Head magic value for packet is not valid.");
			return false;
		}

		uint16_t magicEnd = util::ReadMapped<uint16_t>(data, static_cast<int>(length) - 2);
		if (magicEnd != 0x89AB)
		{
			LOG_ERROR("End magic value for packet is not valid.");
			return false;
		}

		uint16_t messageId = util::ReadMapped<uint16_t>(data, 2);
		uint16_t headSize = util::ReadMapped<uint16_t>(data, 4);
		uint32_t contenSize = util::ReadMapped<uint32_t>(data, 6);

		if (length < headSize + contenSize + 12)
		{
			LOG_ERROR("Packet size is not valid.");
			return false;
		}

		dataOut.messageID = messageId;

		dataOut.head = std::vector<byte>((size_t)headSize, 0);
		memcpy_s(dataOut.head.data(), headSize, data + 10, headSize);

		dataOut.content = std::vector<byte>((size_t)contenSize, 0);
		memcpy_s(dataOut.content.data(), contenSize, data + 10 + headSize, contenSize);

		delete[] data;

		return true;
	}

	bool PacketSniffer::KcpClient_TryDequeueEvent_Hook(void* __this, app::ClientKcpEvent* evt, MethodInfo* method)
	{
		auto result = CALL_ORIGIN(KcpClient_TryDequeueEvent_Hook, __this, evt, method);

		if (!result || evt->_evt.type != app::KcpEventType__Enum::EventRecvMsg ||
			evt->_evt.packet == nullptr || evt->_evt.packet->data == nullptr)
			return result;

		auto& sniffer = GetInstance();
		return sniffer.OnPacketIO(evt->_evt.packet, NetIODirection::Receive);
	}

	int32_t PacketSniffer::KcpNative_kcp_client_send_packet_Hook(void* kcp_client, app::KcpPacket_1* packet, MethodInfo* method)
	{
		auto& sniffer = GetInstance();
		if (!sniffer.OnPacketIO(packet, NetIODirection::Send))
			return 0;

		return CALL_ORIGIN(KcpNative_kcp_client_send_packet_Hook, kcp_client, packet, method);
	}
}

