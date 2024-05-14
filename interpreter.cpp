#include "interpreter.hpp"

namespace squirrel {
    
ValuePtr Interpreter::evaluate(ValuePtr v, ContextPtr c)
{
    if (!c) c = global;
    
    std::cout << "Executing: " << v << std::endl;
    if (!v->quote) {
        if (v->type == Value::LIST) {
            std::cout << "Unquoted list\n";
            ListValuePtr l = CAST_LIST(v, 0);
            ValuePtr name = CHECK_EXCEPTION_WRAP(l->get(0), c);
            std::cout << "Name: " << name << std::endl;
            if (name->type == Value::LIST) return evaluate_body(l, c);
            // XXX throw exception for invalid function call. If it's a list, just return that.
            if (name->type != Value::SYM) return v; 
            ListValuePtr args = l->sub(1);
            std::cout << "args: " << ValuePtr(args) << std::endl;
            return CHECK_EXCEPTION_WRAP(call_function(CAST_SYMBOL(name, 0), args, c), c);
        }
        if (v->type == Value::SYM) {
            SymbolValuePtr s = CAST_SYMBOL(v, 0);
            return c->get(s, c);
        }
    }
    return v;
}

ValuePtr Interpreter::call_function(SymbolValuePtr name, ListValuePtr args, ContextPtr caller)
{
    if (caller->stack_depth >= 1000) {
        return ExceptionValue::make(std::string("Call stack limit exceeded: ") + name->as_string(), caller);
    }
    
    ContextPtr exec_context, func_context;    
    // Look up name to get function/operator
    ValuePtr func = CHECK_EXCEPTION(caller->get(name, caller, exec_context, func_context));
    if (func->type != Value::FUNC && func->type != Value::OPER) 
        return ExceptionValue::make(std::string("Not a valid function or operator: ") + func->as_string(), caller);
    
    // If the function/operator itself if not quoted, then evaluate all args
    if (!func->quote) args = evaluate_list(args, caller);
    
    if (func->type == Value::CLASS) {
        ObjectValuePtr obj = ObjectValue::make();
        obj->parent = CAST_CLASS(func, 0);
        ContextPtr class_context = func->get_context();
        obj->context = class_context->make_object_context(obj->parent->name);
        ValuePtr init = class_context->get(Symbol::make("_init"));
        if (init->type == Value::FUNC) {
            FunctionValuePtr init_func = CAST_FUNC(init, 0);
            evaluate_body(init_func->body, obj->context);
        }
        evaluate_body(args, obj->context);
        return obj;
    } else if (func->type == Value::FUNC) {        
        // For function, create local variable context
        ContextPtr c;
        if (func_context->type == Symbol::class_symbol) {
            c = exec_context->make_function_context(func->get_name());
            c->stack_depth = caller->stack_depth+1;
        } else {
            c = caller->make_function_context(func->get_name());
        }
        // Assign args
        FunctionValuePtr fv = CAST_FUNC(func, 0);
        ListValuePtr params = fv->params;
        int n = std::min(args->size(), params->size());
        for (int i=0; i<n; i++) {
            ValuePtr param = params->get(i);
            if (param->type != Value::SYM) {
                return ExceptionValue::make(std::string("Not a valid function parameter: ") + params->as_string(), caller);
            }
            if (params->quote) {
                // If last param name is quoted, put rest of args into list
                c->set(param, evaluate_list(args->sub(i), caller), caller);
                break;
            } else {
                c->set(param, evaluate(args->get(i), caller), caller);
            }
        }
        // XXX deal with args/params mismatch
        // Execute body of function 
        return evaluate_body(fv->body, c);
    } else {
        std::cout << "Operator\n";
        OperatorValuePtr ov = CAST_OPER(func, 0);
        // Call operator
        ValuePtr vp = ov->oper(args, caller);
        // std::cout << "operator returns: " << vp << std::endl;
        return vp;
    }
}

ListValuePtr Interpreter::evaluate_list(ListValuePtr in, ContextPtr c)
{
    if (!c) c = global;

    std::cout << "Evaluating list: " << ValuePtr(in) << std::endl;
    if (in->quote) return in;
    ListValuePtr out = ListValue::make();
    for (int i=0; i<in->size(); i++) {
        ValuePtr v_in = in->get(i);
        ValuePtr v_out = evaluate(v_in, c);
        out->append(v_out);
    }
    return out;
}

ValuePtr Interpreter::evaluate_body(ListValuePtr in, ContextPtr c)
{
    if (!c) c = global;

    std::cout << "Evaluating body: " << ValuePtr(in) << std::endl;
    ValuePtr out;
    for (int i=0; i<in->size(); i++) {
        ValuePtr v_in = in->get(i);
        out = evaluate(v_in, c);
    }
    return out;
}

void Interpreter::add_operator(const std::string_view& name, built_in_f op, int precedence, int order, bool no_eval)
{
    SymbolPtr sym = Symbol::make(name);
    OperatorValuePtr ov = squirrel::OperatorValue::make(sym, op, precedence, order, no_eval);
    global->set(sym, ov);
}

}; // namespace squirrel
