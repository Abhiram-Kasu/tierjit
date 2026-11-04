#pragma once

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include <type_traits>

namespace tierjit::util {

struct AddFuncTemplateOptions {
  llvm::Type &arg1;
  llvm::Type &arg2;
  llvm::Type &result_type;
  std::optional<std::string_view> function_name;
};
inline auto add_func_template(const llvm::IRBuilder<> &builder,
                              AddFuncTemplateOptions options)
    -> llvm::Function * {
  const static auto function_ty = [&] {
    auto *func_type = llvm::FunctionType::get(
        &options.result_type, {&options.arg1, &options.arg2}, false);
    if (options.function_name) {
      return llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                    *options.function_name);
    }
    return llvm::Function::Create(func_type, llvm::Function::ExternalLinkage);
  };

  return function_ty();
}
} // namespace tierjit::util
