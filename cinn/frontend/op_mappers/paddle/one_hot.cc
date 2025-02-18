// Copyright (c) 2021 CINN Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cinn/frontend/op_mapper_registry.h"
#include "cinn/frontend/op_mappers/common_utils.h"
#include "cinn/frontend/var_type_utils.h"

namespace cinn {
namespace frontend {
namespace paddle_mappers {

void OneHotV2OpMapper(const paddle::cpp::OpDesc& op_desc, const OpMapperContext& ctx) {
  CHECK_EQ(op_desc.Input("X").size(), 1UL);
  auto x_name = op_desc.Input("X").front();
  CHECK_EQ(op_desc.Output("Out").size(), 1UL);
  auto out_name = op_desc.Output("Out").front();

  auto depth = utils::GetAttrOrDefault<int>(op_desc, "depth", 1);
  auto axis  = utils::GetAttrOrDefault<int>(op_desc, "axis", -1);

  auto on_value  = ctx.Builder()->FillConstant({1}, 1, cinn::UniqName(x_name + "_on_value"), "int32");
  auto off_value = ctx.Builder()->FillConstant({1}, 0, cinn::UniqName(x_name + "_off_value"), "int32");
  //   auto data_format = utils::GetAttrOrDefault<std::string>(op_desc, "data_format", "AnyLayout");

  auto dtype_id = utils::GetAttrOrDefault<int>(op_desc, "dtype", static_cast<int>(paddle::cpp::VarDescAPI::Type::FP32));
  auto dtype_pd = static_cast<paddle::cpp::VarDescAPI::Type>(dtype_id);
  auto dtype_cinn = utils::CppVarType2CommonType(dtype_pd);
  auto dtype      = common::Type2Str(dtype_cinn);

  auto x   = ctx.GetVar(x_name);
  auto out = ctx.Builder()->OneHot(x, on_value, off_value, depth, axis, dtype);
  ctx.AddVar(out_name, out);
  ctx.AddVarModelToProgram(out_name, out->id);
}

}  // namespace paddle_mappers
}  // namespace frontend
}  // namespace cinn

CINN_REGISTER_HELPER(paddle_one_hot) {
  CINN_REGISTER_OP_MAPPER(one_hot_v2, cinn::frontend::paddle_mappers::OneHotV2OpMapper)
  return true;
}