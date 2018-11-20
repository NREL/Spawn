package org.jmodelica.util.annotations;

import org.jmodelica.util.annotations.AnnotationProvider.SubNodePair;
import org.jmodelica.util.values.Evaluable;

public interface IterableAnnotationProvider<N extends AnnotationProvider<N, V>, V extends Evaluable>
        extends AnnotationProvider<N, V>, Iterable<SubNodePair<N>> {}
