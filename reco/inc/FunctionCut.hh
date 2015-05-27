#ifndef FUNCTIONCUT_HH
#define FUNCTIONCUT_HH
#include "Selection.hh"
#include "Event.hh"

#define REG_DEC_FC(NAME) \
    static SubscriberRegister<FunctionCut< NAME> > reg_

#define REG_DEF_FC(NAME) \
    template <> \
SubscriberRegister<FunctionCut<NAME>> FunctionCut<NAME>::reg_(#NAME) 

#define REG_CREATE_FC(NAME) \
    template<> Subscriber * \
create_subscriber<FunctionCut<NAME> > \
(  YAML::Node& instruct, RecoFactory& rf );

#define REG_DEF_CREATE_FC(NAME) \
    template<> Subscriber * \
create_subscriber<FunctionCut<NAME> > \
(  YAML::Node& instruct, RecoFactory& rf ) \
{ \
    return new FunctionCut<NAME>( rf.get_event_ptr() ); \
}


#if 0
/*
 *  _____                 _   _              ____      _   
 * |  ___|   _ _ __   ___| |_(_) ___  _ __  / ___|   _| |_ 
 * | |_ | | | | '_ \ / __| __| |/ _ \| '_ \| |  | | | | __|
 * |  _|| |_| | | | | (__| |_| | (_) | | | | |__| |_| | |_ 
 * |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|\____\__,_|\__|
 *                                                         
 * 
 */
#endif
namespace fn
{

    template <typename FunctionID>
        bool selection_function( const fne::Event * e );

    template <typename FunctionID>
        class FunctionCut : public CachedSelection
    {
        private:
            const fne::Event * e_;

            bool do_check() const
            { return selection_function<FunctionID>( e_) ; } 

            REG_DEC_FC( FunctionID );

        public:
            FunctionCut( const fne::Event * e )
                :e_( e ) {}

    };

    struct auto_pass{};


    REG_CREATE_FC( auto_pass );

    template<>
        bool selection_function<auto_pass>( const fne::Event * e );


    class LambdaCut: public CachedSelection
    {
        private:
            bool do_check() const;
            std::function<bool()> cut_function_;

        public:
            template <class F>
            LambdaCut( F cut_function )
            {
                cut_function_ = cut_function;
            }
    };

    class LambdaWgtCut: public CachedSelection
    {
        private:
            bool do_check() const;
            double do_weight() const;
            std::function<bool()> cut_function_;
            std::function<double()> wgt_function_;

        public:
            template <class F, class G>
            LambdaWgtCut( F cut_function, G wgt_function )
            {
                cut_function_ = cut_function;
                wgt_function_ = wgt_function;
            }
    };

}
#endif
