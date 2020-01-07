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

#ifndef NREL_SPAWN_HIT_HPP
#define NREL_SPAWN_HIT_HPP

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

namespace spawn {

  class JIT
  {
  private:
    llvm::orc::ExecutionSession ES;
    std::unique_ptr<llvm::TargetMachine> TM;
    const llvm::DataLayout DL;
    llvm::orc::MangleAndInterner Mangle{ES, DL};
    llvm::orc::RTDyldObjectLinkingLayer ObjectLayer{ES, createMemMgr};
    llvm::orc::IRCompileLayer CompileLayer{ES, ObjectLayer, llvm::orc::SimpleCompiler(*TM)};

    static std::unique_ptr<llvm::SectionMemoryManager> createMemMgr()
    {
      return llvm::make_unique<llvm::SectionMemoryManager>();
    }

    JIT(std::unique_ptr<llvm::TargetMachine> TM, llvm::DataLayout DL, llvm::orc::DynamicLibrarySearchGenerator ProcessSymbolsGenerator)
        : TM(std::move(TM)), DL(std::move(DL))
    {
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
      ES.getMainJITDylib().setGenerator(std::move(ProcessSymbolsGenerator));
    }

  public:
    static llvm::Expected<std::unique_ptr<JIT>> Create()
    {
      auto JTMB = llvm::orc::JITTargetMachineBuilder::detectHost();
      if (!JTMB) return JTMB.takeError();

      auto TM = JTMB->createTargetMachine();
      if (!TM) return TM.takeError();

      auto DL = (*TM)->createDataLayout();

      auto ProcessSymbolsGenerator = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix());

      if (!ProcessSymbolsGenerator) return ProcessSymbolsGenerator.takeError();
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
      return std::unique_ptr<JIT>(new JIT(std::move(*TM), std::move(DL), std::move(*ProcessSymbolsGenerator)));
    }

    const llvm::TargetMachine &getTargetMachine() const
    {
      return *TM;
    }

    void addModule(std::unique_ptr<llvm::Module> module, llvm::orc::ThreadSafeContext ctx)
    {
      auto err = CompileLayer.add(ES.getMainJITDylib(), llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx)));
      if (err) {
        throw err;
      }
    }

    llvm::Expected<llvm::JITEvaluatedSymbol> findSymbol(const llvm::StringRef &Name)
    {
      return ES.lookup({&ES.getMainJITDylib()}, Mangle(Name));
    }

    template <typename FunctionSig> auto get_function(const llvm::StringRef &name)
    {
      auto sa = getSymbolAddress(name);
      if (sa) {
        return reinterpret_cast<std::add_pointer_t<FunctionSig>>(sa.get());
      } else {
        throw std::runtime_error(std::string("Unable to find symbol: ") + std::string(name));
      }
    }

    llvm::Expected<llvm::JITTargetAddress> getSymbolAddress(const llvm::StringRef &Name)
    {
      auto Sym = findSymbol(Name);
      if (!Sym) return Sym.takeError();
      return Sym->getAddress();
    }
  };

} // end namespace spawn

#endif // NREL_SPAWN_HIT_HPP
