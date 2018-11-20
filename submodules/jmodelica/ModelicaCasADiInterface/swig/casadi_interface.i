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

%module modelicacasadi_wrapper

%include "Ref.i" // Must be before %include "std_vector.i". Includes Ref.hpp
%include "vectors.i"

%include "std_string.i"
%include "std_vector.i"
%include "exception.i"

%import "casadi_core.i"


%{
#include <exception>
#include "jccexception.h"
%}

%exception {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const char* e) {
        SWIG_exception(SWIG_RuntimeError, e);
    } catch (JavaError e) {
        describeAndClearJavaException(e);
        SWIG_exception(SWIG_RuntimeError, "a java error occurred; details were printed");
    }
}


%include "ModelicaCasADi.i"


// Pull in numpy
%{
// to perhaps play more nicely with numpy.i
#define SWIG_FILE_WITH_INIT
%}
%init %{
// initialize numpy, should only be done once?
import_array();
%}
