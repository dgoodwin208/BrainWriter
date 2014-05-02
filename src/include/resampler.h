#ifndef RESAMPLER_TEMPLATE_H
#define RESAMPLER_TEMPLATE_H

#include <iostream>
#include <vector>
#include <random>
#include "ran_generator.h"

template<class state_type>
class resampler
{
    public:
        resampler(const std::vector<precision_type> &wi, const std::vector<state_type> &xi2, std::vector<state_type> &xi1);
        virtual ~resampler();
        void virtual operator ()() const {
            std::discrete_distribution<int> gen (wi.begin(),wi.end());
            for(typename std::vector<state_type>::iterator it = xi1.begin();
                    it<xi1.end();
                    it++){
                int index = gen(ran_gen::getInstance().get_gen());
                *it=xi2[index];
            }
        }

    protected:
        const std::vector<precision_type> &wi; ///< weight
        const std::vector<state_type> &xi2; ///< choose one from these
        std::vector<state_type> &xi1;

};



template<class state_type>
resampler<state_type>::resampler(const std::vector<precision_type> &wi, const std::vector<state_type> &xi2, std::vector<state_type> &xi1):
wi(wi), xi2(xi2), xi1(xi1)
{
    //ctor
}

/*template<class state_type>
resampler<state_type>::resampler(const resampler& other){

}

template<class state_type>
resampler<state_type>& resampler<state_type>::operator=(const resampler& other){

}*/


template<class state_type>
resampler<state_type>::~resampler()
{
    //dtor
}


#endif // RESAMPLER_TEMPLATE_H
