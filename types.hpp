#ifndef INCLUDED_SQUIRREL_TYPES_HPP
#define INCLUDED_SQUIRREL_TYPES_HPP

#include <memory>

namespace squirrel {

#define DEF_SHARED_PTR(x) \
    struct x; \
    typedef std::shared_ptr<x> x##Ptr;

DEF_SHARED_PTR(Value);
DEF_SHARED_PTR(NoneValue);
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
DEF_SHARED_PTR(Index);

struct Symbol;
typedef std::shared_ptr<Symbol> SymbolPtr;
typedef std::weak_ptr<Symbol> SymbolWeakPtr;

struct Interpreter;

#define CHECK_EXCEPTION(v) ({ const ValuePtr& __val(v); if (__val && __val->type == Value::EXCEPTION) return __val; __val; })

#define CHECK_EXCEPTION_WRAP(v, c) ({ const ValuePtr& __val(v); if (__val && __val->type == Value::EXCEPTION) return c->wrap_exception(__val); __val; })

#define NULL_EXCEPTION(v, c) ({ const ValuePtr& __val(v); if (!__val) return ExceptionValue::make("Illegal null reference", (c)); __val; })

typedef ValuePtr (*built_in_f)(ListValuePtr, ContextPtr);

} // namespace squirrel

#endif