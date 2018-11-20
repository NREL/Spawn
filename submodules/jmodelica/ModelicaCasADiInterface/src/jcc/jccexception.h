#ifndef __jccexception_h
#define __jccexception_h

typedef int JavaError; // Hopefully JCC will use a better type some day

void describeAndClearJavaException(JavaError e);
void rethrowJavaException(JavaError e);

#endif
