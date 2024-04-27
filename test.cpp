#include "context.hpp"
#include <iostream>

int main()
{
    squirrel::ValuePtr v = squirrel::StringValue::make("Test string");
    std::cout << v << std::endl;
    squirrel::ContextPtr c = squirrel::Context::make_global();
    // squirrel::IdentifierPtr i = squirrel::Identifier::make("myvar");
    // squirrel::IdentifierPtr j = squirrel::Identifier::make("global.myvar");
    // c->set(i, v);
    // std::cout << c->get(j) << std::endl;
    
    squirrel::ContextPtr c2 = c->make_class_context(squirrel::Symbol::make("class1"));
    squirrel::IdentifierPtr k = squirrel::Identifier::make("class1.joe");
    std::cout << c->set(k, squirrel::StringValue::make("value for joe")) << std::endl;
    std::cout << c2->get(squirrel::Identifier::make("joe")) << std::endl;
    return 0;
}