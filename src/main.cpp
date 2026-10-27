
#include "llvm/IR/DerivedTypes.h"
#include <chrono>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <memory>
#include <print>

auto time(auto &&func) -> std::chrono::nanoseconds {
  auto start = std::chrono::steady_clock::now();
  func();
  return std::chrono::steady_clock::now() - start;
}

auto main() -> int {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  auto check = llvm::ExitOnError{};

  auto jit_builder = check(llvm::orc::LLJITBuilder().create());

  auto ctx = std::make_unique<llvm::LLVMContext>();
  auto module = std::make_unique<llvm::Module>("module", *ctx);

  module->setDataLayout(jit_builder->getDataLayout());
  module->setTargetTriple(jit_builder->getTargetTriple());

  auto builder = llvm::IRBuilder<>{*ctx};

  auto *int_type = builder.getInt32Ty();
  // int -> int -> int
  auto *sig = llvm::FunctionType::get(int_type, {int_type, int_type}, false);

  auto *fn = llvm::Function::Create(sig, llvm::Function::ExternalLinkage, "add",
                                    *module);

  auto *entry = llvm::BasicBlock::Create(*ctx, "entry", fn);

  builder.SetInsertPoint(entry);

  auto begin = fn->arg_begin();

  auto *a = begin;
  auto *b = begin + 1;
  auto *result = builder.CreateAdd(a, b, "test", true, true);
  auto squared = builder.CreateMul(result, result, "mul", true, true);
  builder.CreateRet(squared);

  // 5. Verify the generated IR.
  if (verifyFunction(*fn, &llvm::errs())) {
    return 1;
  }

  module->print(llvm::outs(), nullptr);
  check(jit_builder->addIRModule(
      llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx))));

  auto symbol = check(jit_builder->lookup("add"));

  auto func = symbol.toPtr<int (*)(int, int)>();

  auto native_func = +[](int x, int y) {
    auto res = x + y;
    return res * res;
  };

  constexpr auto num_times_iter = 20000;
  std::println("time for native: {}",
               time([native_func]() {
                 volatile size_t res = 0;
                 for (auto i{0uz}; i < num_times_iter; ++i) {
                   res += native_func(i, num_times_iter - i);
                 }
               }) / static_cast<double>(num_times_iter));

  std::println("time for jit: {}", time([func]() {
                                     volatile size_t res = 0;
                                     for (auto i{0uz}; i < num_times_iter;
                                          ++i) {
                                       res += func(i, num_times_iter - i);
                                     }
                                   }) / static_cast<double>(num_times_iter));
}
