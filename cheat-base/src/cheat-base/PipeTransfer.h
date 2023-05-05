#pragma once

#include <string>
#include <vector>

typedef unsigned char byte;

class PipeTransfer;
class PipeSerializedObject
{
public:
	virtual ~PipeSerializedObject() = default;
	virtual void Write(PipeTransfer* transfer) = 0;
	virtual void Read(PipeTransfer* transfer) = 0;
};

class PipeTransfer
{
public:
	PipeTransfer(const std::string& name);
	~PipeTransfer();

	bool Create();
	void Close();
	bool Connect();
	bool WaitForConnection();
	bool IsPipeOpened();

	void ReadBytes(void* buffer, size_t size);
	void WriteBytes(void* buffer, size_t size);

	void ReadObject(PipeSerializedObject& object);
	void WriteObject(PipeSerializedObject& object);

	template<class T>
	void Read(T& value) 
	{
		ReadBytes(&value, sizeof(T));
	}

	template<class T>
	void Write(const T& val) 
	{
		WriteBytes(const_cast<T*>(&val), sizeof(T));
	}

	template<>
	void Read(std::vector<byte>& vector)
	{
		size_t size; Read(size);
		vector.clear();
		vector.resize(size);
		ReadBytes(vector.data(), size);
	}

	template<>
	void Write(const std::vector<byte>& value)
	{
		Write<size_t>(value.size());
		WriteBytes(const_cast<byte*>(value.data()), value.size());
	}

	template<>
	void Read(std::string& value)
	{
		size_t size; Read(size);
		value.clear();
		value.resize(size);
		ReadBytes(value.data(), size);
	}

	template<>
	void Write(const std::string& value)
	{
		Write<size_t>(value.length());
		WriteBytes(const_cast<char*>(value.data()), value.length());
	}

private:

	std::string m_Name;
	HANDLE m_Pipe;
};

