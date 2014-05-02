/// this class is an extension of class binder2nd and bind2nd in STL
/// it extends to ternary function object and binds the third argument

#ifndef binder3rd_H
#define binder3rd_H

#include <functional>
#include "proposal.h"

/// define function object bind3rd

template<class ternary_operation>
class binder3rd : public std::binary_function
                               <typename ternary_operation::first_argument_type,
                                typename ternary_operation::second_argument_type,
                                typename ternary_operation::result_type>
{
    public:
        binder3rd(const ternary_operation &x,
                  const typename ternary_operation::third_argument_type& y);

        typename ternary_operation::result_type operator()
            (const typename ternary_operation::first_argument_type a,
             const typename ternary_operation::second_argument_type b) const{
                return op(a,b,value);
            }

    protected:
        ternary_operation op;
        typename ternary_operation::third_argument_type value;
};


template<class ternary_operation>
binder3rd<ternary_operation>::binder3rd(
                const ternary_operation &x,
                const typename ternary_operation::third_argument_type& y)
                :op(x), value(y)
{
    //ctor
}



/// define function template bind3rd for binder3rd

template <class Operation, class T>
binder3rd<Operation> bind3rd (const Operation& op, const T& x)
{
  return binder3rd<Operation>(op, typename Operation::third_argument_type(x));
}


#endif // binder3rd_H
