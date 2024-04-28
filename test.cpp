#include "context.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <iostream>

squirrel::ValuePtr test_op(squirrel::ValuePtr in, squirrel::ContextPtr con) {
    std::cout << "called test with: " << in << std::endl;
    return in;
}

int main()
{
    squirrel::Interpreter interp;
    
    // squirrel::ValuePtr v = squirrel::StringValue::make("Test string");
    // std::cout << v << std::endl;
    // // squirrel::IdentifierPtr i = squirrel::Identifier::make("myvar");
    // // squirrel::IdentifierPtr j = squirrel::Identifier::make("global.myvar");
    // // c->set(i, v);
    // // std::cout << c->get(j) << std::endl;
    //
    // squirrel::ContextPtr c2 = c->make_class_context(squirrel::Symbol::make("class1"));
    // squirrel::IdentifierPtr k = squirrel::Identifier::make("class1.joe");
    // std::cout << c->set(k, squirrel::StringValue::make("value for joe")) << std::endl;
    // std::cout << c2->get(squirrel::Identifier::make("butt")) << std::endl;
    
    // squirrel::ListValuePtr l = squirrel::ListValue::make();
    // l->append(squirrel::StringValue::make("Test\nstring\033"));
    // std::cout << squirrel::ValuePtr(l) << std::endl;
    
    squirrel::OperatorValuePtr ov = squirrel::OperatorValue::make(squirrel::Symbol::make("test"), test_op, 0, 0);
    interp.global->set(squirrel::Symbol::make("test"), ov);
    
    squirrel::ValuePtr line = squirrel::Parser::parse("test 2");
    std::cout << interp.evaluate(line) << std::endl;
    
    // squirrel::FuncValuePtr fv = squirrel::FuncValue::make();
    // fv->name = Symbol::make("myfunc");
    // fv->
    // squirrel::ValuePtr v = squirrel::Parser::parse("{'{'\"some string\"} '5 '(3 + 4) 'quit '3.14 't}");
    // std::cout << v << std::endl;
    return 0;
}