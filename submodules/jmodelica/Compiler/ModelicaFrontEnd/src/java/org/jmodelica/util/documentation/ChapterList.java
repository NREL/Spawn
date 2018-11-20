package org.jmodelica.util.documentation;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.PriorityQueue;
import java.util.SortedSet;

/**
 * A listing of chapters ordered according to their order as specified by a
 * listing of chapter names in a file.
 * <p>
 * TODO: Would be nice to generalise, but chapters are compared by name. This
 * class could work only on names, but then a map of chapter names -> chapter
 * objects would be required instead.
 * <p>
 * TODO: {@link SortedSet} > {@link List} (having identical chapter names in the
 * ordering is nonsensical).
 * 
 * @param <T>
 *            The type of objects ordered by this list.
 */
public class ChapterList<T extends ChapterElement> extends PriorityQueue<T> {
    /**
     * Serial ID.
     */
    private static final long serialVersionUID = -5742232028918625392L;

    private List<String> order;

    /**
     * Constructs a {@code ChapterList} from the ordering in a file.
     * 
     * @param orderFile
     *            The file from which to read the order.
     * @throws IOException
     *             if there was any error reading the file.
     */
    public ChapterList(File orderFile) throws IOException {
        this(Arrays.asList(new String(Files.readAllBytes(Paths.get(orderFile.getAbsolutePath()))).split("\n")));
    }

    /**
     * Constructs a {@code ChapterList} using an ordering specified by a list.
     * 
     * @param order
     *            The list to order by.
     */
    public ChapterList(List<String> order) {
        super(10, new ChapterComparator(order));
        this.order = order;
    }

    /**
     * {@link ChapterElement} comparator (compares chapter names).
     */
    private static final class ChapterComparator implements Comparator<ChapterElement> {
        private List<String> order;

        /**
         * Constructs a chapter comparator with ordering specified by a list of
         * chapter names.
         * 
         * @param names
         *            The list of chapter names.
         */
        public ChapterComparator(List<String> names) {
            this.order = names;
        }

        @Override
        public int compare(ChapterElement chapter, ChapterElement other) {
            int itemIndex = order.indexOf(chapter.name());
            int otherIndex = order.indexOf(other.name());
            boolean noOther = otherIndex < 0;

            if (itemIndex < 0) {
                if (noOther) {
                    return chapter.compareTo(other);
                }
                return 1;
            }

            if (noOther) {
                return -1;
            }

            return itemIndex - otherIndex;
        }
    }

    /**
     * Adds another name to the ordering.
     * <p>
     * TODO: Make public and remove suppression annotation if implemented.
     * 
     * @param name
     *            The name of the chapter.
     * @param before
     *            Chapter names required to precede {@code name}.
     * @param after
     *            Chapter names required to succeed {@code name}.
     */
    @SuppressWarnings("unused")
    private void addRule(String name, List<String> before, List<String> after) {
        throw new UnsupportedOperationException("NYI.");
    }

    /**
     * Checks whether or not this list is complete, i.e. if all chapters
     * specified by the ordering are accounted for.
     * 
     * @return
     *         {@code true} if, for each name specified in the ordering, there
     *         exists an object in this list with the same name. Otherwise,
     *         {@code false} is returned.
     */
    public boolean complete() {
        for (String name : order) {
            if (!contains(name)) {
                return false;
            }
        }
        return true;
    }

}
