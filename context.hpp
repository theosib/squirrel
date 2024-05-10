#ifndef INCLUDED_SQUIRREL_CONTEXT_HPP
#define INCLUDED_SQUIRREL_CONTEXT_HPP

#include "types.hpp"
#include "symbol.hpp"
#include "value.hpp"
#include "dictionary.hpp"

namespace squirrel {

struct Context : public std::enable_shared_from_this<Context> {
    Interpreter *interp;
    ContextPtr parent;
    Dictionary vars;
    SymbolPtr name;
    SymbolPtr type;
        
    ValuePtr find_ancestor_type(SymbolPtr sym);
    ValuePtr find_owner(IdentifierPtr s, ContextPtr caller, ContextPtr& exec_context, ContextPtr& func_context, bool for_writing = false);
    ValuePtr find_owner_local(IdentifierPtr s, ContextPtr caller, ContextPtr& exec_context, ContextPtr& func_context, bool for_writing = false);
    
    ValuePtr set(IdentifierPtr s, ValuePtr t, ContextPtr caller);
    ValuePtr get(IdentifierPtr s, ContextPtr caller);
    ValuePtr get(IdentifierPtr s, ContextPtr caller, ContextPtr& exec_context, ContextPtr& func_context);
    
    ValuePtr set(IndexPtr s, ValuePtr t, ContextPtr caller);
    ValuePtr get(IndexPtr s, ContextPtr caller);
    
    // XXX wrap exception
    ValuePtr get(SymbolPtr s) { 
        std::cout << "Get in context " << get_name() << std::endl;
        return vars.get(s); 
    }
    
    ValuePtr set(SymbolPtr s, ValuePtr t) { 
        std::cout << "Set in context " << get_name() << std::endl;
        vars.set(s, t); 
        return NoneValue::make();
    }
    
    ValuePtr get(ValuePtr s, ContextPtr caller) { return get(CAST_SYMBOL(s, shared_from_this())->sym, caller); }
    ValuePtr get(ValuePtr s, ContextPtr caller, ContextPtr& exec_context, ContextPtr& func_context) { 
        return get(CAST_SYMBOL(s, shared_from_this())->sym, caller, exec_context, func_context); 
    }
    ValuePtr set(ValuePtr s, ValuePtr t, ContextPtr caller) { return set(CAST_SYMBOL(s, shared_from_this())->sym, t, caller); }
        
    static ContextPtr make(Interpreter *i) { 
        ContextPtr c = std::make_shared<Context>(); 
        c->interp = i;
        return c;
    }
    static ContextPtr make_global(Interpreter *i) { 
        ContextPtr c = make(i);
        c->type = Symbol::global_symbol;
        c->name = Symbol::global_symbol;
        return c;
    }
    
    ContextPtr make_child_context(SymbolPtr type, SymbolPtr name) {
        ContextPtr c = make(interp);
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
            ExceptionValuePtr ev = CAST_EXCEPTION(e, 0);
            auto self = shared_from_this();
            if (ev->context == self) return e;
            
            ExceptionValuePtr new_ev = ExceptionValue::make();
            std::cout << "Wrapping exception: " << ValuePtr(ev) << std::endl;
            new_ev->context = self;
            new_ev->parent = ev;
            return new_ev;
        } else {
            return e;
        }
    }
    
    SymbolPtr get_name() {
        if (!name) return type;
        return name;
    }
};

} // namespace squirrel

#endif