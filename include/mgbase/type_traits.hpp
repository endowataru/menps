
#pragma once

// 20.9.3, helper class:
#include <mgbase/type_traits/integral_constant.hpp>

// 20.9.4.1, primary type categories:
#include <mgbase/type_traits/is_void.hpp>
#include <mgbase/type_traits/is_integral.hpp>
#include <mgbase/type_traits/is_floating_point.hpp>
#include <mgbase/type_traits/is_array.hpp>
#include <mgbase/type_traits/is_pointer.hpp>
#include <mgbase/type_traits/is_lvalue_reference.hpp>
#include <mgbase/type_traits/is_rvalue_reference.hpp>
//#include <mgbase/type_traits/is_member_object_pointer.hpp>
//#include <mgbase/type_traits/is_member_function_pointer.hpp>
//#include <mgbase/type_traits/is_enum.hpp>
//#include <mgbase/type_traits/is_union.hpp>
//#include <mgbase/type_traits/is_class.hpp>
#include <mgbase/type_traits/is_function.hpp>

// 20.9.4.2, composite type categories:
#include <mgbase/type_traits/is_reference.hpp>
#include <mgbase/type_traits/is_arithmetic.hpp>
#include <mgbase/type_traits/is_fundamental.hpp>
//#include <mgbase/type_traits/is_object.hpp>
//#include <mgbase/type_traits/is_scalar.hpp>
#include <mgbase/type_traits/is_compound.hpp>
//#include <mgbase/type_traits/is_member_pointer.hpp>

// 20.9.4.3, type properties:
#include <mgbase/type_traits/is_const.hpp>
//#include <mgbase/type_traits/is_volatile.hpp>
//#include <mgbase/type_traits/is_trivial.hpp>
#include <mgbase/type_traits/is_trivially_copyable.hpp>
//#include <mgbase/type_traits/is_standard_layout.hpp>
//#include <mgbase/type_traits/is_pod.hpp>
//#include <mgbase/type_traits/is_literal_type.hpp>
//#include <mgbase/type_traits/is_empty.hpp>
//#include <mgbase/type_traits/is_polymorphic.hpp>
//#include <mgbase/type_traits/is_abstract.hpp>
//#include <mgbase/type_traits/is_signed.hpp>
//#include <mgbase/type_traits/is_unsigned.hpp>
//#include <mgbase/type_traits/is_constructible.hpp>
//#include <mgbase/type_traits/is_default_constructible.hpp>
//#include <mgbase/type_traits/is_copy_constructible.hpp>
//#include <mgbase/type_traits/is_move_constructible.hpp>
//#include <mgbase/type_traits/is_assignable.hpp>
//#include <mgbase/type_traits/is_copy_assignable.hpp>
//#include <mgbase/type_traits/is_move_assignable.hpp>
//#include <mgbase/type_traits/is_destructible.hpp>
//#include <mgbase/type_traits/is_trivially_constructible.hpp>
//#include <mgbase/type_traits/is_trivially_default_constructible.hpp>
//#include <mgbase/type_traits/is_trivially_copy_constructible.hpp>
//#include <mgbase/type_traits/is_trivially_move_constructible.hpp>
//#include <mgbase/type_traits/is_trivially_assignable.hpp>
//#include <mgbase/type_traits/is_trivially_copy_assignable.hpp>
//#include <mgbase/type_traits/is_trivially_move_assignable.hpp>
#include <mgbase/type_traits/is_trivially_destructible.hpp>
//#include <mgbase/type_traits/is_nothrow_constructible.hpp>
//#include <mgbase/type_traits/is_nothrow_default_constructible.hpp>
//#include <mgbase/type_traits/is_nothrow_copy_constructible.hpp>
//#include <mgbase/type_traits/is_nothrow_move_constructible.hpp>
//#include <mgbase/type_traits/is_nothrow_assignable.hpp>
//#include <mgbase/type_traits/is_nothrow_copy_assignable.hpp>
//#include <mgbase/type_traits/is_nothrow_move_assignable.hpp>
//#include <mgbase/type_traits/is_nothrow_destructible.hpp>
//#include <mgbase/type_traits/has_virtual_destructor.hpp>

// 20.9.5, type property queries:
#include <mgbase/type_traits/alignment_of.hpp>
//#include <mgbase/type_traits/rank.hpp>
//#include <mgbase/type_traits/extent.hpp>

// 20.9.6, type relations:
#include <mgbase/type_traits/is_same.hpp>
//#include <mgbase/type_traits/is_base_of.hpp>
#include <mgbase/type_traits/is_convertible.hpp>

// 20.9.7.1, const-volatile modifications:
#include <mgbase/type_traits/remove_const.hpp>
#include <mgbase/type_traits/remove_volatile.hpp>
#include <mgbase/type_traits/remove_cv.hpp>
//#include <mgbase/type_traits/add_const.hpp>
//#include <mgbase/type_traits/add_volatile.hpp>
//#include <mgbase/type_traits/add_cv.hpp>

// 20.9.7.2, reference modifications:
#include <mgbase/type_traits/remove_reference.hpp>
//#include <mgbase/type_traits/add_lvalue_reference.hpp>
#include <mgbase/type_traits/add_rvalue_reference.hpp>

// 20.9.7.3, sign modifications:
#include <mgbase/type_traits/make_signed.hpp>
#include <mgbase/type_traits/make_unsigned.hpp>

// 20.9.7.4, array modifications:
#include <mgbase/type_traits/remove_extent.hpp>
//#include <mgbase/type_traits/remove_all_extents.hpp>

// 20.9.7.5, pointer modifications:
//#include <mgbase/type_traits/remove_pointer.hpp>
#include <mgbase/type_traits/add_pointer.hpp>

// 20.9.7.6, other transformations:
//#include <mgbase/type_traits/aligned_storage.hpp>
//#include <mgbase/type_traits/aligned_union.hpp>
#include <mgbase/type_traits/decay.hpp>
#include <mgbase/type_traits/enable_if.hpp>
#include <mgbase/type_traits/conditional.hpp>
//#include <mgbase/type_traits/common_type.hpp>
//#include <mgbase/type_traits/underlying_type.hpp>
#include <mgbase/type_traits/result_of.hpp>

