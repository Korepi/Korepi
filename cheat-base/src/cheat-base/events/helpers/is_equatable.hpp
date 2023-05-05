#pragma once

#include <type_traits>


template<class T>
class is_equatable
{
    private:

        template<class U>
        static constexpr std::true_type exists( decltype( std::declval<U>() == std::declval<U>() )* = nullptr ) noexcept;

        template<class U>
        static constexpr std::false_type exists( ... ) noexcept;

    public:

        static constexpr bool value = decltype( exists<T>( nullptr ) )::value;
};
