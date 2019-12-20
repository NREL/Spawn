add_executable(test_compiler test_compiler.cpp)
target_link_libraries(test_compiler compiler LLVMOrcJIT)

add_test(NAME test_compiler COMMAND test_compiler ${CMAKE_SOURCE_DIR}/test/go.c
                                    ${CMAKE_SOURCE_DIR}/test/func.c)
