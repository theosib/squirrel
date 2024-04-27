#include "context.hpp"
#include <iostream>

int main()
{
    squirrel::ValuePtr v = squirrel::StringValue::make("Test string");
    std::cout << v << std::endl;
    squirrel::ContextPtr c = squirrel::Context::make_global();
    squirrel::IdentifierPtr i = squirrel::Identifier::make("myvar");
    squirrel::IdentifierPtr j = squirrel::Identifier::make("global.myvar");
    i->append(squirrel::Symbol::make("myvar"));
    c->set(i, v);
    std::cout << c->get(j) << std::endl;
    return 0;
}