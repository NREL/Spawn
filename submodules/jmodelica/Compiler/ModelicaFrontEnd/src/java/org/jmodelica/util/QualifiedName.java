/*
    Copyright (C) 2018 Modelon AB

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
package org.jmodelica.util;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.jmodelica.util.exceptions.NameFormatException;

/**
 * Handle splitting strings into different parts of a qualified name.
 */
public class QualifiedName {
    private boolean isGlobal;
    private int i = 0;
    ArrayList<String> names;
    private boolean isUnQualifiedImport;

    public QualifiedName(String name) {
        if (name.length() == 0) {
            throw new NameFormatException("A name must have atleast one caracter"); 
        }
        isUnQualifiedImport = name.endsWith(".*");
        isGlobal = name.startsWith(".");
        names = splitQualifiedClassName(name);
    }

    // Interpret name as global or not regardless or dot form or not.
    public QualifiedName(String name, boolean isGlobal) {
        if (name.length() == 0) {
            throw new NameFormatException("A name must have atleast one caracter");
        }
        names = splitQualifiedClassName(name); 
        this.isGlobal = isGlobal;
    }

    public boolean hasNext() {
        return i < names.size();
    }

    public String next() {
        return names.get(i++);
    }

    public ArrayList<String> getNames() {
        return names;
    }

    @Override
    public String toString() {
        return names.toString();
    }

    public boolean isGlobal() {
        return isGlobal;
    }

    static final Pattern p = Pattern.compile("(?<![\\\\])['.]");

    private static void checkNameLength(int start,int end) {
        if (end - start < 2)
            throw new NameFormatException("Names must have a length greater than zero");
    }

    private static void findNameSeparations(Matcher m, ArrayList<Integer> list) {
        boolean inquoted = false;
        int prev = -1;
        while (m.find()) {
            String token = m.group();
            if (token.equals("'")) {
                if (inquoted) {
                    checkNameLength(prev, m.start());
                } else {
                    if (m.start() - prev > 2) {
                        throw new NameFormatException("Quotes not allowed inside unqouted name");
                    }
                }
                prev = inquoted ? prev : m.start();
                inquoted = !inquoted;
            } else if (!inquoted) {
                checkNameLength(prev, m.start());
                prev = m.start();
                list.add(m.start() + 1);
            }
        }
        if (inquoted) {
            throw new NameFormatException("Qualified Name couldn't be interpreted due to unmatched quotes");
        }
    }

    /**
     * Splits a composite class name into all the partial access
     * 
     * @param name
     * @return array with the names of all accessed classes.
     */
    private final ArrayList<String> splitQualifiedClassName(String name) {
        if (isGlobal || isUnQualifiedImport) {
            int start = isGlobal ? 1 : 0;
            int end = isUnQualifiedImport ? name.length() -2 : name.length(); 
            name = name.substring(start, end);
        }
        Matcher m = p.matcher(name);
        ArrayList<Integer> nameSeparations = new ArrayList<Integer>();
        findNameSeparations(m, nameSeparations);
        nameSeparations.add(name.length() + 1);
        ArrayList<String> parts = new ArrayList<String>();
        int partEnd = 0;
        for (int namePart = 0; namePart < nameSeparations.size() ; namePart++) {
            parts.add(name.substring(partEnd, nameSeparations.get(namePart) - 1));
            partEnd = nameSeparations.get(namePart);
        }
        return parts;
    }
}
