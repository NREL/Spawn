/*
Copyright (C) 2013 Modelon AB

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//JNI
#include "jni.h"
#include "jccutils.h"

#include "java/util/Collection.h"
#include "java/util/Iterator.h"
#include "java/lang/String.h"
#include <string>
#include "JCCEnv.h"

#include "CompilerOptionsWrapper.hpp"


namespace ModelicaCasADi 
{
void CompilerOptionsWrapper::setStringOption(std::string opt, std::string val) {
    try 
    {
        optr.setStringOption(StringFromUTF(opt.c_str()), StringFromUTF(val.c_str()));
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }
}
void CompilerOptionsWrapper::setBooleanOption(std::string opt, bool val) {
    try 
    {
    optr.setBooleanOption(StringFromUTF(opt.c_str()), val);
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }
}
void CompilerOptionsWrapper::setIntegerOption(std::string opt, int val) {
    try 
    {
    optr.setIntegerOption(StringFromUTF(opt.c_str()), val);
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }
}
void CompilerOptionsWrapper::setRealOption(std::string opt, double val) {
    try 
    {
    optr.setRealOption(StringFromUTF(opt.c_str()), val);
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }
}

bool CompilerOptionsWrapper::getBooleanOption(std::string opt) {
    bool roption;    
    try 
    {
        roption = optr.getBooleanOption(StringFromUTF(opt.c_str()));
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }
    return roption;
}

void CompilerOptionsWrapper::printCompilerOptions(std::ostream& out){
    try 
    {
        java::util::Collection opts(optr.getOptionKeys().this$);
        
        java::util::Iterator iter(opts.iterator().this$);
        while(iter.hasNext()){
            java::lang::String key(iter.next().this$);
            out <<"\033[31m"<<env->toString(key.this$) <<"\033[0m"<< ": ";            
            out << env->toString(optr.getDescription(key).this$);
            out << "\n";
        }
        
    }
    catch (JavaError e) 
    {
        rethrowJavaException(e);
    }       
}


void CompilerOptionsWrapper::print(std::ostream& os) const { os << "CompilerOptionsWrapper(" << env->toString(optr.this$) << ")"; }
}; // End namespace
