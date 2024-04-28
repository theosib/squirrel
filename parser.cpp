#include "parser.hpp"

namespace squirrel {

int Parser::parse_octal(const char *src, int len)
{
    int val = 0;
    while (len) {
        len--;
        int c = *src++ - '0';
        val = (val * 8) + c;
    }
    return val;
}

int Parser::parse_decimal(const char *src, int len)
{
    int val = 0;
    bool neg = false;
    if (len && (*src == '-' || *src == '+')) {
        neg = (*src == '-');
        len--;
        src++;
    }
    while (len) {
        len--;
        int c = *src++ - '0';
        val = (val * 10) + c;
    }
    if (neg) val = -val;
    return val;
}

int Parser::parse_hex(const char *src, int len)
{
    int val = 0;
    while (len) {
        len--;
        int c = *src++;
        if (c>='a' && c<='f') {
            c = c - 'a' + 10;
        } else if (c>='A' && c<='F') {
            c = c - 'A' + 10;
        } else {
            c -= '0';
        }
        val = (val * 16) + c;
    }
    return val;
}


static char escapes[][2] = {
    'r', 13,
    'a', 7,
    'b', 8,
    'e', 27,
    'n', 10,
    't', 9,
};
constexpr int num_escapes = sizeof(escapes) / sizeof(escapes[0]);

static char translate_escape(char e)
{
    for (int i=0; i<num_escapes; i++) {
        if (e == escapes[i][0]) return escapes[i][1];
    }
    return e;
}

int Parser::parse_string(const char *src, int len_in, char *dst)
{
    Parsing p(src, len_in);
    char val;
    int len_out = 0;
    char c;
    
    while ((c = p.pop())) {
        if (c == '\\') {
            c = p.peek();
            if (c == 'x') {
                p.skip();
                p.set_mark();
                for (int i=0; i<2; i++) {
                    if (isxdigit(p.peek())) {
                        p.skip();
                    } else break;
                }
                val = parse_hex(p.get_mark(), p.mark_len());
            } else if (isdigit(c)) {
                p.set_mark();
                p.skip();
                for (int i=0; i<2; i++) {
                    if (isdigit(p.peek())) {
                        p.skip();
                    } else break;
                }
                val = parse_octal(p.get_mark(), p.mark_len());
            } else {
                val = translate_escape(c);
                p.skip();
            }
            *dst++ = val;
            len_out++;
        } else {
            *dst++ = c;
            len_out++;
        }
    }
    
    return len_out;
}

float Parser::parse_float(const char *str, int length)
{
    float result = 0.0f;
    bool negative = false;
    bool decimalPointEncountered = false;
    int decimals = 0;
    bool exponentEncountered = false;
    int exponent = 0;
    bool exponent_negative = false;

    for (int i = 0; i < length; ++i) {
        char c = str[i];

        if (c == '-' && i == 0) {
            negative = true;
        } else if (c == '+' && i == 0) {
            negative = false;
        } else if (isdigit(c)) {
            if (exponentEncountered) {
                exponent = exponent * 10 + (c - '0');
            } else if (decimalPointEncountered) {
                result = result * 10 + (c - '0');
                decimals++;
            } else {
                result = result * 10 + (c - '0');
            }
        } else if ((c == 'e' || c == 'E') && !exponentEncountered) {
            exponentEncountered = true;
        } else if ((c == '-' || c == '+') && exponentEncountered && (str[i-1] == 'e' || str[i-1] == 'E')) {
            if (c == '-') {
                exponent_negative = true;
            } else {
                exponent_negative = false;
            }
        } else if (c == '.' && !decimalPointEncountered && !exponentEncountered) {
            decimalPointEncountered = true;
        } else {
            break;
        }
    }
    
    if (exponentEncountered) {
        if (exponent_negative) exponent = -exponent;
        if (decimalPointEncountered) exponent -= decimals;
        result *= pow(10, exponent);
    } else if (decimalPointEncountered) {
        result *= pow(10, -decimals);
    }
    
    if (negative) result = -result;
    return result;
}

ValuePtr Parser::parse_number(Parsing& p)
{
    char c;
    int len;

    p.set_mark();
    char first = p.peek();
    
    bool is_float = false;
    int base = 10;
    if (first == '0') {
        p.skip();
        c = p.peek();
        if (c == 'x') {
            base = 16;
            p.skip();
        } else if (c == '.') {
            p.rewind();
        } else if (c >= '0' && c <= '7') {
            base = 8;
            p.rewind();
        } else {
            p.rewind();
        }
    }
        
    switch (base) {
    case 8:
        len = 0;
        while ((c = p.peek())) {
            if (c>='0' && c<='7') {
                p.skip();
                len++;
            } else break;
        }
        if (len > 0) {
            return IntValue::make(parse_octal(p.get_mark() + 1, p.mark_len()));
        } else {
            return Value::ZERO_INT;
        }
        break;
        
    case 16:
        len = 0;
        while ((c = p.peek())) {
            if (isxdigit(c)) {
                p.skip();
                len++;
            } else break;
        }
        if (len > 0) {
            return IntValue::make(parse_hex(p.get_mark() + 2, p.mark_len()));
        }
        break;
        
    default:
        {
            c = p.peek();
            if (c == '-' || c == '+') p.skip();
        }
        len = 0;
        while ((c = p.peek())) {
            if (c == '.' || c == 'e' || c == 'E') {
                is_float = true;
                p.skip();
                len++;
            } else if (isdigit(c)) {
                p.skip();
                len++;
            } else break;
        }
        if (len > 0) {
            if (is_float) {
                return FloatValue::make(parse_float(p.get_mark(), p.mark_len()));
            } else {
                return IntValue::make(parse_decimal(p.get_mark(), p.mark_len()));
            }
        }
        break;
    }
    
    return 0;
}

SymbolValuePtr Parser::parse_symbol(const char *str, int len)
{
    char ch;
    Parsing p(str, len);
    SymbolValuePtr s = SymbolValue::make();
    while (p.more()) {
        p.set_mark();
        while (p.more() && p.peek() != '.') p.skip();
        s->append(Symbol::make(std::string_view(p.get_mark(), p.mark_len())));
        p.skip();
    }
    return s;
}

// XXX error checking
ValuePtr Parser::parse_token(Parsing& p)
{
    char *s;
    bool quote = false;
    char c;
    int len;
    ValuePtr t;
    p.skip_space();
    
    char first = p.peek();
    switch (first) {
    case 0:
        return 0;
    case ')':
    case '}':
        p.pop();
        return 0;
    case '\'':
        p.pop();
        quote = true;
        p.skip_space();
        first = p.peek();
        break;
    }
    
    switch (first) {
    case '\"':
        p.pop();
        p.set_mark();
        while ((c = p.peek())) {
            if (c == '\\') {
                p.skip(2);
            } else if (c == '\"') {
                break;
            } else {
                p.skip();
            }
        }
        
        len = p.mark_len();
        s = new char[len];
        len = Parser::parse_string(p.get_mark(), len, s);
        t = StringValue::make(std::string_view(s, len));
        delete [] s;
        t->quote = quote;
        p.skip();
        return t;
        
    case '{':
        p.skip();
        t = ListValue::make();
        parse(p, t);
        t->quote = quote;
        return t;
        
    case '(':
        p.skip();
        t = InfixValue::make();
        parse(p, t);
        t->quote = quote;
        return t;
        // if (quote) {
        //     return t;
        // } else {
        //     return transform_infix(t);
        // }
    }
    
    if (isdigit(first) || first == '.' || first == '-' || first == '+') {
        t = Parser::parse_number(p);
        if (t) {
            t->quote = quote;
            return t;
        }
        
        while ((c = p.peek())) {
            if (isspace(c) || c == ')' || c == '}' || c == '(' || c == '{') {
                break;
            } else {
                p.skip();
            }
        }
        
        t = parse_symbol(p.get_mark(), p.mark_len());
        t->quote = quote;
        return t;
    }
    
    p.set_mark();
    while ((c = p.peek())) {
        if (isspace(c) || c == ')' || c == '}' || c == '(' || c == '{') {
            break;
        } else {
            p.skip();
        }
    }
    
    t = parse_symbol(p.get_mark(), p.mark_len());
    t->quote = quote;
    return t;
}


ValuePtr Parser::parse(Parsing& p, ValuePtr list)
{
    ListValuePtr l = list ? std::static_pointer_cast<ListValue>(list) : ListValue::make();
    for (;;) {
        ValuePtr n = parse_token(p);
        if (!n) break;
        l->append(n);
    }    
    return l;
}

}; // namespace squirrel
