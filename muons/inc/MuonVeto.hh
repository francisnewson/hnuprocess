#ifndef MUONVETO_HH
#define MUONVETO_HH
#include "Subscriber.hh"
#include "PolarityFinder.hh"
#include "RecoFactory.hh"
#include <vector>
#if 0
/*
 *  __  __                __     __   _
 * |  \/  |_   _  ___  _ _\ \   / /__| |_ ___
 * | |\/| | | | |/ _ \| '_ \ \ / / _ \ __/ _ \
 * | |  | | |_| | (_) | | | \ V /  __/ || (_) |
 * |_|  |_|\__,_|\___/|_| |_|\_/ \___|\__\___/
 *
 *
 */
#endif
namespace fne
{
    class Event;
    class Mc;
}
namespace fn
{
    class SingleTrack;

    int get_muon_status( bool muv1, bool muv2, bool muv3 );

    //**************************************************
    // MuonVeto : provides status for muon cuts
    //**************************************************

    //abstract base class ( implements cacheing )
    class MuonVeto : public Subscriber
    {
        public:
            void new_event();

            virtual int get_muv_status()  const ;
            virtual bool get_muv1()  const;
            virtual bool get_muv2()  const;
            virtual bool get_muv3()  const;

            REG_DEC_SUB( MuonVeto );

        protected:
            //This is how derived classes return their results
            void set_muvs(  bool muv1, bool muv2, bool muv3 ) const;

        private:
            virtual void process_event() const = 0;

            mutable bool dirty_;
            mutable int status_;
            mutable bool muv1_;
            mutable bool muv2_;
            mutable bool muv3_;
    };

    template<>
        Subscriber * create_subscriber<MuonVeto>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //Looks for any muon and takes OR of everything
    class DataMuonVeto : public MuonVeto
    {
        public:
            DataMuonVeto( const fne::Event * e );

        private:
            void process_event() const;
            const fne::Event * e_;
    };

    //--------------------------------------------------

    //Looks for muon with correct Track ID
    class DataMatchedMuonVeto : public MuonVeto
    {
        public:
            DataMatchedMuonVeto( const fne::Event * e,
                    const SingleTrack& st
                    );

        private:
            void process_event() const;
            const fne::Event * e_;
            const SingleTrack& st_;
    };

    //--------------------------------------------------

    //Extrapolates track and applies per strip effs
    //(specified in constructor )
    class MCMuonVeto : public MuonVeto
    {
        public:
            MCMuonVeto( const fne::Event * e,
                    const SingleTrack&, 
                    std::vector<double> muv1_effs,
                    std::vector<double> muv2_effs
                    );

        private:
            void process_event() const;
            const fne::Event * e_;
            const SingleTrack& st_;

            PolarityFinder pf_;
            std::vector<double> muv1_effs_;
            std::vector<double> muv2_effs_;
            mutable std::uniform_real_distribution<double> uni_dist_;
            mutable std::default_random_engine generator_;
    };

    //--------------------------------------------------

    //Handle efficiencies on a 2D grid

    class Eff2D
    {
        public:
            Eff2D( std::vector<double> xedges, 
                    std::vector<double> yedges, 
                    std::vector<double> effs );

                double efficiency( double x, double y ) const;

        private:
            std::size_t get_bin( double val, const std::vector<double>& edges ) const;

            std::vector<double> xedges_;
            std::vector<double> yedges_;
            std::vector<double> effs_;
    };

    //--------------------------------------------------

    class MCXYMuonVeto : public MuonVeto
    {
        public:
            MCXYMuonVeto( const fne::Event * e, 
                    const SingleTrack& st , Eff2D muv_eff );

        private:
            void process_event() const;
            const fne::Event * e_;
            const SingleTrack& st_;

            PolarityFinder pf_;
            Eff2D muv_eff_;

            mutable std::uniform_real_distribution<double> uni_dist_;
            mutable std::default_random_engine generator_;
    };

    std::pair<double, double> muv_impact(
            const fne::Mc & mce, std::size_t muon_pos, int polarity );

    std::size_t find_muon( const fne::Mc& mce );

    //--------------------------------------------------

    MuonVeto * get_muon_veto
        ( YAML::Node& instruct, RecoFactory& rf );

}
#endif
