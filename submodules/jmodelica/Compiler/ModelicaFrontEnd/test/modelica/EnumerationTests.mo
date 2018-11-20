/*
    Copyright (C) 2009-2013 Modelon AB

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

package EnumerationTests

  model EnumerationTest1
    type Size = enumeration(small "1st", medium, large, xlarge); 
	parameter Size t_shirt_size = Size.medium; 

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest1",
			description="Test basic use of enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest1
 parameter EnumerationTests.EnumerationTest1.Size t_shirt_size = EnumerationTests.EnumerationTest1.Size.medium;

public
 type EnumerationTests.EnumerationTest1.Size = enumeration(small \"1st\", medium, large, xlarge);

end EnumerationTests.EnumerationTest1;
")})));
  end EnumerationTest1;

  
  model EnumerationTest2
    type Size = enumeration(small "1st", medium, large, xlarge); 
	  
    model A
      parameter Size t_shirt_size(start = Size.large) = Size.medium; 
	end A;
	
    A a1;
    A a2;
	parameter Size s = Size.large;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest2",
			description="Test basic use of enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest2
 parameter EnumerationTests.EnumerationTest2.Size a1.t_shirt_size(start = EnumerationTests.EnumerationTest2.Size.large) = EnumerationTests.EnumerationTest2.Size.medium /* EnumerationTests.EnumerationTest2.Size.medium */;
 parameter EnumerationTests.EnumerationTest2.Size a2.t_shirt_size(start = EnumerationTests.EnumerationTest2.Size.large) = EnumerationTests.EnumerationTest2.Size.medium /* EnumerationTests.EnumerationTest2.Size.medium */;
 parameter EnumerationTests.EnumerationTest2.Size s = EnumerationTests.EnumerationTest2.Size.large /* EnumerationTests.EnumerationTest2.Size.large */;

public
 type EnumerationTests.EnumerationTest2.Size = enumeration(small \"1st\", medium, large, xlarge);

end EnumerationTests.EnumerationTest2;
")})));
  end EnumerationTest2;


  model EnumerationTest3
    type A = enumeration(a, b, c);
    constant A x = A.b;
	parameter A y = x;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest3",
			description="Test of constant evaluation for enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest3
 constant EnumerationTests.EnumerationTest3.A x = EnumerationTests.EnumerationTest3.A.b;
 parameter EnumerationTests.EnumerationTest3.A y = EnumerationTests.EnumerationTest3.A.b;

public
 type EnumerationTests.EnumerationTest3.A = enumeration(a, b, c);

end EnumerationTests.EnumerationTest3;
")})));
  end EnumerationTest3;
  
  
  model EnumerationTest4
    type A = enumeration(a, b, c);
    type B = enumeration(a, c, b);
	parameter A x = B.a;

    annotation(__JModelica(UnitTesting(tests={
        ErrorTestCase(
            name="EnumerationTest4",
            description="Using incompatible enumerations: binding expression",
            errorMessage="
1 errors found:

Error at line 4, column 18, in file 'Compiler/ModelicaFrontEnd/test/modelica/EnumerationTests.mo', BINDING_EXPRESSION_TYPE_MISMATCH:
  The binding expression of the variable x does not match the declared type of the variable
")})));
  end EnumerationTest4;
  
  
  model EnumerationTest5
    type A = enumeration(a, b, c);
    type B = enumeration(a, c, b);
	parameter A x;
  equation
    x = B.a;

    annotation(__JModelica(UnitTesting(tests={
        ErrorTestCase(
            name="EnumerationTest5",
            description="Using incompatible enumerations: equation",
            errorMessage="
1 errors found:

Error at line 6, column 5, in file 'Compiler/ModelicaFrontEnd/test/modelica/EnumerationTests.mo', TYPE_MISMATCH_IN_EQUATION:
  The right and left expression types of equation are not compatible, type of left-hand side is EnumerationTests.EnumerationTest5.A, and type of right-hand side is EnumerationTests.EnumerationTest5.B
")})));
  end EnumerationTest5;
  
  
  model EnumerationTest6
    type A = enumeration(a, b, c);
    type B = enumeration(a, b, c);
	parameter A x = B.a;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest6",
			description="Using equivalent enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest6
 parameter EnumerationTests.EnumerationTest6.A x = EnumerationTests.EnumerationTest6.B.a;

public
 type EnumerationTests.EnumerationTest6.A = enumeration(a, b, c);

 type EnumerationTests.EnumerationTest6.B = enumeration(a, b, c);

end EnumerationTests.EnumerationTest6;
")})));
  end EnumerationTest6;
  
  model EnumerationTest7
    type A = enumeration(a, b, c);
    Real x[A] = {3,2,1};

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest7",
			description="Compliance error for using enumeration as array size",
            flatModel="
fclass EnumerationTests.EnumerationTest7
 Real x[3] = {3, 2, 1};

public
 type EnumerationTests.EnumerationTest7.A = enumeration(a, b, c);

end EnumerationTests.EnumerationTest7;

")})));
  end EnumerationTest7;
  
  
  model EnumerationTest8
	  type A = enumeration(a, b, c, d, e);
	
	  constant Boolean a[2] = false:true;
	  parameter Boolean b[2] = a;
	  constant A c[3] = A.b:A.d;
	  parameter A d[3] = c;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest8",
			description="Range expressions with Booleans and enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest8
 constant Boolean a[2] = {false, true};
 parameter Boolean b[2] = {false, true} /* { false, true } */;
 constant EnumerationTests.EnumerationTest8.A c[3] = {EnumerationTests.EnumerationTest8.A.b, EnumerationTests.EnumerationTest8.A.c, EnumerationTests.EnumerationTest8.A.d};
 parameter EnumerationTests.EnumerationTest8.A d[3] = {EnumerationTests.EnumerationTest8.A.b, EnumerationTests.EnumerationTest8.A.c, EnumerationTests.EnumerationTest8.A.d} /* { EnumerationTests.EnumerationTest8.A.b, EnumerationTests.EnumerationTest8.A.c, EnumerationTests.EnumerationTest8.A.d } */;

public
 type EnumerationTests.EnumerationTest8.A = enumeration(a, b, c, d, e);

end EnumerationTests.EnumerationTest8;
")})));
  end EnumerationTest8;
  
  
  model EnumerationTest9
	  type A = enumeration(a, b, c, d, e);
	  constant Boolean[:,:] x = {
		  { A.c <  A.b, A.c <  A.c, A.c <  A.d }, 
		  { A.c <= A.b, A.c <= A.c, A.c <= A.d }, 
		  { A.c >  A.b, A.c >  A.c, A.c >  A.d }, 
		  { A.c >= A.b, A.c >= A.c, A.c >= A.d }, 
		  { A.c == A.b, A.c == A.c, A.c == A.d }, 
		  { A.c <> A.b, A.c <> A.c, A.c <> A.d } 
		  };
	  parameter Boolean[:,:] y = x;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest9",
			description="Relational operators with enumerations",
			flatModel="
fclass EnumerationTests.EnumerationTest9
 constant Boolean x[6,3] = {{false, false, true}, {false, true, true}, {true, false, false}, {true, true, false}, {false, true, false}, {true, false, true}};
 parameter Boolean y[6,3] = {{false, false, true}, {false, true, true}, {true, false, false}, {true, true, false}, {false, true, false}, {true, false, true}} /* { { false, false, true }, { false, true, true }, { true, false, false }, { true, true, false }, { false, true, false }, { true, false, true } } */;

public
 type EnumerationTests.EnumerationTest9.A = enumeration(a, b, c, d, e);

end EnumerationTests.EnumerationTest9;
")})));
  end EnumerationTest9;
  
  
  model EnumerationTest10
	  type A = enumeration(a, b, c, d, e);
	  constant Integer i[:] = { Integer(A.a), Integer(A.c), Integer(A.e) };
	  parameter Integer j[:] = i;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest10",
			description="Using the Integer() operator: basic test",
			flatModel="
fclass EnumerationTests.EnumerationTest10
 constant Integer i[3] = {1, 3, 5};
 parameter Integer j[3] = {1, 3, 5} /* { 1, 3, 5 } */;

public
 type EnumerationTests.EnumerationTest10.A = enumeration(a, b, c, d, e);

end EnumerationTests.EnumerationTest10;
")})));
  end EnumerationTest10;
  
  
  model EnumerationTest11
	  parameter Integer is = Integer("1");
	  parameter Integer ir = Integer(1.0);
	  parameter Integer ii = Integer(1);

    annotation(__JModelica(UnitTesting(tests={
        ErrorTestCase(
            name="EnumerationTest11",
            description="Using the Integer() operator: wrong type of argument",
            errorMessage="
3 errors found:

Error at line 2, column 35, in file 'Compiler/ModelicaFrontEnd/test/modelica/EnumerationTests.mo':
  Calling function Integer(): types of positional argument 1 and input x are not compatible
    type of '\"1\"' is String
    expected type is enumeration

Error at line 3, column 35, in file 'Compiler/ModelicaFrontEnd/test/modelica/EnumerationTests.mo':
  Calling function Integer(): types of positional argument 1 and input x are not compatible
    type of '1.0' is Real
    expected type is enumeration

Error at line 4, column 35, in file 'Compiler/ModelicaFrontEnd/test/modelica/EnumerationTests.mo':
  Calling function Integer(): types of positional argument 1 and input x are not compatible
    type of '1' is Integer
    expected type is enumeration
")})));
  end EnumerationTest11;
  
  
  model EnumerationTest12
	  type DigitalCurrentChoices = enumeration(zero, one);
	  type DigitalCurrent = DigitalCurrentChoices(quantity="Current", start = DigitalCurrentChoices.one, fixed = true);
	  parameter DigitalCurrent c = DigitalCurrent.one;

	annotation(__JModelica(UnitTesting(tests={
		FlatteningTestCase(
			name="EnumerationTest12",
			description="",
			flatModel="
fclass EnumerationTests.EnumerationTest12
 parameter EnumerationTests.EnumerationTest12.DigitalCurrent c = EnumerationTests.EnumerationTest12.DigitalCurrentChoices.one /* EnumerationTests.EnumerationTest12.DigitalCurrentChoices.one */;

public
 type EnumerationTests.EnumerationTest12.DigitalCurrent = enumeration(zero, one)(quantity = \"Current\",start = EnumerationTests.EnumerationTest12.DigitalCurrentChoices.one,fixed = true);

 type EnumerationTests.EnumerationTest12.DigitalCurrentChoices = enumeration(zero, one);

end EnumerationTests.EnumerationTest12;
")})));
  end EnumerationTest12;
  
  
  
  model FlatAPIEnum1
	  type A = enumeration(a, b, c);
	  type B = enumeration(d, e, f);
	  
	  constant A aic = A.a;
	  constant B bic = B.e;
	  constant A adc = aic;
	  constant B bdc = bic;
	  parameter A aip = A.b;
	  parameter B bip = B.f;
	  parameter A adp = aip;
	  parameter B bdp = bip;
	  A av = A.c;
	  B bv = B.d;

	annotation(__JModelica(UnitTesting(tests={
		FClassMethodTestCase(
			name="FlatAPIEnum1",
			description="FlatAPI for enumerations: diagnostics",
			equation_sorting=false,
            eliminate_alias_variables=false,
			methodName="diagnostics",
			methodResult="
Diagnostics for flattened class EnumerationTests.FlatAPIEnum1
Number of independent constants:                   6(    6 scalars)
  Number of Real independent constants:            0(    0 scalars)
  Number of Real independent constants:            0(    0 scalars)
  Number of Integer independent constants:         0(    0 scalars)
  Number of Enum independent constants:            6(    6 scalars)
  Number of Boolean independent constants:         0(    0 scalars)
  Number of String independent constants:          0(    0 scalars)
Number of dependent constants:                     0(    0 scalars)
  Number of Real dependent constants:              0(    0 scalars)
  Number of Integer dependent constants:           0(    0 scalars)
  Number of Enum dependent constants:              0(    0 scalars)
  Number of Boolean dependent constants:           0(    0 scalars)
  Number of String dependent constants:            0(    0 scalars)
Number of independent parameters:                  2(    2 scalars)
  Number of Real independent parameters:           0(    0 scalars)
  Number of Integer independent parameters:        0(    0 scalars)
  Number of Enum independent parameters:           2(    2 scalars)
  Number of Boolean independent parameters:        0(    0 scalars)
  Number of String independent parameters:         0(    0 scalars)
Number of dependent parameters:                    2(    2 scalars)
  Number of Real dependent parameters:             0(    0 scalars)
  Number of Integer dependent parameters:          0(    0 scalars)
  Number of Enum dependent parameters:             2(    2 scalars)
  Number of Boolean dependent parameters:          0(    0 scalars)
  Number of String dependent parameters:           0(    0 scalars)
Number of initial parameters:                      0(    0 scalars)
  Number of Real dependent parameters:             0(    0 scalars)
  Number of Integer dependent parameters:          0(    0 scalars)
  Number of Enum dependent parameters:             0(    0 scalars)
  Number of Boolean dependent parameters:          0(    0 scalars)
  Number of String dependent parameters:           0(    0 scalars)
Number of variables:                               0(    0 scalars)
  Number of Real variables:                        0(    0 scalars)
  Number of Integer variables:                     0(    0 scalars)
  Number of Enum variables:                        0(    0 scalars)
  Number of Boolean variables:                     0(    0 scalars)
  Number of String variables:                      0(    0 scalars)
Number of Real differentiated variables:           0(    0 scalars)
Number of Real derivative variables:               0(    0 scalars)
Number of Real continous algebraic variables:      0(    0 scalars)
Number of inputs:                                  0(    0 scalars)
  Number of Real inputs:                           0(    0 scalars)
  Number of Integer inputs:                        0(    0 scalars)
  Number of Enum inputs:                           0(    0 scalars)
  Number of Boolean inputs:                        0(    0 scalars)
  Number of String inputs:                         0(    0 scalars)
Number of discrete variables:                      0(    0 scalars)
  Number of Real discrete variables:               0(    0 scalars)
  Number of Integer discrete variables:            0(    0 scalars)
  Number of Enum discrete variables:               0(    0 scalars)
  Number of Boolean discrete variables:            0(    0 scalars)
  Number of String discrete variables:             0(    0 scalars)
Number of equations:                               0(    0 scalars)
Number of variables with binding expression:       0(    0 scalars)
  Number of Real variables with binding exp:       0(    0 scalars)
  Number of Integer variables binding exp:         0(    0 scalars)
  Number of Enum variables binding exp:            0(    0 scalars)
  Number of Boolean variables binding exp:         0(    0 scalars)
  Number of String variables binding exp:          0(    0 scalars)
Total number of equations:                         0(    0 scalars)
Number of initial equations:                       0(    0 scalars)
Number of event indicators in equations:           0
Number of event indicators in init equations:      0

Independent constants: 
 aic: number of uses: 0, isLinear: true
 bic: number of uses: 0, isLinear: true
 adc: number of uses: 0, isLinear: true
 bdc: number of uses: 0, isLinear: true
 av: number of uses: 0, isLinear: true
 bv: number of uses: 0, isLinear: true

Dependent constants: 

Independent parameters: 
 aip: number of uses: 1, isLinear: true, evaluated binding exp: EnumerationTests.FlatAPIEnum1.A.b
 bip: number of uses: 1, isLinear: true, evaluated binding exp: EnumerationTests.FlatAPIEnum1.B.f

Dependent parameters: 
 adp: number of uses: 1, isLinear: true
 bdp: number of uses: 1, isLinear: true

Differentiated variables: 

Derivative variables: 

Discrete variables: 

Algebraic real variables: 

Input variables: 

Alias sets:
0 variables can be eliminated

Incidence:



Connection sets: 0 sets
")})));
  end FlatAPIEnum1;
  
  
  model ShortEnumDecl
	  type A = enumeration( one, two );
	  type B = A;
	  parameter B b = B.one;
	  Real x = 1;

	annotation(__JModelica(UnitTesting(tests={
		TransformCanonicalTestCase(
			name="ShortEnumDecl",
			description="Short class decl of enumeration",
			flatModel="
fclass EnumerationTests.ShortEnumDecl
 parameter EnumerationTests.ShortEnumDecl.A b = EnumerationTests.ShortEnumDecl.A.one /* EnumerationTests.ShortEnumDecl.A.one */;
 constant Real x = 1;

public
 type EnumerationTests.ShortEnumDecl.A = enumeration(one, two);

end EnumerationTests.ShortEnumDecl;
")})));
  end ShortEnumDecl;



end EnumerationTests;
