#include "context.hpp"
#include "value.hpp"
#include "interpreter.hpp"

namespace squirrel {

ValuePtr wrap_exception(ValuePtr v, ContextPtr c)
{
    if (c) return c->wrap_exception(v);
    return v;
}


ValuePtr Context::set(IndexPtr s, ValuePtr t, ContextPtr caller) {
    if (s->has_index()) {
        ListValuePtr list = CAST_LIST(get(s->sym), shared_from_this());
        list->put(interp->evaluate(s->index, caller)->as_int(), t);
    } else {
        return set(s->sym, t);
    }
    return NoneValue::make();
}

ValuePtr Context::get(IndexPtr s, ContextPtr caller) {
    if (s->has_index()) {
        ListValuePtr list = CAST_LIST(get(s->sym), shared_from_this());
        return list->get(interp->evaluate(s->index, caller)->as_int());
    } else {
        return get(s->sym);
    }
}


// XXX return exception
ValuePtr Context::set(IdentifierPtr s, ValuePtr t, ContextPtr caller) {
    std::cout << "Set in context " << name << std::endl;
    ValuePtr cv = NULL_EXCEPTION(CHECK_EXCEPTION(find_owner(s, caller, true)), shared_from_this());
    std::cout << "Getting context\n";
    ContextPtr c = cv->get_context();
    if (!c) return ExceptionValue::make("Illegal null context", shared_from_this());
    std::cout << "Setting\n";
    return c->set(s->last(), t, caller);
}

ValuePtr Context::get(IdentifierPtr s, ContextPtr caller) {
    std::cout << "Get in context " << get_name() << std::endl;
    ValuePtr cv = NULL_EXCEPTION(CHECK_EXCEPTION(find_owner(s, caller, false)), shared_from_this());
    ContextPtr c = cv->get_context();
    if (!c) return ExceptionValue::make("Illegal null context", shared_from_this());
    return c->get(s->last(), caller);
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

ValuePtr Context::find_owner_local(IdentifierPtr s, ContextPtr caller, bool for_writing)
{
    if (!s->has_first()) return ExceptionValue::make(std::string("Invalid identifier: ") + s->original()->as_string(), shared_from_this());
    IndexPtr first = s->first();

    if (!vars.has_key(first->sym)) {
        if (first->has_index()) ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
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
        ValuePtr v = vars.get(first->sym);
        if (v->type == Value::LIST && first->has_index()) v = CAST_LIST(v, 0)->get(interp->evaluate(first->index, caller)->as_int());
        if (v->has_context()) return v->get_context()->find_owner_local(s->next(), caller, for_writing);
        return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
    }
}

ValuePtr Context::find_owner(IdentifierPtr s, ContextPtr caller, bool for_writing)
{
    std::cout << "Looking for " << s << " in " << get_name() << " writing=" << for_writing << std::endl;
    if (!s->has_first()) ExceptionValue::make(std::string("Invalid identifier: ") + s->original()->as_string(), shared_from_this());
    IndexPtr first = s->first();
    
    if (first->sym == Symbol::parent_symbol) {
        // Parent of global is itself
        if (!parent) return ContextValue::make(shared_from_this());
        return parent->find_owner(s->next(), caller, for_writing);
    } else if (first->sym == Symbol::global_symbol || first->sym == Symbol::class_symbol || first->sym == Symbol::object_symbol) {
        ValuePtr v = NULL_EXCEPTION(CHECK_EXCEPTION(find_ancestor_type(first->sym)), shared_from_this());
        if (v->has_context()) return v->get_context()->find_owner(s->next(), caller, for_writing);
        return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
    } else if (first->sym == Symbol::local_symbol) {
        return find_owner_local(s->next(), caller, for_writing);
    } else {
        if (vars.has_key(first->sym)) {
            std::cout << "Has key " << first->sym << std::endl;
            // If we have the key, then we've found the variable
            // If there are no more symbols, then we're done
            if (!s->has_next()) return ContextValue::make(shared_from_this());
            // Otherwise make sure the variable found if a context and ask it for the next symbol
            ValuePtr v = vars.get(first->sym);
            std::cout << "Checking for index\n";
            if (v->type == Value::LIST && first->has_index()) v = CAST_LIST(v, 0)->get(interp->evaluate(first->index, caller)->as_int());
            if (v->has_context()) return v->get_context()->find_owner(s->next(), caller, for_writing);
            return ExceptionValue::make(std::string("Not a context: ") + s->as_string(), shared_from_this());
        } else {
            std::cout << "No has key " << first->sym << std::endl;
            if (first->has_index()) ExceptionValue::make(std::string("No such identifier: ") + s->original()->as_string(), shared_from_this());
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
                return parent->find_owner(s, caller, false);
            }
        }
    }
}


} // namespace squirrel