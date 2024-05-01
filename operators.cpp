#include "interpreter.hpp"

namespace squirrel {

static ValuePtr builtin_defun(ListValuePtr list, ContextPtr context)
{    
    if (list->size() < 2) {
        return ExceptionValue::make(std::string("func requires name and args arguments: ") + list->as_string(), context);
    }
    
    // std::cout << "name\n";
    SymbolValuePtr name = CAST_SYMBOL(list->get(0), context);
    // std::cout << "params\n";
    ListValuePtr params = CAST_LIST(list->get(1), context);
    // std::cout << "body\n";
    ListValuePtr body = list->sub(2);
    
    // std::cout << "name: " << ValuePtr(name) << std::endl;
    // std::cout << "params: " << ValuePtr(params) << std::endl;
    // std::cout << "Body: " << ValuePtr(body) << std::endl;
    
    // Locate target context
    // std::cout << "context\n";
    ContextPtr owner = GET_CONTEXT(context->find_owner(name->sym, context, true), context);
        
    FunctionValuePtr f = FunctionValue::make();
    f->name = name->sym->last()->sym;
    f->params = params;
    f->body = body;
    
    // If the function name is quoted, calling it won't automatically evaluate the arguments
    f->quote = name->quote;

    CHECK_EXCEPTION(owner->set(f->name, f));
    return f;
}

static ValuePtr builtin_set(ListValuePtr list, ContextPtr context)
{
    if (list->size() < 2) {
        return ExceptionValue::make(std::string("set requires name and value: ") + list->as_string(), context);
    }
    
    SymbolValuePtr name = CAST_SYMBOL(list->get(0), context);
    ContextPtr owner = GET_CONTEXT(context->find_owner(name->sym, context, true), context);
    std::cout << "Going to eval\n";
    ValuePtr val = context->interp->evaluate(list->get(1), context);
    CHECK_EXCEPTION(owner->set(name->sym->last(), val, context));
    return val;
}

static ValuePtr builtin_print(ListValuePtr list, ContextPtr context)
{
    for (int i=0; i<list->size(); i++) {
        std::cout << list->get(i) << std::endl;
    }
    return NoneValue::make();
}

typedef ValuePtr (*combine_f)(ValuePtr a, ValuePtr b);

static ValuePtr cat_two(ValuePtr a, ValuePtr b)
{
    return StringValue::make(a->as_string() + b->as_string());
}

static ValuePtr oper_reduce_list(ListValuePtr list, ContextPtr context, ValuePtr initial, combine_f comb)
{
    for (int i=0; i<list->size(); i++) {
        ValuePtr item = list->get(i);
        initial = comb(initial, item);
    }
    return initial;
}

static ValuePtr builtin_cat(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::EMPTY_STR, cat_two);
}

void Interpreter::load_operators() {
    global->set(Symbol::make("true"), Value::TRUE);
    global->set(Symbol::make("false"), Value::FALSE);
    global->set(Symbol::make("none"), NoneValue::make());
    
#if 0
    add_operator("+", builtin_add, 4);
    add_operator("-", builtin_sub, 4);
    add_operator("*", builtin_mul, 3);
    add_operator("/", builtin_div, 3);
    add_operator("%", builtin_mod, 3);
    add_operator("**", builtin_pow, 1, Token::RASSOC);

    add_operator("&", builtin_and_bitwise, 8);
    add_operator("^", builtin_xor_bitwise, 9);
    add_operator("|", builtin_or_bitwise, 10);
    
    add_operator("&&", builtin_and_bool, 11);
    add_operator("and", builtin_and_bool, 11);
    add_operator("||", builtin_or_bool, 12);
    add_operator("or", builtin_or_bool, 12);
    add_operator("xor", builtin_or_bool, 13);
    
    add_operator("!!", builtin_notzero, 2, Token::UNARY);
    add_operator("!", builtin_not_bool, 2, Token::UNARY);
    add_operator("not", builtin_not_bool, 2, Token::UNARY);
    add_operator("~", builtin_not_bitwise, 2, Token::UNARY);
    add_operator("neg", builtin_neg, 2, Token::UNARY);
    
    add_operator("=", builtin_eq, 7);
    add_operator("==", builtin_eq, 7);
    add_operator("eq", builtin_eq, 7);
    add_operator("<>", builtin_ne, 7);
    add_operator("!=", builtin_ne, 7);
    add_operator("ne", builtin_ne, 7);

    add_operator("<", builtin_lt, 6);
    add_operator(">", builtin_gt, 6);
    add_operator("<=", builtin_le, 6);
    add_operator(">=", builtin_ge, 6);
#endif
    
    add_operator("cat", builtin_cat, 0);
    add_operator("print", builtin_print, 0);
    
    add_operator("func", builtin_defun, 0, 0, NoEval);
    add_operator("set", builtin_set, 0, 0, NoEval);
    // add_operator("set@", builtin_set_obj, 0, 0, NoEval);
    // add_operator("set@@", builtin_set_class, 0, 0, NoEval);
    // add_operator("class", builtin_defclass, 0, 0, NoEval);

    // add_operator("identity", builtin_identity, 0, Token::UNARY);
    // add_operator("int", builtin_int, 0, Token::UNARY);
    // add_operator("floor", builtin_floor, 0, Token::UNARY);
    // add_operator("ceil", builtin_ceil, 0, Token::UNARY);
    // add_operator("round", builtin_round, 0, Token::UNARY);
    // add_operator("str", builtin_str, 0, Token::UNARY);
    
    // add_operator("copy", builtin_shallow_copy, 0, Token::UNARY);
}

} // namespace squirrel