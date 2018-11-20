#include <iostream>
#include <stdexcept>
#include <jni.h>

#include "initjcc.h"
#include "jccutils.h"

using namespace java::lang;

void describeAndClearJavaException(JavaError e) {
    std::cout << "Java error occurred: " << std::endl;
    describeJavaException(); clearJavaException();
}

void rethrowJavaException(JavaError e) {
    describeAndClearJavaException(e);
    throw std::runtime_error("a java error occurred; details were printed");
}

jstring fromUTF(const char *bytes) {
    return vm_env->NewStringUTF(bytes);
}
String StringFromUTF(const char *bytes) {
    return String(fromUTF(bytes));
}

JArray<String> new_JArray(const char *items[], int n) {
    String *itemsS = new String[n];
    for (int k=0; k < n; k++) itemsS[k] = StringFromUTF(items[k]);
    JArray<String> result = new_JArray<String>(itemsS, n);
    delete[] itemsS;
    return result;
}
