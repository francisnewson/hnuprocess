#include "FunctionCut.hh"
#include "RecoFactory.hh"

namespace fn
{
    REG_DEF_FC( auto_pass );

    template<>
        bool selection_function<auto_pass>( const fne::Event * e )
        { return true; }

    REG_DEF_CREATE_FC(auto_pass);

    bool LambdaCut::do_check() const
    {
        return cut_function_();
    }
}
