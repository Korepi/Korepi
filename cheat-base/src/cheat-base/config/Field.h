#pragma once

#include "internal/FieldSerialize.h"
#include "internal/FieldBase.h"
namespace config 
{
	template<class T>
	class Field : public internal::FieldBase<T>
	{
	public:
		using base = internal::FieldBase<T>;
		using base::operator=;
		using base::base;
	};
}