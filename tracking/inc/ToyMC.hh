#ifndef TOYMC_HH
#define TOYMC_HH
#include <vector>
#include <memory>
#include <random>

#if 0
/*
 *  _____           __  __  ____
 * |_   _|__  _   _|  \/  |/ ___|
 *   | |/ _ \| | | | |\/| | |
 *   | | (_) | |_| | |  | | |___
 *   |_|\___/ \__, |_|  |_|\____|
 *            |___/
 *
 */
#endif

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
            double q;
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
            virtual void set_length(double length) = 0;
            virtual ~ToyMC(){};
            virtual ToyMC * clone() = 0;
    };

    //Lifetime are handled separately so 
    //components can easily be reused.
    class ToyMCLibrary
    {
        public:
            ToyMC * add_toy( std::unique_ptr<ToyMC>  toymc );

        private:
            std::vector<std::unique_ptr<ToyMC>> toy_lib_;
    };

    //--------------------------------------------------

    //Composite class holds pointers to other ToyMC's
    class ToyMCComposite : public ToyMC
    {
        public:
            ToyMCComposite()  = default;
            ToyMCComposite( const std::vector<ToyMC*>& children );
            track_params transfer( track_params tp ) const;
            track_params noisy_transfer( track_params tp ) const;
            double get_length() const;
            void set_length(double length);
            void add_child( ToyMC * child );
            virtual ToyMCComposite * clone();

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
            void set_length(double length);
            virtual ToyMCScatter * clone();

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

    class ToyMCThickScatter : public ToyMC
    {
        public:
            ToyMCThickScatter( RNGBase& gen, double rad_length, 
                    double length, int n_div );

            virtual track_params transfer( track_params ) const;
            virtual double get_length() const;
            void set_length(double length);

            virtual ToyMCThickScatter * clone();

        private:
            ToyMCScatter toy_scatter_;
            int n_div_;
            double length_;
    };

    //--------------------------------------------------

    class ToyMCDipoleBend : public ToyMC
    {
        public:
            ToyMCDipoleBend( double mom_kick, int polarity );
            void set_magnet_polarity( int polarity );
            virtual track_params transfer( track_params tp ) const;
            virtual double get_length() const;
            void set_length(double length);

            virtual ToyMCDipoleBend * clone();

        private:
            double mom_kick_;
            double polarity_;
    };

    //--------------------------------------------------

    class ToyMCPropagate : public ToyMC
    {
        public:
            ToyMCPropagate( double length );

            virtual track_params transfer( track_params ) const;
            virtual double get_length() const;
            void set_length(double length);
            virtual ToyMCPropagate * clone();

        private:
            double length_;
    };

}
#endif
