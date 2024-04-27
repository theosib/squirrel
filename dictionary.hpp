#ifndef INCLUDED_SQUIRREL_DICTIONARY_HPP
#define INCLUDED_SQUIRREL_DICTIONARY_HPP

#include "value.hpp"

namespace squirrel {

struct Dictionary {
    std::unordered_map<int, std::pair<SymbolPtr, ValuePtr>> entries;
    
    void set(SymbolPtr s, ValuePtr t) {
        std::cout << "Setting " << s << " to " << t << std::endl;
        entries[s->index] = {s, t};
    }
    
    void unset(SymbolPtr s) {
        entries.erase(s->index);
    }
    
    ValuePtr get(SymbolPtr s) {
        auto i = entries.find(s->index);
        if (i == entries.end()) {
            std::cout << "Variable " << s << " not found\n";
            return ExceptionValue::make(std::string("No such symbol ") + s->as_string(), 0);
        }
        ValuePtr r = i->second.second;
        std::cout << "Getting " << s << " as " << r << std::endl;
        return r;
    }
    
    bool has_key(SymbolPtr s) {
        return entries.find(s->index) != entries.end();
    }
};

typedef std::shared_ptr<Dictionary> DictionaryPtr;

} // namespace squirrel

#endif