#pragma once
#include <vector>

#include "PipeIO.h"

class PipeClient : public PipeIO
{
public:
	PipeClient() : PipeIO(), m_ConnectionDelay(5), m_NextTimeToConnect(0) {}

	void SetConnectionDelay(uint32_t delay);
	uint32_t GetConnectionDelay();

	bool IsConnected() final;
	void Connect(const std::string& pipeName);
	void Disconnect();

private:
	uint32_t m_ConnectionDelay;
	std::time_t m_NextTimeToConnect;

	bool TryConnectToPipe();
};