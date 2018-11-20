package org.jmodelica.common.options;

import java.util.Locale;

import org.jmodelica.common.options.OptionRegistry.Category;
import org.jmodelica.common.options.OptionRegistry.Default;
import org.jmodelica.common.options.OptionRegistry.OptionType;

class RealOption extends Option<Double> {
    protected double min;
    protected double max;

    /**
     * Creates an option for real values.
     * 
     * @param key
     *          The key to (name of) the option.
     * @param type
     *          The type of option.
     * @param category
     *          The category of the option.
     * @param description
     *          A description of the option.
     * @param defaultValue
     *          The option's default value.
     * @param testDefault
     *          The option's default value when under test.
     */
    public RealOption(String key, OptionType type, Category category, String description,
            Default<Double> defaultValue, Default<Double> testDefault) {

        this(key, type, category, description, defaultValue, testDefault, Double.MIN_VALUE, Double.MAX_VALUE);
    }

    /**
     * Creates an option for real values.
     * 
     * @param key
     *          The key to (name of) the option.
     * @param type
     *          The type of option.
     * @param category
     *          The category of the option.
     * @param description
     *          A description of the option.
     * @param defaultValue
     *          The option's default value.
     * @param testDefault
     *          The option's default value when under test.
     * @param min
     *          The minimum allowed value for this option.
     * @param max
     *          The maximum allowed value for this option.
     */
    public RealOption(String key, OptionType type, Category category, String description,
            Default<Double> defaultValue, Default<Double> testDefault, double min, double max) {

        super(key, type, category, description, defaultValue, testDefault, 0.0);
        this.min = min;
        this.max = max;
    }

    @Override
    public void setValue(Double d) {
        this.isSet = true;
        this.value = d.doubleValue();
    }

    @Override
    protected void setValue(String str) {
        try {
            setValue(Double.parseDouble(str));
        } catch (NumberFormatException e) {
            invalidValue(str, ", expecting integer value" + minMaxStr());
        }
    }

    /**
     * Sets the value for this option.
     * <p>
     * This method is required to avoid auto-boxing {@code double}s to {@link Double} and lose precision.
     */
    public void setValue(double value) {
        if (value < min || value > max) {
            invalidValue(value, minMaxStr());
        }
        this.isSet = true;
        this.value = value;
    }

    private String minMaxStr() {
        return (min == Double.MIN_VALUE ? "" : ", min: " + min) + (max == Double.MAX_VALUE ? "" : ", max: " + max);
    }

    /**
     * Retrieves the minimum allowed value for this option.
     * 
     * @return
     *          the minimum allowed value for this option.
     */
    public double getMin() {
        return min;
    }

    /**
     * Retrieves the maximum allowed value for this option.
     * 
     * @return
     *          the maximum allowed value for this option.
     */
    public double getMax() {
        return max;
    }

    @Override
    public Boolean isLimited() {
        return (min != Double.MIN_VALUE) || (max != Double.MAX_VALUE);
    }

    /**
     * Lowers the minimum allowed value for this option. Will not raise it.
     * 
     * @param val
     *          The new minimum allowed value.
     */
    public void expandMin(double val) {
        if (val < min)
            min = val;
    }

    /**
     * Raises the maximum allowed value for this option. Will not lower it.
     * 
     * @param val
     *          The new maximum allowed value.
     */
    public void expandMax(double val) {
        if (val > max)
            max = val;
    }

    @Override
    public String getType() {
        return "real";
    }

    @Override
    public String getValueString() {
        return Double.toString(getValue());
    }

    @Override
    public String getValueForDoc() {
        String raw = getValueString();
        String round = String.format((Locale) null, "%.2E", getValue());
        return (round.length() < raw.length()) ? round : raw;
    }


    @Override
    protected void copyTo(OptionRegistry reg, String key) {
        if (!reg.hasOption(key)) {
            reg.addRealOption(key, getOptionType(), getCategory(), defaultValue, testDefault,
                    getDescription(), min, max);
        }
        if (isSet) {
            reg.setRealOption(key, value);
        }
    }

}