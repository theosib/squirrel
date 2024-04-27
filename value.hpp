#ifndef INCLUDED_SQUIRREL_VALUE_HPP
#define INCLUDED_SQUIRREL_VALUE_HPP

#include "types.hpp"
#include "enable_shared_from_base.hpp"
#include <symbol.hpp>
#include <string_view>

namespace squirrel {

#define DEF_MAKE(x, t) \
    static x##Ptr make() { x##Ptr p = std::make_shared<x>(); p->type = t; return p; }

struct Value : public enable_shared_from_base<Value>  {
    enum {
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
    
    std::string as_string() const;
    
    bool has_context() {
        return type == CLASS || type == OBJECT || type == CONTEXT;
    }
    
    virtual ContextPtr get_context() const;
    
    static ValuePtr EMPTY_STR, ZERO_INT, ONE_INT, ZERO_FLOAT, ONE_FLOAT, TRUE, FALSE;
};

inline std::ostream& operator<<(std::ostream& os, const Value& s) {
    os << s.as_string();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ValuePtr& s) {
    os << (s ? s->as_string() : "[null value]");
    return os;
}


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
};

struct SymbolValue : public Value {
    IdentifierPtr sym;
    virtual ValuePtr to_string() const;
    DEF_MAKE(SymbolValue, SYM);
    static SymbolValuePtr make(SymbolPtr v) {
        SymbolValuePtr p = make();
        p->sym->append(v);
        return p;
    }
};

struct ListValue : public Value {
    std::vector<ValuePtr> list;
    
    ListValuePtr parent;
    int start, len;
    
    DEF_MAKE(ListValue, LIST);
    
    void append(ValuePtr v) { list.push_back(v); }
    
    // XXX return exception
    ListValuePtr sub(int s, int l) {
        ListValuePtr p = make();
        if (parent) {
            p->parent = parent;
            p->start = start + s;
            if (l < 0) {
                p->len = len - p->start;
            } else {
                p->len = len;
            }
        } else {
            p->parent = shared_from_base<ListValue>();
            p->start = s;
        }
        p->len = l;
        if (l < 0) {
            p->len = len - start;
        } else {
            p->len = len;
        }
        return p;
    }
    
    // XXX return exception
    ValuePtr get(int index) {
        if (parent) {
            return parent->get(index + start);
        } else {
            return list[index];
        }
    }
    
    // XXX Return exception
    ValuePtr put(int index, ValuePtr v) {
        if (parent) {
            parent->put(index + start, v);
        } else {
            list[index] = v;
        }
        return 0;
    }
};

struct InfixValue : public Value {
    std::vector<ValuePtr> list;
    DEF_MAKE(InfixValue, INFIX);
    void append(ValuePtr v) { list.push_back(v); }
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
    DEF_MAKE(ClassValue, CLASS);
    static ClassValuePtr make(ContextPtr c) {
        ClassValuePtr p = make();
        p->context = c;
        return p;
    }
};

struct ObjectValue : public ContextValue {
    ClassValuePtr parent;
    DEF_MAKE(ObjectValue, OBJECT);
};

struct ExceptionValue : public Value {
    std::string error;
    ExceptionValuePtr parent;
    ContextPtr context;
    virtual ValuePtr to_string() const;
    DEF_MAKE(ExceptionValue, EXCEPTION);
    static ExceptionValuePtr make(const std::string& err, ContextPtr c) {
        ExceptionValuePtr p = make();
        p->error = err;
        p->context = c;
        return p;
    }
    void setParent(ExceptionValuePtr p) { parent = p; }
};

struct FunctionValue : public Value {
    ListValuePtr params, body;
    DEF_MAKE(FunctionValue, FUNC);
};

struct OperatorValue : public Value {
    built_in_f oper;
    DEF_MAKE(OperatorValue, OPER);
};

}; // namespace squirrel

#endif