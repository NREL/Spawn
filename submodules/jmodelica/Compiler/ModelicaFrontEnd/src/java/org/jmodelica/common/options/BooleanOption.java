package org.jmodelica.common.options;

import org.jmodelica.common.options.OptionRegistry.Category;
import org.jmodelica.common.options.OptionRegistry.Default;
import org.jmodelica.common.options.OptionRegistry.OptionType;

class BooleanOption extends Option<Boolean> {

    public BooleanOption(String key, OptionType type, Category cat, String description,
            Default<Boolean> def, Default<Boolean> testDefault) {

        super(key, type, cat, description, def, testDefault, false);
    }

    @Override
    protected void setValue(String str) {
        if (str.equals("true") || str.equals("yes") || str.equals("on"))
            setValue(true);
        else if (str.equals("false") || str.equals("no") || str.equals("off"))
            setValue(false);
        else
            invalidValue(str, ", expecting boolean value.");
    }

    @Override
    public String getType() {
        return "boolean";
    }

    @Override
    public String getValueString() {
        return Boolean.toString(getValue());
    }

    @Override
    protected void copyTo(OptionRegistry reg, String key) {
        if (!reg.hasOption(key)) {
            reg.addBooleanOption(key, getOptionType(), getCategory(), defaultValue, testDefault, getDescription());
        }
        if (isSet) {
            reg.setBooleanOption(key, value);
        }
    }

}