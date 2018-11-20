package org.jmodelica.util.problemHandling;

import org.jmodelica.common.options.OptionRegistry;

public interface ProblemOptionsProvider {
    public OptionRegistry getOptionRegistry();
    public boolean filterThisWarning(String identifier);
}
