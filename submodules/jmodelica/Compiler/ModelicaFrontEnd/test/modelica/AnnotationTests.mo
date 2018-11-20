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

within;

package AnnotationTests


model ShortClassConstrainedBy1
    model A end A;
    model B extends A; end B;
    replaceable model C = B annotation(a = b) constrainedby A "A";

    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ShortClassConstrainedBy1",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printClassAnnotations",
            arguments={""},
            methodResult="
(a = b)
")})));
end ShortClassConstrainedBy1;

model ShortClassConstrainedBy2
    model A end A;
    model B extends A; end B;
    replaceable model C = B constrainedby A "A" annotation(a = b);

    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ShortClassConstrainedBy2",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printClassAnnotations",
            arguments={""},
            methodResult="
(a = b)
")})));
end ShortClassConstrainedBy2;

model ShortClassConstrainedBy3
    model A end A;
    model B extends A; end B;
    replaceable model C = B annotation(a = b) constrainedby A "A" annotation(b = c);

    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ShortClassConstrainedBy3",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printClassAnnotations",
            arguments={""},
            methodResult="
(a = b, b = c)
")})));
end ShortClassConstrainedBy3;

model ComponentConstrainedByOnlyNormal
    model A
    end A;
    replaceable A a annotation(a = b) constrainedby A;
    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ComponentConstrainedByOnlyNormal",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printComponentAnnotations",
            methodResult="
(a = b)
")})));
end ComponentConstrainedByOnlyNormal;
model ComponentConstrainedByOnlyConstrained
    model A
    end A;
    replaceable A a constrainedby A annotation(a = b);
    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ComponentConstrainedByOnlyConstrained",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printComponentAnnotations",
            methodResult="
(a = b)
")})));
end ComponentConstrainedByOnlyConstrained;
model ComponentConstrainedByBoth
    model A
    end A;
    replaceable A a annotation(a = b) constrainedby A annotation(b = c);
    annotation(__JModelica(UnitTesting(tests={
        InstClassMethodTestCase(
            name="ComponentConstrainedByBoth",
            description="Tests annotations on replaceable constrainedby declarations.",
            methodName="printComponentAnnotations",
            methodResult="
(a = b, b = c)
")})));
end ComponentConstrainedByBoth;


end AnnotationTests;