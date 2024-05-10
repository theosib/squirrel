#include "value.hpp"
#include "context.hpp"
#include <sstream>
#include "parser.hpp"

namespace squirrel {

ValuePtr Value::EMPTY_STR = StringValue::make(Symbol::empty_symbol);
ValuePtr Value::ZERO_INT = IntValue::make(0);
ValuePtr Value::ONE_INT = IntValue::make(1);
ValuePtr Value::NEGONE_INT = IntValue::make(-1);
ValuePtr Value::ZERO_FLOAT = FloatValue::make(0);
ValuePtr Value::ONE_FLOAT = FloatValue::make(1);
ValuePtr Value::TRUE = BoolValue::make(true);
ValuePtr Value::FALSE = BoolValue::make(false);

ValuePtr BoolValue::TRUE_STR = StringValue::make(Symbol::find("true"));
ValuePtr BoolValue::FALSE_STR = StringValue::make(Symbol::find("false"));

static const char *type_names[] = {
    "NONE",
    "LIST",
    "INT",
    "STR",
    "FLOAT",
    "OPER",
    "SYM",
    "FUNC",
    "INFIX",
    "BOOL",
    "CLASS",
    "OBJECT",
    "EXCEPTION",
    "CONTEXT"
};

// XXX produce string based on type
ValuePtr Value::to_string() const {
    // std::cout << "General to_string\n";
    std::string n(type_names[type]);
    SymbolPtr name = get_name();
    if (name) {
        n += ':';
        n += name->as_string();
    }
    return StringValue::make(n);
}

ValuePtr Value::to_int() const { return ZERO_INT; }
ValuePtr Value::to_float() const { return ZERO_FLOAT; }
ValuePtr Value::to_number() const { return ZERO_INT; }
ValuePtr Value::to_bool() const { return FALSE; }

ContextPtr Value::get_context() const { return 0; }
SymbolPtr Value::get_name() const {
    ContextPtr c = get_context();
    if (!c) return Symbol::empty_symbol;
    return c->get_name();
}

NoneValuePtr NoneValue::none_value = std::make_shared<NoneValue>();

ValuePtr NoneValue::to_string() const {
    return Value::EMPTY_STR;
}

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
    // std::cout << "int to_string\n";
    std::stringstream ss;
    ss << ival;
    return StringValue::make(ss.str());
}

ValuePtr IntValue::to_int() const { return std::const_pointer_cast<Value>(shared_from_this()); }
ValuePtr IntValue::to_float() const { return FloatValue::make(ival); }
ValuePtr IntValue::to_number() const { return to_int(); }
ValuePtr IntValue::to_bool() const { return ival ? TRUE : FALSE; }


ValuePtr FloatValue::to_string() const {
    std::stringstream ss;
    ss << fval;
    return StringValue::make(ss.str());
}

ValuePtr FloatValue::to_float() const { return std::const_pointer_cast<Value>(shared_from_this()); }
ValuePtr FloatValue::to_int() const { return IntValue::make((int)fval); }
ValuePtr FloatValue::to_number() const { return to_float(); }
ValuePtr FloatValue::to_bool() const { return fval ? TRUE : FALSE; }

ValuePtr StringValue::to_string() const { return std::const_pointer_cast<Value>(shared_from_this()); }
ValuePtr StringValue::to_int() const { return to_number()->to_int(); }
ValuePtr StringValue::to_float() const { return to_number()->to_float(); }
ValuePtr StringValue::to_number() const {
    Parsing p(sym->str.data(), sym->str.size());
    ValuePtr t = Parser::parse_number(p);
    if (!t) return ZERO_INT;
    return t; 
}
ValuePtr StringValue::to_bool() const {
    if (sym->str.size() == 0) return FALSE;
    if (sym->str.size() == 1 && sym->str[0] == '0') return FALSE;
    return TRUE; 
}

ValuePtr SymbolValue::to_string() const { 
    std::stringstream ss;
    ss << sym;
    return StringValue::make(ss.str());
}

ContextPtr ContextValue::get_context() const { return context; }

std::ostream& ListValue::print(std::ostream& os) const {
    bool first = true;
    for (int i=0; i<size(); i++) {
        if (!first) os << ' ';
        ValuePtr v = get(i);
        if (v) {
            os << v->as_print_string();
        } else {
            os << "[null]";
        }
        first = false;
    }
    return os;
}

ValuePtr InfixValue::to_string() const {
    std::stringstream ss;
    ss << '(';
    print(ss);
    ss << ')';
    return StringValue::make(ss.str());
}

ValuePtr ListValue::to_string() const {
    std::stringstream ss;
    ss << '{';
    print(ss);
    ss << '}';
    return StringValue::make(ss.str());
}

// XXX
ValuePtr ExceptionValue::to_string() const {
    std::stringstream ss;
    std::shared_ptr<const ExceptionValue> p = std::static_pointer_cast<const ExceptionValue>(shared_from_this());
    bool first = true;
    while (p) {
        //std::cout << err << " from " << p->get_context()->name << std::endl;
        if (!first) ss << std::endl;
        ContextPtr c = p->get_context();
        if (c) {
            if (p->err.size() > 0) {
                ss << "Exception from " << c->name << ": " << p->err;
            } else {
                ss << "Exception from " << c->name;
            }
        } else {
            ss << p->err;
        }
        p = p->parent;
        first = false;
    }
    return StringValue::make(ss.str());
}

int Value::as_int() const {
    IntValuePtr s = std::static_pointer_cast<IntValue>(to_int());
    return s->ival;
}

float Value::as_float() const {
    FloatValuePtr s = std::static_pointer_cast<FloatValue>(to_float());
    return s->fval;
}

bool Value::as_bool() const {
    BoolValuePtr s = std::static_pointer_cast<BoolValue>(to_bool());
    return s->bval;
}

std::string Value::as_string() const {
    // std::cout << "As string: " << type_names[type] << std::endl;
    StringValuePtr s = std::static_pointer_cast<StringValue>(to_string());
    // std::cout << "Now calling as_string()\n";
    return s->sym->as_string();
}

std::string Value::as_print_string() const { 
    if (quote) {
        return std::string("\'") + as_string();
    } else {
        return as_string(); 
    }
}


std::ostream& print_octal(std::ostream& os, int val, int len)
{
    for (int i=len-1; i>=0; i--) {
        char c = ((val >> (i*3)) & 7) + '0';
        os << c;
    }
    return os;
}

std::ostream& print_string(std::ostream& os, const std::string_view& s)
{
    for (int i=0; i<s.size(); i++) {
        char c = s[i];
        if (c < 32) {
            os << '\\';
            print_octal(os, c, 3);
        } else {
            os << c;
        }
    }
    return os;
}

std::string StringValue::as_print_string() const {
    std::stringstream ss;
    if (quote) ss << '\'';
    ss << '\"';
    print_string(ss, sym->as_string());
    ss << '\"';
    return ss.str();
}

SymbolPtr FunctionValue::get_name() const {
    return name;
}

SymbolPtr ClassValue::get_name() const {
    return name;
}

SymbolPtr OperatorValue::get_name() const {
    return name;
}


} // namespace squirrel
