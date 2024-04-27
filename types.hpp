#ifndef INCLUDED_SQUIRREL_TYPES_HPP
#define INCLUDED_SQUIRREL_TYPES_HPP

#include <memory>

namespace squirrel {

#define DEF_SHARED_PTR(x) \
    struct x; \
    typedef std::shared_ptr<x> x##Ptr;

DEF_SHARED_PTR(Value);
DEF_SHARED_PTR(BoolValue);
DEF_SHARED_PTR(IntValue);
DEF_SHARED_PTR(FloatValue);
DEF_SHARED_PTR(StringValue);
DEF_SHARED_PTR(SymbolValue);
DEF_SHARED_PTR(ListValue);
DEF_SHARED_PTR(ClassValue);
DEF_SHARED_PTR(ObjectValue);
DEF_SHARED_PTR(ExceptionValue);
DEF_SHARED_PTR(OperatorValue);
DEF_SHARED_PTR(FunctionValue);
DEF_SHARED_PTR(InfixValue);
DEF_SHARED_PTR(ContextValue);
DEF_SHARED_PTR(Context);
DEF_SHARED_PTR(Dictionary);
DEF_SHARED_PTR(Identifier);

struct Symbol;
typedef std::shared_ptr<Symbol> SymbolPtr;
typedef std::weak_ptr<Symbol> SymbolWeakPtr;

#define CHECK_EXCEPTION(v) ({ const ValuePtr& val(v); if (val && val->type == Value::EXCEPTION) { std::cout << "returning exception:" << val << std::endl; return val; }; val; })

#define NULL_EXCEPTION(v, c) ({ const ValuePtr& val(v); if (!val) { std::cout << "Null ref\n"; return ExceptionValue::make("Illegal null reference", (c));}; val; })

typedef ValuePtr (*built_in_f)(ValuePtr, ContextPtr);

} // namespace squirrel

#endif