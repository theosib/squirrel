#include "value.hpp"
#include <sstream>

namespace squirrel {

ValuePtr Value::EMPTY_STR = StringValue::make(Symbol::empty_symbol);
ValuePtr Value::ZERO_INT = IntValue::make(0);
ValuePtr Value::ONE_INT = IntValue::make(1);
ValuePtr Value::ZERO_FLOAT = FloatValue::make(0);
ValuePtr Value::ONE_FLOAT = FloatValue::make(1);
ValuePtr Value::TRUE = BoolValue::make(true);
ValuePtr Value::FALSE = BoolValue::make(false);

ValuePtr BoolValue::TRUE_STR = StringValue::make(Symbol::find("true"));
ValuePtr BoolValue::FALSE_STR = StringValue::make(Symbol::find("true"));

// XXX produce string based on type
ValuePtr Value::to_string() const { return EMPTY_STR; }
ValuePtr Value::to_int() const { return ZERO_INT; }
ValuePtr Value::to_float() const { return ZERO_FLOAT; }
ValuePtr Value::to_number() const { return ZERO_INT; }
ValuePtr Value::to_bool() const { return FALSE; }

ContextPtr Value::get_context() { return 0; }

ValuePtr BoolValue::to_string() const {
    return bval ? TRUE_STR : FALSE_STR;
}

ValuePtr BoolValue::to_int() const {
    return bval ? ONE_INT : ZERO_INT;
}

ValuePtr BoolValue::to_float() const {
    return bval ? ONE_FLOAT : ZERO_FLOAT;
}

ValuePtr BoolValue::to_number() const { return to_int(); }
ValuePtr BoolValue::to_bool() const { return bval ? TRUE : FALSE; }


ValuePtr IntValue::to_string() const {
    std::stringstream ss;
    ss << ival;
    return StringValue::make(ss.str());
}

ValuePtr IntValue::to_int() const { return IntValue::make(ival); }
ValuePtr IntValue::to_float() const { return FloatValue::make(ival); }
ValuePtr IntValue::to_number() const { return to_int(); }
ValuePtr IntValue::to_bool() const { return ival ? TRUE : FALSE; }


ValuePtr FloatValue::to_string() const {
    std::stringstream ss;
    ss << fval;
    return StringValue::make(ss.str());
}

ValuePtr FloatValue::to_float() const { return FloatValue::make(fval); }
ValuePtr FloatValue::to_int() const { return IntValue::make((int)fval); }
ValuePtr FloatValue::to_number() const { return to_float(); }
ValuePtr FloatValue::to_bool() const { return fval ? TRUE : FALSE; }


// XXX use parser
ValuePtr StringValue::to_string() const { return StringValue::make(sym); }
ValuePtr StringValue::to_int() const { return 0; }
ValuePtr StringValue::to_float() const { return 0; }
ValuePtr StringValue::to_number() const { return 0; }
ValuePtr StringValue::to_bool() const { return 0; }

ValuePtr SymbolValue::to_string() const { 
    std::stringstream ss;
    ss << sym;
    return StringValue::make(ss.str());
}

ContextPtr ContextValue::get_context() { return context; }

// XXX
ValuePtr ExceptionValue::to_string() const {
    return 0;
}

std::string Value::as_string() const {
    StringValuePtr s = std::static_pointer_cast<StringValue>(to_string());
    return s->sym->as_string();
}


} // namespace squirrel
