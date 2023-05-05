#include "pch-il2cpp.h"
#include "PipeClient.h"

bool PipeClient::IsConnected()
{
	if (m_Pipe == nullptr)
		return false;

	return m_Pipe->IsPipeOpened() || TryConnectToPipe();
}

void PipeClient::SetConnectionDelay(uint32_t delay)
{
	m_ConnectionDelay = delay;
}

uint32_t PipeClient::GetConnectionDelay()
{
	return m_ConnectionDelay;
}

void PipeClient::Connect(const std::string& pipeName)
{
	if (m_Pipe != nullptr)
		delete m_Pipe;

	m_Pipe = new PipeTransfer(pipeName);
	TryConnectToPipe();
}

void PipeClient::Disconnect()
{
	if (m_Pipe == nullptr)
		return;

	delete m_Pipe;
}

bool PipeClient::TryConnectToPipe()
{
	if (m_Pipe == nullptr)
		return false;

	std::time_t currTime = std::time(0);
	if (m_NextTimeToConnect > currTime)
		return false;

	bool result = m_Pipe->Connect();
	if (result)
		LOG_INFO("Connected to pipe successfully.");
	else
		m_NextTimeToConnect = currTime + 5; // delay in 5 sec
	return result;
}