#ifndef COMPOSE3_H
#define COMPOSE3_H

#include <functional>
#include "ternary.h"
#include "statefun.h"
#include "obsvfun.h"
#include "proposal.h"

template<class state_type, class obsv_type>
class compose3 : public ternary_function<state_type, state_type, obsv_type, precision_type>
{
    public:
        compose3(statefun<state_type> f,
                 obsvfun<state_type, obsv_type> g,
                 proposal<state_type,obsv_type> q):
                 f(f),g(g),q(q)
                 {}
        virtual ~compose3() {}
        precision_type virtual operator()(const state_type a, const state_type b, const obsv_type c) const{
            return f(a,b)*g(a,c)/q(a,b,c);
        }
        //compose3(const compose3& other) {}
        //compose3& operator=(const compose3& other) { return *this; }

    protected:
        statefun<state_type> f;
        obsvfun<state_type, obsv_type> g;
        proposal<state_type,obsv_type> q;
};

#endif // COMPOSE3_H
