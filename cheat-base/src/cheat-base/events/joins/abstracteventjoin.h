#pragma once


namespace events::joins 
{
	class AbstractEventJoin
	{
	public:

		virtual ~AbstractEventJoin();

		virtual bool isJoined() const = 0;
		virtual bool join() = 0;
		virtual bool unjoin() = 0;

	protected:

		AbstractEventJoin();
	};
}
