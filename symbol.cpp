#include "symbol.hpp"
#include <sstream>
#include "value.hpp"

namespace squirrel {

std::ostream& operator<<(std::ostream& os, const Index& ix) {
    os << ix.sym;
    if (ix.has_index()) {
        //std::cout << "Printing index of type " << ix.index->type << "\n";
        os << '[' << ix.index << ']';
    }
    return os;
}

std::vector<SymbolWeakPtr> Symbol::interns;

SymbolPtr Symbol::empty_symbol = Symbol::find("");
SymbolPtr Symbol::parent_symbol = Symbol::find("parent");
SymbolPtr Symbol::global_symbol = Symbol::find("global");
SymbolPtr Symbol::class_symbol = Symbol::find("class");
SymbolPtr Symbol::object_symbol = Symbol::find("object");
SymbolPtr Symbol::local_symbol = Symbol::find("local");
SymbolPtr Symbol::func_symbol = Symbol::find("func");

SymbolPtr Symbol::find(const std::string_view& str)
{
    int free_code = -1;
    for (int i=0; i<interns.size(); i++) {
        SymbolPtr s = interns[i].lock();
        if (!s) {
            free_code = i;
        } else {
            if (str == s->str) return s;
        }
    }
    if (free_code < 0) {
        free_code = interns.size();
        interns.resize(free_code+1);
    }
    SymbolPtr s = Symbol::make();
    s->str = str;
    s->code = free_code;
    interns[free_code] = s;
    return s;  
}

std::string Identifier::as_string()
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

IdentifierPtr Identifier::make(const std::string_view& s)
{
    std::cout << "Parsing " << s << std::endl;
    IdentifierPtr i = make();
    const char *p = s.data();
    const char *e = p + s.size();
    const char *q;
    while (p < e) {
        q = p;
        while (q < e && *q != '.') q++;
        i->append(std::string_view(p, q-p));
        p = q + 1;
    }
    return i;
}


} // namespace squirrel
