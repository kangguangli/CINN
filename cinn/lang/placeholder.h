#pragma once
#include <string>
#include <vector>

#include "cinn/common/common.h"
#include "cinn/ir/buffer.h"
#include "cinn/ir/ir.h"
#include "cinn/ir/ir_printer.h"
#include "cinn/ir/operation.h"
#include "cinn/lang/tensor.h"
#include "cinn/runtime/intrinsic.h"

namespace cinn {
namespace lang {

using ir::Expr;

/**
 * Placeholder
 * @tparam T
 */
template <typename T>
class Placeholder {
 public:
  Placeholder(const std::string &name, const std::vector<int> &shape);
  Placeholder(const std::string &name, const std::vector<Expr> &shape);

  //! Get a slice.
  // @{
  Expr operator()(Expr a) const { return Call({a}); }
  Expr operator()(Expr a, Expr b) const { return Call({a, b}); }
  Expr operator()(Expr a, Expr b, Expr c) const { return Call({a, b, c}); }
  Expr operator()(Expr a, Expr b, Expr c, Expr d) const { return Call({a, b, c, d}); }
  // @}

  const Type &type() const { return tensor_->type(); }

  operator ir::Tensor() { return tensor_; }
  operator ir::Expr() { return Expr(tensor_); }

 private:
  Expr operator()(const std::vector<Expr> &indices) const;

  Expr Call(const std::vector<Expr> &indices) const;

  void Init(const std::string &name, const std::vector<Expr> &shape) {
    ir::Var buffer_ptr(Context::Global().NewName("buffer"));
    buffer_ptr->set_type(type_of<T>());

    std::vector<Expr> strides(shape.size(), Expr(1));
    Expr offset(0);

    std::vector<ir::Var> axis;
    for (int i = 0; i < shape.size(); i++) axis.emplace_back(common::axis_name(i));

    auto op = ir::PlaceholderOp::Make(name, shape, type_of<T>());

    tensor_ = ir::_Tensor_::Make(name, type_of<T>(), shape, shape, op, {});
    Buffer buffer(tensor_->type());
    tensor_->Bind(buffer);
  }

  ir::Tensor tensor_;
};

template <typename T>
Expr Placeholder<T>::operator()(const std::vector<Expr> &indices) const {
  return tensor_(indices);
}

template <typename T>
Expr Placeholder<T>::Call(const std::vector<Expr> &indices) const {
  return tensor_(indices);
}

template <typename T>
Placeholder<T>::Placeholder(const std::string &name, const std::vector<int> &shape) {
  std::vector<Expr> _shape;
  for (int v : shape) _shape.push_back(Expr(v));
  Init(name, _shape);
}

template <typename T>
Placeholder<T>::Placeholder(const std::string &name, const std::vector<Expr> &shape) {
  Init(name, shape);
}

ir::Tensor CreatePlaceHolder(const std::vector<Expr> &shape, Type type, const std::string &name);

}  // namespace lang
}  // namespace cinn
