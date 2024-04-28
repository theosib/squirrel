#include "interpreter.hpp"

namespace squirrel {
    
ValuePtr Interpreter::evaluate(ValuePtr v, ContextPtr c)
{
    if (!c) c = global;
    
    std::cout << "Executing: " << v << std::endl;
    if (!v->quote) {
        if (v->type == Value::LIST) {
            std::cout << "Unquoted list\n";
            ListValuePtr l = std::static_pointer_cast<ListValue>(v);
            ValuePtr name = CHECK_EXCEPTION_WRAP(l->get(0), c);
            std::cout << "Name: " << name << std::endl;
            // XXX throw exception for invalid function call. If it's a list, just return that.
            if (name->type != Value::SYM) return v; 
            ListValuePtr args = l->sub(1);
            std::cout << "args: " << ValuePtr(args) << std::endl;
            return CHECK_EXCEPTION_WRAP(call_function(name, args, c), c);
        }
        if (v->type == Value::SYM) {
            SymbolValuePtr s = std::static_pointer_cast<SymbolValue>(v);
            return c->get(s);
        }
    }
    return v;
}

ValuePtr Interpreter::call_function(ValuePtr name, ListValuePtr args, ContextPtr caller)
{
    // Look up name to get function/operator
    ValuePtr func = CHECK_EXCEPTION(caller->get(name));
    if (func->type != Value::FUNC && func->type != Value::OPER) 
        return ExceptionValue::make(std::string("Not a valid function or operator: ") + func->as_string(), caller);
    
    // If the function/operator itself if not quoted, then evaluate all args
    if (!func->quote) args = evaluate_list(args, caller);
    
    if (func->type == Value::FUNC) {
        // For function, create local variable context
        ContextPtr c = caller->make_function_context(func->get_name());
        // Assign args
        FunctionValuePtr fv = std::static_pointer_cast<FunctionValue>(func);
        ListValuePtr params = fv->params;
        int n = std::min(args->size(), params->size());
        for (int i=0; i<n; i++) {
            ValuePtr param = params->get(i);
            if (param->type != Value::SYM) {
                return ExceptionValue::make(std::string("Not a valid function parameter: ") + params->as_string(), caller);
            }
            if (params->quote) {
                // If last param name is quoted, put rest of args into list
                c->set(param, args->sub(i));
                break;
            } else {
                c->set(param, args->get(i));
            }
        }
        // XXX deal with args/params mismatch
        // Execute body of function 
        return evaluate_list(fv->body, c);
    } else {
        std::cout << "Operator\n";
        OperatorValuePtr ov = std::static_pointer_cast<OperatorValue>(func);
        // Call operator
        return ov->oper(args, caller);
    }
}

ListValuePtr Interpreter::evaluate_list(ListValuePtr in, ContextPtr c)
{
    if (!c) c = global;

    if (in->quote) return in;
    ListValuePtr out = ListValue::make();
    for (int i=0; i<in->size(); i++) {
        ValuePtr v_in = in->get(i);
        ValuePtr v_out = evaluate(v_in, c);
        out->append(v_out);
    }
    return out;
}

}; // namespace squirrel
