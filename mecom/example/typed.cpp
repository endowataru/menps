
#include "mecom_pointer.hpp"
#include <iostream>

struct Y {
    int x;
};

struct X {
    Y y;
    Y y2;
};

mecom::index_t entry_size() {
    return 8;
}

int main()
{
    mecom::typed_rma::remote_ptr<X> p;
    mecom::typed_rma::remote_ptr<Y> q = p.member(&X::y);
    mecom::typed_rma::remote_ptr<int> r = q.member(&Y::x);
    
    std::cout << r.to_address().offset;
    
    mecom::typed_rma::remote_ptr< mecom::typed_rma::static_buffer<entry_size> > p2;
    
}

