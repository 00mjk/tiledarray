/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2015  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Justus Calvin
 *  Department of Chemistry, Virginia Tech
 *
 *  tensor.h
 *  Jun 16, 2015
 *
 */

#ifndef TILEDARRAY_TENSOR_H__INCLUDED
#define TILEDARRAY_TENSOR_H__INCLUDED

#include <TiledArray/tensor/tensor.h>
#include <TiledArray/tensor/tensor_map.h>
#include <TiledArray/tensor/tensor_interface.h>
#include <TiledArray/tensor/shift_wrapper.h>
#include <TiledArray/tensor/operators.h>
#include <TiledArray/block_range.h>

namespace TiledArray {

  // Template aliases for TensorInterface objects

  template <typename T>
  using TensorView =
      detail::TensorInterface<T, BlockRange>;

  template <typename T>
  using TensorConstView =
      detail::TensorInterface<typename std::add_const<T>::type, BlockRange>;


  /// Tensor output operator

  /// Ouput tensor \c t to the output stream, \c os .
  /// \tparam T The tensor type
  /// \param os The output stream
  /// \param t The tensor to be output
  /// \return A reference to the output stream
  template <typename T,
      typename std::enable_if<
          detail::is_tensor<T>::value &&
          detail::is_contiguous_tensor<T>::value>::type* = nullptr>
  inline std::ostream& operator<<(std::ostream& os, const T& t) {
    os << t.range() << " { ";
    const auto n = t.range().volume();
    for(auto i = 0ul; i < n; ++i)
      os << t[i] << " ";

    os << "}";

    return os;
  }

  /// Tensor output operator

  /// Ouput tensor \c t to the output stream, \c os .
  /// \tparam T The tensor type
  /// \param os The output stream
  /// \param t The tensor to be output
  /// \return A reference to the output stream
  template <typename T,
      typename std::enable_if<
          detail::is_tensor<T>::value &&
          ! detail::is_contiguous_tensor<T>::value>::type* = nullptr>
  inline std::ostream& operator<<(std::ostream& os, const T& t) {

    const auto stride = inner_size(t);
    const auto volume = t.range().volume();

    auto tensor_print_range =
        [&os,stride] (typename T::const_pointer restrict const t_data) {
          for(decltype(t.range().volume()) i = 0ul; i < stride; ++i)
            os << t_data[i] << " ";
        };

    os << t.range() << " { ";

    for(decltype(t.range().volume()) i = 0ul; i < volume; i += stride)
      tensor_print_range(t.data() + t.range().ordinal(i));

    os << "}";

    return os;
  }

} // namespace TiledArray

#endif // TILEDARRAY_SRC_TILEDARRAY_TENSOR_H__INCLUDED
