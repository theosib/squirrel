#ifndef INCLUDED_SQUIRREL_VALUE_HPP
#define INCLUDED_SQUIRREL_VALUE_HPP

#include "types.hpp"
#include "enable_shared_from_base.hpp"
#include "symbol.hpp"
#include <string_view>
#include <algorithm>

namespace squirrel {

#define DEF_MAKE(x, t) \
    static x##Ptr make() { x##Ptr p = std::make_shared<x>(); p->type = t; return p; }

struct Value : public enable_shared_from_base<Value>  {
    enum {
        NONE,
        LIST,
        INT,
        STR,
        FLOAT,
        OPER,
        SYM,
        FUNC,
        INFIX,
        BOOL,
        CLASS,
        OBJECT,
        EXCEPTION,
        CONTEXT
    };
    
    uint8_t type;
    uint8_t quote;
    
    virtual ValuePtr to_string() const;
    virtual ValuePtr to_int() const;
    virtual ValuePtr to_float() const;
    virtual ValuePtr to_number() const;
    virtual ValuePtr to_bool() const;
    
    int as_int() const;
    float as_float() const;    
    std::string as_string() const;
    virtual std::string as_print_string() const;
    bool as_bool() const;
    
    bool has_context() {
        return type == CLASS || type == OBJECT || type == CONTEXT || type == EXCEPTION;
    }
    
    virtual SymbolPtr get_name() const;
    virtual ContextPtr get_context() const;
    
    static ValuePtr EMPTY_STR, ZERO_INT, ONE_INT, ZERO_FLOAT, ONE_FLOAT, NEGONE_INT, TRUE, FALSE;    
};

inline std::ostream& operator<<(std::ostream& os, const Value& s) {
    os << s.as_string();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ValuePtr& s) {
    os << (s ? s->as_string() : "[null value]");
    return os;
}

ValuePtr wrap_exception(ValuePtr v, ContextPtr c);

#define CAST_VALUE(v, c, type_code, type_name) ({ \
    const ValuePtr& __val(v); \
    if (!__val) return ExceptionValue::make("Illegal null reference", (c)); \
    if (__val->type == Value::EXCEPTION) return ::squirrel::wrap_exception(__val, c); \
    if (__val->type != type_code) return ExceptionValue::make(std::string("Expected " #type_code " type for: ") + __val->as_string(), (c)); \
    std::static_pointer_cast<type_name>(__val); })

#define CAST_CONTEXT(v, c) ({ \
    const ValuePtr& __val(v); \
    if (!__val) return ExceptionValue::make("Illegal null reference", (c)); \
    if (__val->type == Value::EXCEPTION) return ::squirrel::wrap_exception(__val, c); \
    if (!__val->has_context()) return ExceptionValue::make(std::string("Expected context type for: ") + __val->as_string(), (c)); \
    std::static_pointer_cast<ContextValue>(__val); })

#define GET_CONTEXT(v, c) ({ CAST_CONTEXT(v, c)->get_context(); })
        
#define CAST_LIST(v, c) ({ \
    const ValuePtr& __val(v); \
    if (!__val) return ExceptionValue::make("Illegal null reference", (c)); \
    if (__val->type == Value::EXCEPTION) return ::squirrel::wrap_exception(__val, c); \
    if (__val->type != Value::LIST && __val->type != Value::INFIX) return ExceptionValue::make(std::string("Expected list type for: ") + __val->as_string(), (c)); \
    std::static_pointer_cast<ListValue>(__val); })

#define CAST_EXCEPTION(v, c) ({ \
    const ValuePtr& __val(v); \
    if (!__val) return ExceptionValue::make("Illegal null reference", (c)); \
    if (__val->type != Value::EXCEPTION) return ExceptionValue::make(std::string("Expected EXCEPTION type for: ") + __val->as_string(), (c)); \
    std::static_pointer_cast<ExceptionValue>(__val); })

#define CAST_BOOL(v, c) CAST_VALUE(v, c, Value::BOOL, BoolValue)
#define CAST_INT(v, c) CAST_VALUE(v, c, Value::INT, IntValue)
#define CAST_FLOAT(v, c) CAST_VALUE(v, c, Value::FLOAT, FloatValue)
#define CAST_STRING(v, c) CAST_VALUE(v, c, Value::STR, StringValue)
#define CAST_SYMBOL(v, c) CAST_VALUE(v, c, Value::SYM, SymbolValue)
#define CAST_CLASS(v, c) CAST_VALUE(v, c, Value::CLASS, ClassValue)
#define CAST_OBJECT(v, c) CAST_VALUE(v, c, Value::OBJECT, ObjectValue)
#define CAST_FUNC(v, c) CAST_VALUE(v, c, Value::FUNC, FunctionValue)
#define CAST_OPER(v, c) CAST_VALUE(v, c, Value::OPER, OperatorValue)
#define CAST_INFIX(v, c) CAST_VALUE(v, c, Value::INFIX, InfixValue)

struct NoneValue : public Value {
    static NoneValuePtr none_value;
    virtual ValuePtr to_string() const;
    static NoneValuePtr make() {
        return none_value;
    }
};

struct BoolValue : public Value {
    uint8_t bval;

    virtual ValuePtr to_string() const;
    virtual ValuePtr to_int() const;
    virtual ValuePtr to_float() const;
    virtual ValuePtr to_number() const;
    virtual ValuePtr to_bool() const;
    
    DEF_MAKE(BoolValue, BOOL);
    static BoolValuePtr make(bool v) {
        BoolValuePtr p = make();
        p->bval = v;
        return p;
    }
    
    static ValuePtr TRUE_STR, FALSE_STR;
};

struct IntValue : public Value {
    int ival;

    virtual ValuePtr to_string() const;
    virtual ValuePtr to_int() const;
    virtual ValuePtr to_float() const;
    virtual ValuePtr to_number() const;
    virtual ValuePtr to_bool() const;

    DEF_MAKE(IntValue, INT);
    static IntValuePtr make(int v) {
        IntValuePtr p = make();
        p->ival = v;
        return p;
    }
};

struct FloatValue : public Value {
    float fval;

    virtual ValuePtr to_string() const;
    virtual ValuePtr to_int() const;
    virtual ValuePtr to_float() const;
    virtual ValuePtr to_number() const;
    virtual ValuePtr to_bool() const;
    
    DEF_MAKE(FloatValue, FLOAT);
    static FloatValuePtr make(float v) {
        FloatValuePtr p = make();
        p->fval = v;
        return p;
    }
};

struct StringValue : public Value {
    SymbolPtr sym;

    virtual ValuePtr to_string() const;
    virtual ValuePtr to_int() const;
    virtual ValuePtr to_float() const;
    virtual ValuePtr to_number() const;
    virtual ValuePtr to_bool() const;
    
    DEF_MAKE(StringValue, STR);
    static StringValuePtr make(SymbolPtr v) {
        StringValuePtr p = make();
        p->sym = v;
        return p;
    }
    static StringValuePtr make(const std::string_view& s) {
        return make(Symbol::find(s));
    }
    
    std::string as_print_string() const;
};

struct SymbolValue : public Value {
    IdentifierPtr sym = Identifier::make();
    virtual ValuePtr to_string() const;
    DEF_MAKE(SymbolValue, SYM);
    static SymbolValuePtr make(IndexPtr v) {
        SymbolValuePtr p = make();
        p->sym->append(v);
        return p;
    }
    void append(IndexPtr s) {
        sym->append(s);
    }
};

struct ContextValue : public Value {
    ContextPtr context;
    DEF_MAKE(ContextValue, CONTEXT);
    static ContextValuePtr make(ContextPtr c) {
        ContextValuePtr p = make();
        p->context = c;
        return p;
    }
    virtual ContextPtr get_context() const;
};

struct ClassValue : public ContextValue {
    SymbolPtr name;
    DEF_MAKE(ClassValue, CLASS);
    static ClassValuePtr make(ContextPtr c) {
        ClassValuePtr p = make();
        p->context = c;
        return p;
    }
    virtual SymbolPtr get_name() const;
};

struct ObjectValue : public ContextValue {
    ClassValuePtr parent;
    DEF_MAKE(ObjectValue, OBJECT);
};

struct ExceptionValue : public ContextValue {
    std::string err;
    ExceptionValuePtr parent;
    virtual ValuePtr to_string() const;
    DEF_MAKE(ExceptionValue, EXCEPTION);
    static ExceptionValuePtr make(const std::string& err, ContextPtr c) {
        // std::cout << "Making exception: " << err << std::endl;
        ExceptionValuePtr p = make();
        p->err = err;
        p->context = c;
        return p;
    }
    void setParent(ExceptionValuePtr p) { parent = p; }
};

struct FunctionValue : public Value {
    SymbolPtr name;
    ListValuePtr params, body;
    DEF_MAKE(FunctionValue, FUNC);
    virtual SymbolPtr get_name() const;
    // XXX set quote for no eval
};

struct OperatorValue : public Value {
    SymbolPtr name;
    built_in_f oper;
    uint8_t precedence;
    uint8_t order;
    DEF_MAKE(OperatorValue, OPER);
    static OperatorValuePtr make(SymbolPtr name, built_in_f f, uint8_t p, uint8_t o, bool no_eval) {
        OperatorValuePtr v = make();
        v->name = name;
        v->oper = f;
        v->precedence = p;
        v->order = o;
        v->quote = no_eval;
        return v;
    }
    virtual SymbolPtr get_name() const;
};

struct ListValue : public Value {
    std::vector<ValuePtr> list;
    
    ListValuePtr parent;
    int start, len;
    
    DEF_MAKE(ListValue, LIST);
    
    void append(ValuePtr v) { list.push_back(v); }
    
    // XXX return exception
    // Check bounds
    ListValuePtr sub(int s, int l = -1) {
        ListValuePtr p = make();
        if (parent) {
            // std::cout << "parent: " << start << " " << len << std::endl;
            p->parent = parent;
            p->start = start + s;
            if (l < 0) {
                p->len = len - s;
            } else {
                p->len = std::min(l, len - s);
            }
        } else {
            std::cout << "no parent\n";
            p->parent = shared_from_base<ListValue>();
            p->start = s;
            if (l < 0) {
                p->len = list.size() - s;
            } else {
                p->len = std::min(l, (int)list.size() - s);
            }
        }
        return p;
    }
    
    // XXX return exception
    ValuePtr get(int index) const {
        if (index < 0) return ExceptionValue::make("Index out of bounds", 0);
        if (parent) {
            if (index >= len) return NoneValue::make();
            return parent->get(index + start);
        } else {
            if (index >= list.size()) return NoneValue::make();
            return list[index];
        }
    }
    
    // XXX Return exception
    ValuePtr put(int index, ValuePtr v) {
        if (index < 0) return ExceptionValue::make("Index out of bounds", 0);
        if (parent) {
            parent->put(index + start, v);
        } else {
            while (index >= list.size()) list.push_back(NoneValue::make());
            list[index] = v;
        }
        return NoneValue::make();
    }
    
    int size() const {
        return parent ? len : list.size();
    }
    
    virtual ValuePtr to_string() const;
    
    std::ostream& print(std::ostream& os) const;
};

struct InfixValue : public ListValue {
    DEF_MAKE(InfixValue, INFIX);
    virtual ValuePtr to_string() const;
};

}; // namespace squirrel

#endif