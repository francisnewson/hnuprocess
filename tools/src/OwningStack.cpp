#include "OwningStack.hh"

namespace fn
{

    OwningStack::OwningStack(){}

    OwningStack::OwningStack( std::string name, std::string title )
        :stack_( name.c_str(), title.c_str() ) {}

    void OwningStack::Add( std::unique_ptr<TH1> h,
            Option_t* option)
    {
        stack_.Add( h.get(), option );
        hist_ptrs_.push_back( h.get()) ;
        store_.AddHistogram( std::move( h ) );
    }

    void OwningStack::AddCloned( const std::unique_ptr<TH1>& h, 
            Option_t* option )
    {
        auto newh = std::unique_ptr<TH1>
        {  static_cast<TH1*>( h->Clone() ) };
        Add( std::move( newh), option );
    }

    Int_t OwningStack::Write( const char* name, 
            Int_t option, Int_t bufsize )
    {
        return stack_.Write( name, option, bufsize );
    }

    Int_t OwningStack::Write( std::string name , Int_t option , 
            Int_t bufsize  )
    {
        return Write( name.c_str(), option, bufsize );
    }

    THStack& OwningStack::Stack()
    {
        return stack_;
    }

    void OwningStack::Rebin( int rebin )
    {
        for ( auto& h : store_ )
        {
            h->Rebin( rebin );
        }

    }

    TH1 * OwningStack::GetTotal()
    {
        return static_cast<TH1*>(stack_.GetStack()->Last() );
    }
}
