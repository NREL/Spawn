package org.jmodelica.junit;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import org.jmodelica.util.exceptions.InternalCompilerError;

public class Util {

    /**
     * Get the path to a file using the class loader for provided object, e.g.
     * JUnit test.
     */
    public static String resource(Object test, String name) {
        return resource(test.getClass(), name);
    }

    /**
     * Get the path to a file using the class loader for provided class object.
     */
    public static String resource(Class<?> clazz, String name) {
        try {
            // This ensures that spaces are encoded correctly!
            return URLDecoder.decode(clazz.getResource(name).getPath(), "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new InternalCompilerError("Unable to decode loaded resource URL; " + e.getMessage(), e);
        }
    }

}
