#ifndef INCLUDED_SQUIRREL_PARSER_HPP
#define INCLUDED_SQUIRREL_PARSER_HPP

#include "value.hpp"
#include "symbol.hpp"

namespace squirrel {

struct Parsing {
    const char *p = 0, *q = 0;
    int len = 0;
    char peek() { 
        if (len < 1) return 0;
        return *p;
    }
    void skip(int n=1) {
        if (len > n) {
            len -= n;
            p += n;
        } else {
            p += len;
            len = 0;
        }
    }
    char pop() { 
        if (len < 1) return 0;
        char val = *p++;
        len--;
        return val;
    }
    bool more() {
        return len>0;
    }
    void skip_space() {
        while (more() && isspace(*p)) { p++; len--; }
    }
    void set_mark() {
        q = p;
    }
    const char *get_mark() {
        return q;
    }
    int mark_len() {
        return p - q;
    }
    void rewind() {
        len += p - q;
        p = q;
    }
    
    Parsing() {}
    Parsing(const char *ptr, int l) : p(ptr), len(l) {}
};

struct Parser {
    static int parse_octal(const char *src, int len);
    static int parse_decimal(const char *src, int len);
    static int parse_hex(const char *src, int len);
    static int parse_string(const char *src, int len_in, char *dst);
    static float parse_float(const char *str, int length);
    static ValuePtr parse_number(Parsing& p);
    static SymbolValuePtr parse_symbol(Parsing& p);
    static ValuePtr parse_token(Parsing& p);
    static ValuePtr parse(Parsing& p, ValuePtr list = 0);
    static ValuePtr parse(const std::string_view& s) {
        Parsing p(s.data(), s.size());
        return parse(p);
    }
};

}; // namespace squirrel

#endif