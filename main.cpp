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

#include <cassert>
#include <variant>

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
using type_list_element = typename type_list_extract<idx, TypeList>::type;
 
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
            << type_name_traits<type_list_element<(N-1), List>>::name << std::endl;

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



template <typename>
struct variant_wrap {};

template <typename... Types>
struct variant_wrap<type_list<Types...>> {
    using type = std::variant<Types...>;
};



template <class TypeList, std::size_t Index = type_list_size<TypeList>::size>
struct type_array_visitor{
    template <typename Array>
    static void visit(Array && array) {
        try {
            // code...
            std::cout << std::get< type_list_element<Index-1, TypeList> >( std::get<Index-1>(array) ) << std::endl;
            
            type_array_visitor<TypeList, Index-1>::visit(array);
        } catch(...) {
            // handle variant execption...
        }
    }
};

template <class TypeList>
struct type_array_visitor<TypeList, 1>{
    template <typename Array>
    static void visit(Array && array) {
        std::cout << std::get< type_list_element<0, TypeList> >( std::get<0>(array) ) << std::endl;
    }
};



int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;

    /* type list tests */

    using list_t = type_list<char, bool, void>;

    static_assert( type_list_size<list_t>::size == 3 , "!" );
    std::cout << type_list_size<list_t>::size << std::endl;

    type_list_visit<type_list_size<list_t>::size, list_t>::visit();

    static_assert( std::is_same<char, type_list_element<0, list_t>>::value, "!" );
    static_assert( std::is_same<bool, type_list_element<1, list_t>>::value, "!" );
    static_assert( std::is_same<void, type_list_element<2, list_t>>::value, "!" );

    using list_t2 = type_list_append<list_t>::type<double, std::string>;
    
    static_assert( type_list_size<list_t2>::size == 5, "!" );
    std::cout << type_list_size<list_t2>::size << std::endl;

    type_list_visit<type_list_size<list_t2>::size, list_t2>::visit();

    static_assert( std::is_same<char, type_list_element<0, list_t2>>::value, "!" );
    static_assert( std::is_same<bool, type_list_element<1, list_t2>>::value, "!" );
    static_assert( std::is_same<void, type_list_element<2, list_t2>>::value, "!" );
    static_assert( std::is_same<double, type_list_element<3, list_t2>>::value, "!" );
    static_assert( std::is_same<std::string, type_list_element<4, list_t2>>::value, "!" );

    using list_t3 = type_list_remove<list_t2, void>::type;

    std::cout << type_list_size<list_t3>::size << std::endl;
    type_list_visit<type_list_size<list_t3>::size, list_t3>::visit();

    static_assert( std::is_same<std::false_type, type_list_has_type<list_t3, float>::type>::value, "!" );
    static_assert( std::is_same<std::false_type, type_list_has_type<list_t3, int>::type>::value, "!" );
    static_assert( std::is_same<std::true_type, type_list_has_type<list_t3, double>::type>::value, "!" );
    static_assert( std::is_same<std::true_type, type_list_has_type<list_t3, std::string>::type>::value, "!" );

    /* variant example */

    using variant_helper_list = type_list<int, double, std::string>;
    using variant_type = variant_wrap<variant_helper_list>::type;
    using type_array = std::array<variant_type, type_list_size<variant_helper_list>::size>;
    type_array variant_instance;
    variant_instance[0] = 1;
    variant_instance[1] = 3.14;
    variant_instance[2] = "string";

    type_array_visitor<variant_helper_list>::visit(variant_instance);

    // todo: variant of optionals

    return 0;
}
