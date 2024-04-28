#ifndef INCLUDED_SQUIRREL_SYMBOL_HPP
#define INCLUDED_SQUIRREL_SYMBOL_HPP

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <vector>
#include <string_view>
#include <string>
#include <iostream>
#include "types.hpp"

namespace squirrel {

struct Symbol {
    std::string str;
    int index;
    
    Symbol() {}
    Symbol(const std::string_view& s_in, int ix) {
        str = s_in;
        index = ix;
    }
    
    static SymbolPtr make() {
        return std::make_shared<Symbol>();
    }
    
    static SymbolPtr find(const std::string_view& str);
    static SymbolPtr make(const std::string_view& str) { return find(str); }
    
    static std::vector<SymbolWeakPtr> interns;
    static SymbolPtr empty_symbol, parent_symbol, global_symbol, class_symbol, object_symbol, local_symbol, func_symbol;
    
    bool operator==(const Symbol& other) {
        return other.index == index;
    }
    bool operator!=(const Symbol& other) {
        return other.index != index;
    }
    
    const std::string& as_string() { return str; }
};

inline std::ostream& operator<<(std::ostream& os, const Symbol& s) {
    os << s.str;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const SymbolPtr& s) {
    if (!s) {
        os << "[no symbol]";
    } else {
        os << *s;
    }
    return os;
}

struct Identifier : public std::enable_shared_from_this<Identifier> {
    std::vector<SymbolPtr> syms;

    IdentifierPtr parent;
    int offset = 0;
    
    void append(SymbolPtr s) { 
        //std::cout << "Appending " << s << std::endl;
        if (parent) {
            parent->syms.push_back(s);
        } else {
            syms.push_back(s);
        }
    }
    
    void append(const std::string_view& s) { append(Symbol::make(s)); }
    
    static IdentifierPtr make() {
        return std::make_shared<Identifier>();
    }
    
    static IdentifierPtr make(const std::string_view& s);
    
    bool has_next() const {
        int next_offset = offset+1;
        return parent ? (next_offset < parent->syms.size()) : (next_offset < syms.size());
    }
    
    IdentifierPtr next() {
        if (!has_next()) return 0;
        
        IdentifierPtr n = make();
        if (parent) {
            n->parent = parent;
            n->offset = offset+1;
        } else {
            n->parent = shared_from_this();
            n->offset = 1;
        }
        return n;
    }
    
    SymbolPtr first() const {
        return parent ? parent->syms[offset] : syms[offset];
    }
    
    SymbolPtr last() const {
        return parent ? parent->syms[parent->syms.size()-1] : syms[syms.size()-1];
    }
    
    bool has_first() const {
        return parent ? (offset < parent->syms.size()) : (offset < syms.size());
    }
    
    IdentifierPtr original() {
        if (parent) return parent;
        return shared_from_this();
    }
    
    std::string as_string();
    
    auto begin() const {
        if (parent) {
            return parent->syms.cbegin() + offset;
        } else {
            return syms.cbegin();
        }
    }
    
    auto end() const {
        if (parent) {
            return parent->syms.cend();
        } else {
            return syms.cend();
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const Identifier& i) {
    bool first = true;
    for (const auto& s : i) {
        if (!first) os << '.';
        os << s;
        first = false;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const IdentifierPtr& s) {
    if (!s) {
        os << "[no identifier]";
    } else {
        os << *s;
    }
    return os;
}

}; // namespace squirrel

#endif