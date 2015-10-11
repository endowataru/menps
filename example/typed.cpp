
#include "mgcom_pointer.hpp"
#include <iostream>

struct Y {
    int x;
};

struct X {
    Y y;
    Y y2;
};

int main()
{
    mgcom::typed_rma::remote_pointer<X> p;
    mgcom::typed_rma::remote_pointer<Y> q = p.member(&X::y);
    mgcom::typed_rma::remote_pointer<int> r = q.member(&Y::x);
    
    std::cout << r.to_address().offset;
}

