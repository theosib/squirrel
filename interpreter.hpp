#ifndef INCLUDED_SQUIRREL_INTERPRETER_HPP
#define INCLUDED_SQUIRREL_INTERPRETER_HPP

#include "value.hpp"
#include "context.hpp"
#include "parser.hpp"

namespace squirrel {

namespace OpOrder {
    enum {
        LASSOC,
        RASSOC,
        UNARY
    };
};

constexpr bool NoEval = true;
    
struct Interpreter {
    ContextPtr global = Context::make_global(this);
        
    ValuePtr evaluate(ValuePtr v, ContextPtr c = 0);
    ListValuePtr evaluate_list(ListValuePtr in, ContextPtr c = 0);
    ValuePtr evaluate_body(ListValuePtr in, ContextPtr c = 0);
    ValuePtr call_function(SymbolValuePtr name, ListValuePtr args, ContextPtr caller);
    
    void add_operator(const std::string_view& name, built_in_f op, int precedence = 0, int order = 0, bool no_eval = false);
    
    void load_operators();    
    Interpreter() { load_operators(); }
    
    ValuePtr parse(const std::string_view& s) {
        return Parser::parse(s);
    }
    ValuePtr evaluate(const std::string_view& s) {
        return evaluate(parse(s));
    }
};
    
}; // namespace squirrel

#endif
