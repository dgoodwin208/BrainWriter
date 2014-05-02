#ifndef TERNARY_H_INCLUDED
#define TERNARY_H_INCLUDED

template <class Arg1, class Arg2, class Arg3, class Result>
  struct ternary_function {
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;
    typedef Arg3 third_argument_type;
    typedef Result result_type;
  };

#endif // TERNARY_H_INCLUDED
