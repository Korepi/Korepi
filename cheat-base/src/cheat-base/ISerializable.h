#pragma once

#include <nlohmann/json.hpp>

class ISerializable
{
public:
	virtual void to_json(nlohmann::json& j) const = 0;
	virtual void from_json(const nlohmann::json& j) = 0;
};

inline void to_json(nlohmann::json& j, const ISerializable& ser)
{
	ser.to_json(j);
}

inline void from_json(const nlohmann::json& j, ISerializable& ser)
{
	ser.from_json(j);
}