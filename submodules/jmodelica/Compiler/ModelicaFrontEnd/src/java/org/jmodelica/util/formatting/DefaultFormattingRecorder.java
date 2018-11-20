package org.jmodelica.util.formatting;

/**
 * Default hander that does nothing, just ignores the formatting.
 */
public final class DefaultFormattingRecorder<T> extends FormattingRecorder<T> {
    @Override
    public void addItem(FormattingType type, String data, int startLine, int startColumn, int endLine, int endColumn) {
    }

    @Override
    public void reset() {
    }

    @Override
    public void postParsing(T t) {
        // Do nothing
    }
}
