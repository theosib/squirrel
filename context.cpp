#include "context.hpp"
#include "value.hpp"

namespace squirrel {

// XXX return exception
ValuePtr Context::set(IdentifierPtr s, ValuePtr t) {
    ValuePtr cv = NULL_EXCEPTION(CHECK_EXCEPTION(find_owner(s, true)), shared_from_this());
    ContextPtr c = cv->get_context();
    if (!c) return ExceptionValue::make("Illegal null context", shared_from_this());
    c->vars.set(s->last(), t);
    return 0;
}

ValuePtr Context::get(IdentifierPtr s) {
    ValuePtr cv = NULL_EXCEPTION(CHECK_EXCEPTION(find_owner(s, true)), shared_from_this());
    ContextPtr c = cv->get_context();
    if (!c) return ExceptionValue::make("Illegal null context", shared_from_this());
    return c->vars.get(s->last());
}

ValuePtr Context::find_ancestor_type(SymbolPtr sym)
{
    ContextPtr current = shared_from_this();
    while (current) {
        if (current->type == sym) return ContextValue::make(current);
        current = current->parent;
    }
    return ExceptionValue::make(std::string("No ancestor of type ") + sym->as_string(), shared_from_this());
}

ValuePtr Context::find_owner_local(IdentifierPtr s, bool for_writing)
{
    if (!s->has_first()) return ExceptionValue::make(std::string("Invalid identifier: ") + s->original()->as_string(), shared_from_this());
    SymbolPtr first = s->first();

    if (!vars.has_key(first)) {
        if (for_writing) {
            if (s->has_next()) {
                // The variable doesn't exist, but we have more symbols?
                return ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
            }
            return ContextValue::make(shared_from_this());
        }
        return ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
    } else {
        // If there are no more symbols, we've found the context
        if (!s->has_next()) return ContextValue::make(shared_from_this());
        // Otherwise make sure the current name is a context
        ValuePtr v = vars.get(first);
        if (v->has_context()) return v->get_context()->find_owner_local(s->next());
        return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
    }
}

ValuePtr Context::find_owner(IdentifierPtr s, bool for_writing)
{
    std::cout << "Looking for " << s << std::endl;
    if (!s->has_first()) ExceptionValue::make(std::string("Invalid identifier: ") + s->original()->as_string(), shared_from_this());
    
    SymbolPtr first = s->first();
    if (first == Symbol::parent_symbol) {
        // Parent of global is itself
        if (!parent) return ContextValue::make(shared_from_this());
        return parent->find_owner(s->next());
    } else if (first == Symbol::global_symbol || first == Symbol::class_symbol || first == Symbol::object_symbol) {
        ValuePtr v = NULL_EXCEPTION(CHECK_EXCEPTION(find_ancestor_type(first)), shared_from_this());
        if (v->has_context()) return v->get_context()->find_owner(s->next());
        return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
    } else if (first == Symbol::local_symbol) {
        return find_owner_local(s->next(), for_writing);
    } else {
        if (vars.has_key(first)) {
            // If we have the key, then we've found the variable
            // If there are no more symbols, then we're done
            if (!s->has_next()) return ContextValue::make(shared_from_this());
            // Otherwise make sure the variable found if a context and ask it for the next symbol
            ValuePtr v = vars.get(first);
            if (v->has_context()) return v->get_context()->find_owner(s->next());
            return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
        } else {
            if (for_writing) {
                if (s->has_next()) {
                    // The variable doesn't exist, but we have more symbols?
                    return ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
                }
                // Symbol not found, but we're writing, return current context
                return ContextValue::make(shared_from_this());
            } else {
                // For reading, we can search upwards in scope
                if (!parent) {
                    return ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
                }
                return parent->find_owner(s, false);
            }
        }
    }
}


} // namespace squirrel