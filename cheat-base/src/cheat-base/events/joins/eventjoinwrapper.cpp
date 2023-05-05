#include "eventjoinwrapper.h"

#include <type_traits>
#include "abstracteventjoin.h"


namespace events::joins 
{
    constexpr EventJoinWrapper::EventJoinWrapper() noexcept :
        m_eventJoin( nullptr )
    { }

    EventJoinWrapper::EventJoinWrapper( EventJoinWrapper&& other ) noexcept :
        m_eventJoin( std::move( other.m_eventJoin ) )
    { }

    EventJoinWrapper::EventJoinWrapper( EventJoinWrapper& other ) noexcept :
        m_eventJoin( other.m_eventJoin )
    { }

    EventJoinWrapper& EventJoinWrapper::operator=( EventJoinWrapper&& other ) noexcept
    {
        m_eventJoin = std::move( other.m_eventJoin );

        return *this;
    }

    EventJoinWrapper& EventJoinWrapper::operator=( const EventJoinWrapper& other ) noexcept
    {
        m_eventJoin = other.m_eventJoin;

        return *this;
    }


    EventJoinWrapper::operator bool() const
    {
        return isJoined();
    }


    bool EventJoinWrapper::isAssigned() const
    {
        return ( m_eventJoin != nullptr );
    }


    bool EventJoinWrapper::isJoined() const
    {
        return ( m_eventJoin != nullptr && m_eventJoin->isJoined() );
    }

    bool EventJoinWrapper::join()
    {
        return ( m_eventJoin != nullptr ? m_eventJoin->join() : false );
    }

    bool EventJoinWrapper::unjoin()
    {
        return ( m_eventJoin != nullptr ? m_eventJoin->unjoin() : false );
    }
} // events
