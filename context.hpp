#ifndef INCLUDED_SQUIRREL_CONTEXT_HPP
#define INCLUDED_SQUIRREL_CONTEXT_HPP

#include "types.hpp"
#include "symbol.hpp"
#include "value.hpp"
#include "dictionary.hpp"

namespace squirrel {

struct Context : public std::enable_shared_from_this<Context> {
    ContextPtr parent;
    Dictionary vars;
    SymbolPtr name;
    SymbolPtr type;
        
    ValuePtr find_ancestor_type(SymbolPtr sym);
    ValuePtr find_owner(IdentifierPtr s, bool for_writing = false);
    ValuePtr find_owner_local(IdentifierPtr s, bool for_writing = false);
    
    ValuePtr set(IdentifierPtr s, ValuePtr t);
    ValuePtr get(IdentifierPtr s);
    
    ValuePtr set(SymbolPtr s, ValuePtr t) { 
        std::cout << "Set in context " << name << std::endl;
        vars.set(s, t); return 0;
    }
    // XXX wrap exception
    ValuePtr get(SymbolPtr s) { 
        std::cout << "Get in context " << name << std::endl;
        return vars.get(s); 
    }
        
    static ContextPtr make() { return std::make_shared<Context>(); }
    static ContextPtr make_global() { 
        ContextPtr c = make();
        c->type = Symbol::global_symbol;
        c->name = Symbol::global_symbol;
        return c;
    }
    
    ContextPtr make_child_context(SymbolPtr type, SymbolPtr name) {
        ContextPtr c = make();
        c->parent = shared_from_this();
        c->type = type;
        c->name = name;
        //if (name) set(name, ContextValue::make(c));
        return c;
    }
    
    ContextPtr make_function_context(SymbolPtr name) { 
        return make_child_context(Symbol::func_symbol, name);
    }
    ContextPtr make_class_context(SymbolPtr name) { 
        ContextPtr c = make_child_context(Symbol::class_symbol, name); 
        set(name, ClassValue::make(c));
        return c;
    }
    ContextPtr make_object_context(SymbolPtr name) { 
        return make_child_context(Symbol::object_symbol, name); 
    }
    
    ValuePtr wrap_exception(ValuePtr e) {
        if (e->type == Value::EXCEPTION) {
            ExceptionValuePtr new_ev = ExceptionValue::make();
            ExceptionValuePtr ev = std::static_pointer_cast<ExceptionValue>(e);
            ev->parent = new_ev;
            new_ev->context = shared_from_this();
            return new_ev;
        } else {
            return e;
        }
    }
};

} // namespace squirrel

#endif