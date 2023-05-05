#pragma once

#include "eventjoinwrapper.h"
#include "handlereventjoin.h"
#include "../handlers/handlercast.hpp"


namespace events::joins 
{
    template<class TSome, class ...TParams>
    EventJoinWrapper::EventJoinWrapper( IEvent<TParams...>& _event, TSome&& handler ) :
        m_eventJoin( std::make_shared<HandlerEventJoin<TParams...>>( _event, ::events::handlers::HandlerCast<TSome>::template cast<TParams...>( handler ) ) )
    { }
}
