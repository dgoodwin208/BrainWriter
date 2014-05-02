#ifndef OBSVFUN_TEMPLATE_H
#define OBSVFUN_TEMPLATE_H

#include <functional>

template<class state_type, class obsv_type>
class obsvfun :
    public std::binary_function<state_type, obsv_type, precision_type>
{
    public:
        obsvfun();
        obsvfun(precision_type (*f)(state_type,obsv_type));
        virtual ~obsvfun();
        obsvfun(const obsvfun& other);
        obsvfun& operator=(const obsvfun& other);

        precision_type virtual operator() (const state_type a, const obsv_type b) const {  return f(a,b) ;}

    protected:
        precision_type (*f)(state_type, obsv_type );


};




template<class state_type, class obsv_type>
obsvfun<state_type,obsv_type>::obsvfun()
{
    //ctor
}

template<class state_type, class obsv_type>
obsvfun<state_type,obsv_type>::obsvfun(precision_type (*f)(state_type,obsv_type))
:f(f)
{

}

template<class state_type, class obsv_type>
obsvfun<state_type,obsv_type>::~obsvfun()
{
    //dtor
}

template<class state_type, class obsv_type>
obsvfun<state_type,obsv_type>::obsvfun(const obsvfun& other)
{
    //copy ctor
    f = other.f;
}

template<class state_type, class obsv_type>
obsvfun<state_type,obsv_type>& obsvfun<state_type,obsv_type>::operator=(const obsvfun& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    f = rhs.f;
    return *this;
}






#endif // OBSVFUN_TEMPLATE_H
