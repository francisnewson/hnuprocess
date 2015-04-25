#ifndef TOYMC_HH
#define TOYMC_HH
#include <vector>
#include <memory>
#include <random>

namespace toymc
{
    struct track_params
    {
        public:
            void shift_x( double xshift );
            void shift_y( double yshift );

            void kick_tx( double txkick );
            void kick_ty( double tykick );

            void transfer_z( double dz );

            const double get_mom(){ return p; };

            double p;
            double x;
            double tx;
            double y;
            double ty;
            double z;
    };

    //--------------------------------------------------

    //Inhreitance hierarch for random number generators
    struct RNGBase
    {
        virtual uint_fast32_t operator()() = 0;
        virtual uint_fast32_t max() = 0;
        virtual uint_fast32_t min() = 0;
        virtual ~RNGBase(){};
    };

    template <class RNG >
        struct SmartRNG : RNGBase {
            RNG gen;
            virtual uint_fast32_t operator()() { return gen(); } 
            virtual uint_fast32_t max(){ return gen.max() ; }
            virtual uint_fast32_t min(){ return gen.min() ; }
        };

    //--------------------------------------------------

    //ABC for ToyMC components
    class ToyMC
    {
        public:
            virtual track_params transfer( track_params ) const = 0;
            virtual double get_length() const = 0;
            virtual ~ToyMC(){};
    };

    //Lifetime are handled separately so 
    //components can easily be reused.
    class ToyMCLibrary
    {
        void add_toy( ToyMC * toymc );

        private:
        std::vector<std::unique_ptr<ToyMC>> toy_lib_;
    };

    //--------------------------------------------------

    //Composite class holds pointers to other ToyMC's
    class ToyMCComposite : public ToyMC
    {
        public:
            track_params transfer( track_params tp ) const;
            double get_length() const;
            void add_child( ToyMC * child );

        private:

            std::vector<ToyMC*> children_;
    };

    //--------------------------------------------------

    class ToyMCScatter : public ToyMC
    {
        public:
        ToyMCScatter( RNGBase& gen, double rad_length, double length );

        virtual track_params transfer( track_params ) const;
        virtual double get_length() const;

        private:
        RNGBase& gen_;

        double rad_length_;  //cm
        double length_; //cm
        double multiplier_; //GeV
        mutable std::normal_distribution<double> normal_;
    };

    double get_shift( double r1, double r2, double theta_0, double dz );
    double get_kick(  double r2, double theta_0 );

    //--------------------------------------------------

    class ToyMCDipoleBend : public ToyMC
    {
        public:
        virtual track_params transfer( track_params ) const;
        virtual double get_length() const;
        private:
    };

}
#endif
