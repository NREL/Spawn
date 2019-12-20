add_executable(test_compiler test_compiler.cpp)
target_link_libraries(test_compiler compiler LLVMOrcJIT)

