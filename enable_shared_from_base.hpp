#ifndef INCLUDED_ENABLE_SHARED_FROM_BASE
#define INCLUDED_ENABLE_SHARED_FROM_BASE

#include <memory> 

template <class Base>
struct enable_shared_from_base : public std::enable_shared_from_this<Base> {
    template <class Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(std::enable_shared_from_this<Base>::shared_from_this());
    }
};

#endif
