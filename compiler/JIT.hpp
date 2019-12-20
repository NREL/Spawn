//===- JIT.h - A simple JIT for Kaleidoscope --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Contains a simple JIT definition for use in the kaleidoscope tutorials.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
#define LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>

namespace llvm {
namespace orc {

class SimpleJIT {
private:
  ExecutionSession ES;
  std::unique_ptr<TargetMachine> TM;
  const DataLayout DL;
  MangleAndInterner Mangle{ES, DL};
  RTDyldObjectLinkingLayer ObjectLayer{ES, createMemMgr};
  IRCompileLayer CompileLayer{ES, ObjectLayer, SimpleCompiler(*TM)};

  static std::unique_ptr<SectionMemoryManager> createMemMgr() {
    return llvm::make_unique<SectionMemoryManager>();
  }

  SimpleJIT(std::unique_ptr<TargetMachine> TM, DataLayout DL,
            DynamicLibrarySearchGenerator ProcessSymbolsGenerator)
      : TM(std::move(TM)), DL(std::move(DL)) {
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    ES.getMainJITDylib().setGenerator(std::move(ProcessSymbolsGenerator));
  }

public:
  static Expected<std::unique_ptr<SimpleJIT>> Create() {
    auto JTMB = JITTargetMachineBuilder::detectHost();
    if (!JTMB)
      return JTMB.takeError();

    auto TM = JTMB->createTargetMachine();
    if (!TM)
      return TM.takeError();

    auto DL = (*TM)->createDataLayout();

    auto ProcessSymbolsGenerator =
        DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix());

    if (!ProcessSymbolsGenerator)
      return ProcessSymbolsGenerator.takeError();

    return std::unique_ptr<SimpleJIT>(new SimpleJIT(
        std::move(*TM), std::move(DL), std::move(*ProcessSymbolsGenerator)));
  }

  const TargetMachine &getTargetMachine() const { return *TM; }

  Error addModule(std::unique_ptr<llvm::Module> module, llvm::orc::ThreadSafeContext ctx) {
    return CompileLayer.add(ES.getMainJITDylib(), ThreadSafeModule(std::move(module), std::move(ctx)));
  }

  Expected<JITEvaluatedSymbol> findSymbol(const StringRef &Name) {
    return ES.lookup({&ES.getMainJITDylib()}, Mangle(Name));
  }

  Expected<JITTargetAddress> getSymbolAddress(const StringRef &Name) {
    auto Sym = findSymbol(Name);
    if (!Sym)
      return Sym.takeError();
    return Sym->getAddress();
  }
};


} // end namespace orc
} // end namespace llvm

#endif // LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
