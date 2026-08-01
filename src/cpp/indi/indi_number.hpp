/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_NUMBER_HPP
#define NYX_INDI_NUMBER_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "utils.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup NUMBER_MESSAGE_CPP Nyx Number Message
 * @brief Nyx / INDI Number Message API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/* NYX PROPERTIES                                                                                                     */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief Base class for INDI / Nyx number properties.
 */
class NumberProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the @c name property of this object.
     *
     * @return The current value.
     */
    STR_t name() const { return get_string("@name"); }

    /**
     * @brief Gets the @c label property of this object.
     *
     * @return The current value.
     */
    STR_t label() const { return get_string("@label"); }

    /**
     * @brief Gets the @c format property of this object.
     *
     * @return The current value.
     */
    STR_t format() const { return get_string("@format"); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the @c name property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_name(STR_t value) const { return set_string_attr("@name", value); }

    /**
     * @brief Sets the @c label property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_label(STR_t value) const { return set_string_attr("@label", value); }

    /**
     * @brief Sets the @c format property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_format(STR_t value) const { return set_string_attr("@format", value); }

    /*----------------------------------------------------------------------------------------------------------------*/

protected:
    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberProp(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx @c int32_t number property.
 */
class NumberIntProp : public NumberProp
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c int32_t number property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Printf-style formatting string (%[flags][width]d).
     * @param min Range min, ignored if min == max.
     * @param max Range max, ignored if min == max.
     * @param step Step size, ignored if step == 0.
     * @param value Initial @c int32_t value.
     */
    NumberIntProp(STR_t name, STR_t label, STR_t format, int32_t min, int32_t max, int32_t step, int32_t value) : NumberProp(nyx_number_prop_new_int(name, label, format, min, max, step, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberIntProp(nyx_dict_t *ptr, bool borrowed = false) : NumberProp(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this property object.
     *
     * @return The value.
     */
    int32_t value() const { return nyx_number_prop_get_int(dict_ptr()); }

    /**
     * @brief Sets the value of this property object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(int32_t value) const { return nyx_number_prop_set_int(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const NumberIntProp &, int32_t, int32_t)>
    void on() const
    {
        dict_ptr()->base.callback._int = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const NumberIntProp &, int32_t, int32_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, int new_value, int old_value)
    {
        Vector vector_obj(vector, true);
        NumberIntProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<int32_t>(new_value), static_cast<int32_t>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx @c uint32_t number property.
 */
class NumberUIntProp : public NumberProp
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c uint32_t number property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Printf-style formatting string (%[flags][width]{uoxX}).
     * @param min Range min, ignored if min == max.
     * @param max Range max, ignored if min == max.
     * @param step Step size, ignored if step == 0.
     * @param value Initial @c uint32_t value.
     */
    NumberUIntProp(STR_t name, STR_t label, STR_t format, uint32_t min, uint32_t max, uint32_t step, uint32_t value) : NumberProp(nyx_number_prop_new_uint(name, label, format, min, max, step, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberUIntProp(nyx_dict_t *ptr, bool borrowed = false) : NumberProp(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this property object.
     *
     * @return The value.
     */
    uint32_t value() const { return nyx_number_prop_get_uint(dict_ptr()); }

    /**
     * @brief Sets the value of this property object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(uint32_t value) const { return nyx_number_prop_set_uint(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const NumberUIntProp &, uint32_t, uint32_t)>
    void on() const
    {
        dict_ptr()->base.callback._uint = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const NumberUIntProp &, uint32_t, uint32_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, unsigned int new_value, unsigned int old_value)
    {
        Vector vector_obj(vector, true);
        NumberUIntProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<uint32_t>(new_value), static_cast<uint32_t>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx @c int64_t number property.
 */
class NumberLongProp : public NumberProp
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c int64_t number property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Printf-style formatting string (%[flags][width]ld).
     * @param min Range min, ignored if min == max.
     * @param max Range max, ignored if min == max.
     * @param step Step size, ignored if step == 0.
     * @param value Initial @c int64_t value.
     */
    NumberLongProp(STR_t name, STR_t label, STR_t format, int64_t min, int64_t max, int64_t step, int64_t value) : NumberProp(nyx_number_prop_new_long(name, label, format, min, max, step, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberLongProp(nyx_dict_t *ptr, bool borrowed = false) : NumberProp(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this property object.
     *
     * @return The value.
     */
    int64_t value() const { return nyx_number_prop_get_long(dict_ptr()); }

    /**
     * @brief Sets the value of this property object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(int64_t value) const { return nyx_number_prop_set_long(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const NumberLongProp &, int64_t, int64_t)>
    void on() const
    {
        dict_ptr()->base.callback._long = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const NumberLongProp &, int64_t, int64_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, long new_value, long old_value)
    {
        Vector vector_obj(vector, true);
        NumberLongProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<int64_t>(new_value), static_cast<int64_t>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx @c uint64_t number property.
 */
class NumberULongProp : public NumberProp
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c uint64_t number property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Printf-style formatting string (%[flags][width]l{uoxX}).
     * @param min Range min, ignored if min == max.
     * @param max Range max, ignored if min == max.
     * @param step Step size, ignored if step == 0.
     * @param value Initial @c uint64_t value.
     */
    NumberULongProp(STR_t name, STR_t label, STR_t format, uint64_t min, uint64_t max, uint64_t step, uint64_t value) : NumberProp(nyx_number_prop_new_ulong(name, label, format, min, max, step, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberULongProp(nyx_dict_t *ptr, bool borrowed = false) : NumberProp(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this property object.
     *
     * @return The value.
     */
    uint64_t value() const { return nyx_number_prop_get_ulong(dict_ptr()); }

    /**
     * @brief Sets the value of this property object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(uint64_t value) const { return nyx_number_prop_set_ulong(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const NumberULongProp &, uint64_t, uint64_t)>
    void on() const
    {
        dict_ptr()->base.callback._ulong = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const NumberULongProp &, uint64_t, uint64_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, unsigned long new_value, unsigned long old_value)
    {
        Vector vector_obj(vector, true);
        NumberULongProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<uint64_t>(new_value), static_cast<uint64_t>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx @c double number property.
 */
class NumberDoubleProp : public NumberProp
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c double number property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Printf-style formatting string (%[flags][width]l?{fFeEgGaAm}).
     * @param min Range min, ignored if min == max.
     * @param max Range max, ignored if min == max.
     * @param step Step size, ignored if step == 0.
     * @param value Initial @c double value.
     */
    NumberDoubleProp(STR_t name, STR_t label, STR_t format, double min, double max, double step, double value) : NumberProp(nyx_number_prop_new_double(name, label, format, min, max, step, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NumberDoubleProp(nyx_dict_t *ptr, bool borrowed = false) : NumberProp(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this property object.
     *
     * @return The value.
     */
    double value() const { return nyx_number_prop_get_double(dict_ptr()); }

    /**
     * @brief Sets the value of this property object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(double value) const { return nyx_number_prop_set_double(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const NumberDoubleProp &, double, double)>
    void on() const
    {
        dict_ptr()->base.callback._double = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const NumberDoubleProp &, double, double)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, double new_value, double old_value)
    {
        Vector vector_obj(vector, true);
        NumberDoubleProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, new_value, old_value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/* NYX VECTORS                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_MESSAGE_CPP
 * @brief INDI / Nyx number vector.
 */
class NumberVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx number vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param perm Vector permissions.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    NumberVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        const std::vector<NumberProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_number_vector_new(device, name, state, perm, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx number property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    NumberVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        std::initializer_list<NumberProp> props,
        const Opts &opts = Opts()
    ) : NumberVector(device, name, state, perm, std::vector<NumberProp>(props), opts)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the @c perm property of this object.
     *
     * @return The current value.
     */
    Perm perm() const { return nyx_str_to_perm(get_string("@perm")); }

    /**
     * @brief Sets the @c perm property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_perm(Perm value) const { return set_string_attr("@perm", nyx_perm_to_str(value)); }

    /**
     * @brief Appends a property to this vector.
     *
     * @param prop Property to be added.
     * @return @c true if the vector was modified, @c false otherwise.
     */
    bool add(const NumberProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_NUMBER_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/
