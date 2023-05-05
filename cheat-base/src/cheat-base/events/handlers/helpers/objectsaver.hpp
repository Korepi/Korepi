#pragma once


namespace events::handlers
{
	template<class TSome>
	struct ObjectSaver;

	template<class LValue>
	struct ObjectSaver<LValue&>
	{
		using TObject = LValue&;
	};

	template<class RValue>
	struct ObjectSaver<RValue&&>
	{
		using TObject = RValue;
	};
}
