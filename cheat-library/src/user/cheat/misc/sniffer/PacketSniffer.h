// ReSharper disable All
#pragma once

#include <filesystem>

#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

#include <cheat-base/PipeTransfer.h>
#include "pipe/messages/PipeModifyData.h"
#include "pipe/messages/PipePacketData.h"
#include "pipe/PipeClient.h"

namespace cheat::feature 
{
	
	class PacketSniffer : public Feature
    {
	public:
		config::Field<bool> f_CaptureEnabled;
		config::Field<bool> f_ManipulationEnabled;
		config::Field<std::string> f_PipeName;

		PipeClient client;

		static PacketSniffer& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

	private:
		PacketSniffer();

		bool ProcessModifiedData(app::KcpPacket_1*& packet);
		bool ParseRawPacketData(char* encryptedData, uint32_t length, PipePacketData& dataOut);
		static void EncryptXor(void* content, uint32_t length);
		static bool KcpClient_TryDequeueEvent_Hook(void* __this, app::ClientKcpEvent* evt, MethodInfo* method);
		static int32_t KcpNative_kcp_client_send_packet_Hook(void* kcp_client, app::KcpPacket_1* packet, MethodInfo* method);
		
		bool OnPacketIO(app::KcpPacket_1*& packet, NetIODirection direction);
	};
}

