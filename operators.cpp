#include "interpreter.hpp"

namespace squirrel {

typedef ValuePtr (*combine_f)(ValuePtr a, ValuePtr b);

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
    ContextPtr exec_context, func_context;    
    CHECK_EXCEPTION(context->find_owner(name->sym, context, exec_context, func_context, true));    
    // ContextPtr owner = GET_CONTEXT(context->find_owner(name->sym, context, true), context);
        
    FunctionValuePtr f = FunctionValue::make();
    f->name = name->sym->last()->sym;
    f->params = params;
    f->body = body;
    
    // If the function name is quoted, calling it won't automatically evaluate the arguments
    f->quote = name->quote;

    CHECK_EXCEPTION(exec_context->set(f->name, f));
    return f;
}

static ValuePtr builtin_set(ListValuePtr list, ContextPtr context)
{
    if (list->size() < 2) {
        return ExceptionValue::make(std::string("set requires name and value: ") + list->as_string(), context);
    }
    
    SymbolValuePtr name = CAST_SYMBOL(list->get(0), context);
    
    ContextPtr exec_context, func_context;    
    CHECK_EXCEPTION(context->find_owner(name->sym, context, exec_context, func_context, true));
    // ContextPtr owner = GET_CONTEXT(context->find_owner(name->sym, context, true), context);
    std::cout << "Going to eval\n";
    ValuePtr val = context->interp->evaluate(list->get(1), context);
    CHECK_EXCEPTION(exec_context->set(name->sym->last(), val, context));
    return val;
}

static ValuePtr builtin_print(ListValuePtr list, ContextPtr context)
{
    for (int i=0; i<list->size(); i++) {
        std::cout << list->get(i) << std::endl;
    }
    return NoneValue::make();
}

static ValuePtr builtin_defclass(ListValuePtr list, ContextPtr context)
{   
    if (list->size() < 1) {
        return ExceptionValue::make(std::string("class requires name: ") + list->as_string(), context);
    }
    
    SymbolValuePtr path_name = CAST_SYMBOL(list->get(0), context);
    SymbolPtr name = path_name->sym->last()->sym;
    
    ContextPtr exec_context, func_context;    
    CHECK_EXCEPTION(context->find_owner(path_name->sym, context, exec_context, func_context, true));
    // ContextPtr owner = GET_CONTEXT(context->find_owner(path_name->sym, context, true), context);
    
    ClassValuePtr cl = ClassValue::make();
    cl->name = name;
    cl->context = exec_context->make_class_context(name);
    
    CHECK_EXCEPTION(exec_context->set(name, cl));
    
    ListValuePtr init_code = list->sub(1);
    CHECK_EXCEPTION(context->interp->evaluate_body(init_code, cl->context));
    
    return cl;
}

static ValuePtr eq_two(ValuePtr a, ValuePtr b)
{
    std::cout << "Comparing " << a << " and " << b << std::endl;
    if (a->type == Value::NONE && b->type == Value::NONE) return Value::TRUE;
    if (a->type == Value::NONE) {
        if (b->type == Value::LIST || b->type == Value::INFIX) {
            return CAST_LIST(b, 0)->size() == 0 ? Value::TRUE : Value::FALSE;
        } else {
            return Value::FALSE;
        }
    }
    if (b->type == Value::NONE) {
        if (a->type == Value::LIST || a->type == Value::INFIX) {
            return CAST_LIST(a, 0)->size() == 0 ? Value::TRUE : Value::FALSE;
        } else {
            return Value::FALSE;
        }
    }
    
    if ((a->type == Value::LIST || a->type == Value::INFIX) && (b->type == Value::LIST || b->type == Value::INFIX)) {
        ListValuePtr al = CAST_LIST(a, 0);
        ListValuePtr bl = CAST_LIST(b, 0);
        if (al->size() != bl->size()) return Value::FALSE;
        for (int i=0; i<al->size(); i++) {
            ValuePtr c = eq_two(al->get(i), bl->get(i));
            if (c == Value::FALSE) return c;
        }
        return Value::TRUE;
    }
    
    if (a->type == Value::STR && b->type == Value::STR) {
        StringValuePtr as = CAST_STRING(a, 0);
        StringValuePtr bs = CAST_STRING(b, 0);
        return as->sym->code == bs->sym->code ? Value::TRUE : Value::FALSE;
    }

    if (a->type == Value::STR || b->type == Value::STR) {
        // XXX maybe implement this; requires conversion of other types to string
        return Value::FALSE;
    }
    
    if (a->type == Value::SYM || b->type == Value::SYM) {
        // XXX maybe implement this
        return Value::FALSE;
    }
    
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        std::cout << "Floats\n";
        return a->as_float() == b->as_float() ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::INT || b->type == Value::INT) {
        std::cout << "Ints " << a->as_int() << " and " << b->as_int() << "\n";
        return a->as_int() == b->as_int() ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::BOOL || b->type == Value::BOOL) {
        return a == b ? Value::TRUE : Value::FALSE;
    }
    
    return Value::FALSE;
}

static ValuePtr lt_two(ValuePtr a, ValuePtr b)
{
    if (a->type == Value::NONE && b->type == Value::NONE) return Value::FALSE;
    if (a->type == Value::NONE) {
        if (b->type == Value::LIST || b->type == Value::INFIX) {
            return CAST_LIST(b, 0)->size() == 0 ? Value::FALSE : Value::TRUE;
        } else {
            return Value::TRUE;
        }
    }
    if (b->type == Value::NONE) {
        return Value::FALSE;
    }
    
    if ((a->type == Value::LIST || a->type == Value::INFIX) && (b->type == Value::LIST || b->type == Value::INFIX)) {
        ListValuePtr al = CAST_LIST(a, 0);
        ListValuePtr bl = CAST_LIST(b, 0);
        int len = std::min(al->size(), bl->size());
        bool got_lt = true;
        for (int i=0; i<len; i++) {
            ValuePtr ai = al->get(i);
            ValuePtr bi = bl->get(i);
            ValuePtr c = lt_two(ai, bi);
            if (c == Value::TRUE) {
                got_lt = true;
            } else {
                c = eq_two(ai, bi);
                if (c == Value::FALSE) return c;
            }
        }
        if (al->size() > bl->size()) return Value::FALSE;
        if (al->size() < bl->size()) return Value::TRUE;
        return got_lt ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::STR && b->type == Value::STR) {
        StringValuePtr as = CAST_STRING(a, 0);
        StringValuePtr bs = CAST_STRING(b, 0);
        std::string& ar(as->sym->str);
        std::string& br(bs->sym->str);
        return ar < br ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::STR || b->type == Value::STR) {
        // XXX maybe implement this; requires conversion of other types to string
        return Value::FALSE;
    }
    
    if (a->type == Value::SYM || b->type == Value::SYM) {
        // XXX maybe implement this
        return Value::FALSE;
    }
    
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        return a->as_float() < b->as_float() ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::INT || b->type == Value::INT) {
        return a->as_int() < b->as_int() ? Value::TRUE : Value::FALSE;
    }
    
    if (a->type == Value::BOOL || b->type == Value::BOOL) {
        if (a == Value::FALSE && b == Value::TRUE) return b;
        return Value::FALSE;
    }
    
    return Value::FALSE;
}

static ValuePtr gt_two(ValuePtr a, ValuePtr b)
{
    return lt_two(b, a);
}

static ValuePtr le_two(ValuePtr a, ValuePtr b)
{
    if (gt_two(a, b) == Value::TRUE) return Value::FALSE;
    return Value::TRUE;
}

static ValuePtr ge_two(ValuePtr a, ValuePtr b)
{
    if (lt_two(a, b) == Value::TRUE) return Value::FALSE;
    return Value::TRUE;
}

static ValuePtr ne_two(ValuePtr a, ValuePtr b)
{
    if (eq_two(a, b) == Value::TRUE) return Value::FALSE;
    return Value::TRUE;
}

static ValuePtr and_two_bitwise(ValuePtr a, ValuePtr b)
{
    return IntValue::make(a->as_int() & b->as_int());
}

static ValuePtr and_two_bool(ValuePtr a, ValuePtr b)
{
    return BoolValue::make(a->as_bool() && b->as_bool());
}

static ValuePtr or_two_bitwise(ValuePtr a, ValuePtr b)
{
    return IntValue::make(a->as_int() | b->as_int());
}

static ValuePtr or_two_bool(ValuePtr a, ValuePtr b)
{
    return BoolValue::make(a->as_bool() || b->as_bool());
}

static ValuePtr xor_two_bitwise(ValuePtr a, ValuePtr b)
{
    return IntValue::make(a->as_int() ^ b->as_int());
}

// static ValuePtr xor_two_bool(ValuePtr a, ValuePtr b)
// {
//     return BoolValue::make(a->as_bool() != b->as_bool());
// }


static ValuePtr cat_two(ValuePtr a, ValuePtr b)
{
    return StringValue::make(a->as_string() + b->as_string());
}

static ValuePtr add_two(ValuePtr a, ValuePtr b)
{
    a = a->to_number(); b = b->to_number();
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        return FloatValue::make(a->as_float() + b->as_float());
    } else {
        return IntValue::make(a->as_int() + b->as_int());
    }
}

static ValuePtr mul_two(ValuePtr a, ValuePtr b)
{
    a = a->to_number(); b = b->to_number();
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        return FloatValue::make(a->as_float() * b->as_float());
    } else {
        return IntValue::make(a->as_int() * b->as_int());
    }
}

static ValuePtr sub_two(ValuePtr a, ValuePtr b)
{
    a = a->to_number(); b = b->to_number();
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        return FloatValue::make(a->as_float() - b->as_float());
    } else {
        return IntValue::make(a->as_int() - b->as_int());
    }
}

static ValuePtr div_two(ValuePtr a, ValuePtr b)
{
    a = a->to_number(); b = b->to_number();
    if (a->type == Value::FLOAT || b->type == Value::FLOAT) {
        return FloatValue::make(a->as_float() / b->as_float());
    } else {
        return IntValue::make(a->as_int() / b->as_int());
    }
}

static ValuePtr mod_two(ValuePtr a, ValuePtr b)
{
    return IntValue::make(a->as_int() % b->as_int());
}

static ValuePtr pow_two(ValuePtr a, ValuePtr b)
{
    a = a->to_number(); b = b->to_number();
    bool is_int = (a->type == Value::INT && b->type == Value::INT && CAST_INT(b, 0)->ival >= 0);
    float af = a->as_float();
    float bf = b->as_float();
    float cf = pow(af, bf);
    // std::cout << "af=" << af << " bf=" << bf << " cf=" << cf << std::endl;
    if (is_int && isfinite(cf) && cf <= std::numeric_limits<int>::max() && cf >= std::numeric_limits<int>::min()) {
        return IntValue::make(cf);
    } else {
        return FloatValue::make(cf);
    }
}

static ValuePtr oper_reduce_list(ListValuePtr list, ContextPtr context, ValuePtr initial, combine_f comb)
{
    for (int i=0; i<list->size(); i++) {
        ValuePtr item = list->get(i);
        initial = comb(initial, item);
    }
    return initial;
}

static ValuePtr oper_reduce_two(ListValuePtr list, ContextPtr context, combine_f comb)
{    
    if (list->size() == 0) return Value::ZERO_INT;
    if (list->size() == 1) return list->get(0);    
    return comb(list->get(0), list->get(1));
}


static ValuePtr builtin_cat(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::EMPTY_STR, cat_two);
}

static ValuePtr builtin_add(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::ZERO_INT, add_two);
}

static ValuePtr builtin_mul(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::ONE_INT, mul_two);
}

static ValuePtr builtin_sub(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, sub_two);
}

static ValuePtr builtin_pow(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, pow_two);
}

static ValuePtr builtin_div(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, div_two);
}

static ValuePtr builtin_mod(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, mod_two);
}

static ValuePtr builtin_eq(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, eq_two);
}

static ValuePtr builtin_ne(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, ne_two);
}

static ValuePtr builtin_lt(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, lt_two);
}

static ValuePtr builtin_gt(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, gt_two);
}

static ValuePtr builtin_le(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, le_two);
}

static ValuePtr builtin_ge(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_two(list, context, ge_two);
}

static ValuePtr builtin_and_bitwise(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::NEGONE_INT, and_two_bitwise);
}

static ValuePtr builtin_and_bool(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::TRUE, and_two_bool);
}

static ValuePtr builtin_or_bitwise(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::ZERO_INT, or_two_bitwise);
}

static ValuePtr builtin_or_bool(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::FALSE, or_two_bool);
}

static ValuePtr builtin_xor_bitwise(ListValuePtr list, ContextPtr context)
{
    return oper_reduce_list(list, context, Value::ZERO_INT, xor_two_bitwise);
}

// static ValuePtr builtin_xor_bool(ListValuePtr list, ContextPtr context)
// {
//     return oper_reduce_list(list, context, Token::bool_false, xor_two_bool);
// }

static ValuePtr builtin_notzero(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    return list->get(0)->to_bool();
}

static ValuePtr builtin_not_bool(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr c = list->get(0)->to_bool();
    if (c == Value::TRUE) return Value::FALSE;
    return Value::TRUE;
}

static ValuePtr builtin_not_bitwise(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr c = list->get(0);
    return IntValue::make(~ c->as_int());
}

static ValuePtr builtin_neg(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr c = list->get(0);
    if (c->type == Value::FLOAT) {
        return FloatValue::make(- c->as_float());
    } else {
        return IntValue::make(- c->as_int());
    }
}

static ValuePtr builtin_identity(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    return list->get(0);
}

static ValuePtr builtin_str(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return Value::EMPTY_STR;
    return list->get(0)->to_string();
}

static ValuePtr builtin_int(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return Value::ZERO_INT;
    return list->get(0)->to_int();
}

static ValuePtr builtin_float(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return Value::ZERO_FLOAT;
    return list->get(0)->to_float();
}

static ValuePtr builtin_floor(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr item = list->get(0);
    if (item->type == Value::INT) return item;
    
    float cf = floor(item->as_float());
    if (isfinite(cf) && cf <= std::numeric_limits<int>::max() && cf >= std::numeric_limits<int>::min()) {
        return IntValue::make(cf);
    } else {
        return FloatValue::make(cf);
    }
}

static ValuePtr builtin_ceil(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr item = list->get(0);
    if (item->type == Value::INT) return item;
    
    float cf = ceil(item->as_float());
    if (isfinite(cf) && cf <= std::numeric_limits<int>::max() && cf >= std::numeric_limits<int>::min()) {
        return IntValue::make(cf);
    } else {
        return FloatValue::make(cf);
    }
}

static ValuePtr builtin_round(ListValuePtr list, ContextPtr context)
{
    if (list->size() == 0) return NoneValue::make();
    ValuePtr item = list->get(0);
    if (item->type == Value::INT) return item;
    
    float cf = round(item->as_float());
    if (isfinite(cf) && cf <= std::numeric_limits<int>::max() && cf >= std::numeric_limits<int>::min()) {
        return IntValue::make(cf);
    } else {
        return FloatValue::make(cf);
    }
}

void Interpreter::load_operators() {
    global->set(Symbol::make("true"), Value::TRUE);
    global->set(Symbol::make("false"), Value::FALSE);
    global->set(Symbol::make("none"), NoneValue::make());
    
    add_operator("+", builtin_add, 4);
    add_operator("-", builtin_sub, 4);
    add_operator("*", builtin_mul, 3);
    add_operator("/", builtin_div, 3);
    add_operator("%", builtin_mod, 3);
    add_operator("**", builtin_pow, 1, OpOrder::RASSOC);

    add_operator("&", builtin_and_bitwise, 8);
    add_operator("^", builtin_xor_bitwise, 9);
    add_operator("|", builtin_or_bitwise, 10);
    
    add_operator("&&", builtin_and_bool, 11);
    add_operator("and", builtin_and_bool, 11);
    add_operator("||", builtin_or_bool, 12);
    add_operator("or", builtin_or_bool, 12);
    add_operator("xor", builtin_or_bool, 13);
    add_operator("~", builtin_not_bitwise, 2, OpOrder::UNARY);
    
    add_operator("!!", builtin_notzero, 2, OpOrder::UNARY);
    add_operator("!", builtin_not_bool, 2, OpOrder::UNARY);
    add_operator("not", builtin_not_bool, 2, OpOrder::UNARY);
    add_operator("neg", builtin_neg, 2, OpOrder::UNARY);
    
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
    
    add_operator("cat", builtin_cat, 0);
    add_operator("print", builtin_print, 0);
    
    add_operator("func", builtin_defun, 0, 0, NoEval);
    add_operator("set", builtin_set, 0, 0, NoEval);
    // add_operator("set@", builtin_set_obj, 0, 0, NoEval);
    // add_operator("set@@", builtin_set_class, 0, 0, NoEval);
    add_operator("class", builtin_defclass, 0, 0, NoEval);

    add_operator("identity", builtin_identity, 0, OpOrder::UNARY);
    add_operator("int", builtin_int, 0, OpOrder::UNARY);
    add_operator("float", builtin_float, 0, OpOrder::UNARY);
    add_operator("floor", builtin_floor, 0, OpOrder::UNARY);
    add_operator("ceil", builtin_ceil, 0, OpOrder::UNARY);
    add_operator("round", builtin_round, 0, OpOrder::UNARY);
    add_operator("str", builtin_str, 0, OpOrder::UNARY);
    
    // add_operator("copy", builtin_shallow_copy, 0, Token::UNARY);
    // list -- turn args into list
}

} // namespace squirrel