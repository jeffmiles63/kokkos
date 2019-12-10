/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 2.0
//              Copyright (2014) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Christian R. Trott (crtrott@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#ifndef KOKKOS_VIEWHOOKS_HPP
#define KOKKOS_VIEWHOOKS_HPP

#include <Kokkos_Macros.hpp>

#if defined(KOKKOS_ACTIVE_EXECUTION_MEMORY_SPACE_HOST)

#include <functional>
#include <memory>
#include <type_traits>

namespace Kokkos {
template <class DataType, class... Properties>
class View;

class ViewHolderBase {
 public:
  virtual size_t span() const                = 0;
  virtual bool span_is_contiguous() const    = 0;
  virtual const void *data() const           = 0;
  virtual std::string label() const noexcept = 0;

  virtual ViewHolderBase *clone() const = 0;
  virtual size_t data_type_size() const = 0;
};

template <typename View>
class ViewHolder : public ViewHolderBase {
 public:
  using view_type = View;
  explicit ViewHolder(View &view) : m_view(&view) {}

  size_t span() const override { return m_view->span(); }
  bool span_is_contiguous() const override {
    return m_view->span_is_contiguous();
  }
  const void *data() const override { return m_view->data(); };

  ViewHolder *clone() const override { return new ViewHolder(*this); }

  std::string label() const noexcept override { return m_view->label(); }
  size_t data_type_size() const noexcept override {
    return sizeof(typename View::value_type);
  }

 private:
  View *m_view;
};

struct ViewHooks {
  using callback_type = std::function<void(ViewHolderBase &)>;

  template <typename F, typename ConstF>
  static void set(F &&fun, ConstF &&const_fun) {
    s_callback       = std::forward<F>(fun);
    s_const_callback = std::forward<ConstF>(const_fun);
  }

  static void clear() {
    s_callback       = callback_type{};
    s_const_callback = callback_type{};
  }

  static bool is_set() noexcept {
    return static_cast<bool>(s_callback) || static_cast<bool>(s_const_callback);
  }

  template <class DataType, class... Properties>
  static void call(View<DataType, Properties...> &view) {
    auto holder = ViewHolder<View<DataType, Properties...> >(view);

    do_call(holder);
  }

 private:
  template <class ViewHolderType>
  static typename std::enable_if<
      !std::is_const<typename ViewHolderType::view_type::value_type>::value,
      void>::type
  do_call(ViewHolderType &view) {
    if (s_callback) s_callback(view);
  }

  template <class ViewHolderType>
  static typename std::enable_if<
      std::is_const<typename ViewHolderType::view_type::value_type>::value,
      void>::type
  do_call(ViewHolderType &view) {
    if (s_const_callback) s_const_callback(view);
  }

  static callback_type s_callback;
  static callback_type s_const_callback;
};

}  // namespace Kokkos

#endif

#endif  // KOKKOS_VIEWHOOKS_HPP
