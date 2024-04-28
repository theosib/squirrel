#ifndef INCLUDED_SQUIRREL_INTERPRETER_HPP
#define INCLUDED_SQUIRREL_INTERPRETER_HPP

#include "value.hpp"
#include "context.hpp"
#include "parser.hpp"

namespace squirrel {
    
struct Interpreter {
    ContextPtr global = Context::make_global(this);
    
    ValuePtr parse(const std::string_view& s) {
        return Parser::parse(s);
    }
    
    ValuePtr evaluate(ValuePtr v, ContextPtr c = 0);
    ListValuePtr evaluate_list(ListValuePtr in, ContextPtr c = 0);
    ValuePtr call_function(ValuePtr name, ListValuePtr args, ContextPtr caller);
};
    
}; // namespace squirrel

#endif
