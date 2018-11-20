package org.jmodelica.common.options;

/**
 * An {@link OptionRegistry} wrapper class to be used for testing.
 */
public class TestOptions extends OptionRegistry {

    /**
     * Creates an instance of {@code TestOptions}.
     */
    public TestOptions() {
        super();
    }

    /**
     * Retrieves an option by its key name.
     * 
     * @param <T>
     *          The type of option.
     * @param key
     *          The option's key (name).
     * @param allowMissing
     *          A flag specifying whether or not to allow missing values. If this flag is set to {@code false},
     *          an exception will be raised in case the option was missing.
     * @return
     *          the option with the key {@code key}.
     * @throws UnknownOptionException
     *          if the option could not be found, and the {@code allowMissing} flag is false.
     */
    @SuppressWarnings("unchecked")
    public <T> Option<T> findTestOption(String key, boolean allowMissing) {
        Option<?> option = optionsMap.get(key);
        Option<T> returnOption = null;
        try {
            returnOption = (Option<T>) option;
        } catch (ClassCastException e) {
            throw new UnknownOptionException("Option: " + key + " is of " +
                    option.getClass().getSimpleName() + " type.");
        }
        if (returnOption != null || allowMissing) {
            return returnOption;
        }
        throw new UnknownOptionException(unknownOptionMessage(key));
    }

    /**
     * Retrieves an option setting but defaults to the test-specific default
     * value rather than the normal default value.
     * <p>
     * TODO: Use this for all options in {@link OptionRegistry}?
     * 
     * @param <T>
     *            The type of option to retrieve.
     * @param key
     *            The key (name) of the option.
     * @return
     *         The option value, or the test-specific default value in case the
     *         value is not set.
     */
    @SuppressWarnings("unchecked")
    public <T> T getTestOption(String key) {
        return (T) findTestOption(key, false).getTestValue();
    }

    /**
     * Getters used by {@link TestCase} and {@link TestAnnotationizerHelper} in order to retrieve options from a
     * testing context.
     * <p>
     * These methods retrieves options of the correct type via generics' magic.
     */

    @Override
    public boolean getBooleanOption(String key) {
        return this.<Boolean> getTestOption(key);
    }

    @Override
    public int getIntegerOption(String key) {
        return this.<Integer> getTestOption(key);
    }

    @Override
    public double getRealOption(String key) {
        return this.<Double> getTestOption(key);
    }

    @Override
    public String getStringOption(String key) {
        return this.<String> getTestOption(key);
    }

    @Override
    public OptionRegistry copy() {
        OptionRegistry res = new TestOptions() {};
        res.copyAllOptions(this);
        return res;
    }

}
