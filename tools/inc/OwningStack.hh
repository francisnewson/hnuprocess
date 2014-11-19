#ifndef OWNINGSTACK_HH
#define OWNINGSTACK_HH

#include "THStack.h"
#include "HistStore.hh"
#include <vector>

namespace fn
{

    /* **************************************************
     * A wrapper for THStack which takes ownership of
     * histograms either with std::move or by cloning
     * its own copy
     * **************************************************/

    class OwningStack
    {
        //Constructors
        public:
        OwningStack();
        OwningStack( std::string name, std::string title );

        void Add( std::unique_ptr<TH1> h, Option_t* option = "" );
        void AddCloned( const std::unique_ptr<TH1>& h,
                Option_t* option = "" );

        Int_t Write( const char* name = 0, Int_t option = 0, 
                Int_t bufsize = 0 );

        Int_t Write( std::string name , Int_t option = 0, 
                Int_t bufsize = 0 );

        void Rebin( int rebin );

        THStack& Stack();

        typedef std::vector<TH1*>::iterator iterator;
        typedef std::vector<TH1*>::const_iterator const_iterator;
        typedef std::vector<TH1*>::size_type size_type;

        iterator begin() { return hist_ptrs_.begin();}
        iterator end() { return hist_ptrs_.end();}
        size_type size() const { return hist_ptrs_.size() ; }

        const_iterator begin() const { return std::begin(hist_ptrs_);}
        const_iterator end() const { return std::end(hist_ptrs_);}

        TH1 * GetTotal();

        private:
        THStack stack_;
        HistStore store_;
        std::vector<TH1*> hist_ptrs_;
    };
}


#endif
