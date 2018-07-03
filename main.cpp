// -------------------------------------------------------------------------- //
// created by: xcodeassociated ( https://github.com/xcodeassociated/ ) 	
//
// Copyright 2018, Janusz Majchrzak, All rights reserved.
// -------------------------------------------------------------------------- //

#include <iostream>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <tuple>

template <typename T>
struct type_name_traits;
 
#define REGISTER_TYPE(TYPE) \
template <> \
struct type_name_traits<TYPE> \
{ static const char* name; \
}; const char* type_name_traits<TYPE>::name = #TYPE \

REGISTER_TYPE(int);
REGISTER_TYPE(double);
REGISTER_TYPE(char);
REGISTER_TYPE(bool);
REGISTER_TYPE(void);
REGISTER_TYPE(std::string);
 
template <std::size_t idx, class... Types>
struct extract {
    static_assert( idx < sizeof...( Types ), "index out of bounds" );
 
    template <std::size_t i, std::size_t n, class... Rest>
    struct extract_impl;
 
    template <std::size_t i, std::size_t n, class T, class... Rest>
    struct extract_impl<i, n, T, Rest...> {
        using type = typename extract_impl<i + 1, n, Rest...>::type;
    };
 
    template <std::size_t n, class T, class... Rest>
    struct extract_impl<n, n, T, Rest...> {
        using type = T;
    };
 
    using type = typename extract_impl<0, idx, Types...>::type;
};
 
template <class... Types>
struct type_list {};
 
template <std::size_t idx, class TypeList>
struct type_list_extract;
 
template <std::size_t idx, template <class...> class TypeList, class... Types>
struct type_list_extract<idx, TypeList<Types...>> {
    using type = typename extract<idx, Types...>::type;
};
 
template <std::size_t idx, class TypeList>
using type_list_extract_t = typename type_list_extract<idx, TypeList>::type;
 
template <typename>
struct type_list_size {};
 
template <typename... Types>
struct type_list_size<type_list<Types...>> {
    enum : int {
        size = sizeof...(Types)
    };
};
 
template <std::size_t N, class List>
struct type_list_visit{
    static void visit() {
       
        // todo: this is temporary...
        std::cout << "N: " << N - 1 << " - " 
            << type_name_traits<type_list_extract_t<(N-1), List>>::name << std::endl;

        type_list_visit<N-1, List>::visit();
    }
};
 
template<class List>
struct type_list_visit<0, List> {
    static void visit() { ; }
};
 
template <typename>
struct type_list_append {};
 
template <typename... Types>
struct type_list_append<type_list<Types...>> {
    template <typename... New>
    using type = type_list<Types..., New...>;
    template<typename T>
    struct list {};
 
    template<typename... New>
    struct list<type_list<New...>> {
    	using type = type_list<Types..., New...>;
    };
};
 
template<typename R, typename... TArgs> 
struct type_list_remove_handler;
 
template<typename R>
struct type_list_remove_handler<R> {
    using type = type_list<>;
};
 
template<typename R, typename T, typename ...TArgs>
struct type_list_remove_handler<R, T, TArgs...> {
    using type = typename std::conditional<
        std::is_same<R, T>::value, 
        typename type_list_remove_handler<R, TArgs...>::type, 
        typename type_list_append<type_list<T>>::template 
            list<typename type_list_remove_handler<R, TArgs...>::type>::type
    >::type;
};
 
template <typename, typename>
struct type_list_remove {};
 
template <typename T, typename... Types>
struct type_list_remove<type_list<Types...>, T> {
	using type = typename type_list_remove_handler<T, Types...>::type;
};

template <typename, typename>
struct type_list_has_type {};

template <typename T, typename... Types>
struct type_list_has_type<type_list<Types...>, T> {
    using in = type_list<Types...>;
    using out = typename type_list_remove<type_list<Types...>, T>::type;

    using type = typename std::conditional<type_list_size<in>::size != type_list_size<out>::size, 
        std::true_type, std::false_type>::type;
};

int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;

    using list_t = type_list<char, bool, void>;

    static_assert( type_list_size<list_t>::size == 3 , "!" );
    std::cout << type_list_size<list_t>::size << std::endl;

    type_list_visit<type_list_size<list_t>::size, list_t>::visit();

    static_assert( std::is_same<char, type_list_extract_t<0, list_t>>::value, "!" );
    static_assert( std::is_same<bool, type_list_extract_t<1, list_t>>::value, "!" );
    static_assert( std::is_same<void, type_list_extract_t<2, list_t>>::value, "!" );

    using list_t2 = type_list_append<list_t>::type<double, std::string>;
    
    static_assert( type_list_size<list_t2>::size == 5, "!" );
    std::cout << type_list_size<list_t2>::size << std::endl;

    type_list_visit<type_list_size<list_t2>::size, list_t2>::visit();

    static_assert( std::is_same<char, type_list_extract_t<0, list_t2>>::value, "!" );
    static_assert( std::is_same<bool, type_list_extract_t<1, list_t2>>::value, "!" );
    static_assert( std::is_same<void, type_list_extract_t<2, list_t2>>::value, "!" );
    static_assert( std::is_same<double, type_list_extract_t<3, list_t2>>::value, "!" );
    static_assert( std::is_same<std::string, type_list_extract_t<4, list_t2>>::value, "!" );

    using list_t3 = type_list_remove<list_t2, void>::type;

    std::cout << type_list_size<list_t3>::size << std::endl;
    type_list_visit<type_list_size<list_t3>::size, list_t3>::visit();

    static_assert( std::is_same<std::false_type, type_list_has_type<list_t3, float>::type>::value, "!" );
    static_assert( std::is_same<std::false_type, type_list_has_type<list_t3, int>::type>::value, "!" );
    static_assert( std::is_same<std::true_type, type_list_has_type<list_t3, double>::type>::value, "!" );
    static_assert( std::is_same<std::true_type, type_list_has_type<list_t3, std::string>::type>::value, "!" );

    return 0;
}
