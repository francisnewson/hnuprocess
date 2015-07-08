#include <random>
#include "TLorentzVector.h"
#include <cmath>
#include <cassert>
#include <utility>
#include <chrono>
#include <boost/math/constants/constants.hpp>
#include "TH1D.h"
#include "TH2D.h"
#include "stl_help.hh"
#include "HistStore.hh"
#include "easy_app.hh"

//CONSTANTS
//Nature
constexpr double mK = 0.493;
const double mMu = 0.105;
const double pi = boost::math::constants::pi<double>();

//NA62
constexpr double p_nominal_kaon = 74.0;
const double e_nominal_kaon = std::hypot( p_nominal_kaon, mK );

const TLorentzVector p4_nominal_kaon = TLorentzVector
{ 0, 0, p_nominal_kaon, e_nominal_kaon};

//MATHS
TLorentzVector TBoost( const TLorentzVector& p4_in_rest_frame,
        double frame_rest_mass, const TLorentzVector& p4_of_rest_frame )
{
    //Stolen from compact fortran routine
    auto& p4A = p4_in_rest_frame;
    auto& p40 = p4_of_rest_frame;
    auto& m  = frame_rest_mass;

    auto ep = ( p4A.Vect() * p40.Vect() + p4A.E() * p40.E() ) / m;
    auto cons = ( p4A.E() + ep ) / ( m + p40.E() );

    return TLorentzVector( p4A.Vect() + cons * p40.Vect(), ep );
}

//DISTRIBUTIONS
template <class RealType = double>
class uniform_theta_phi_distribution
{
    public:
        uniform_theta_phi_distribution()
            : fUni{0, 1} {}

        template <class Generator>
            std::pair<RealType, RealType> operator()( Generator& g)
            {
                auto u = fUni( g );
                auto v = fUni( g );

                auto theta = std::acos(2 * v - 1);
                auto phi = 2 * boost::math::constants::pi<RealType>() * u;

                return std::make_pair( theta, phi );
            }
    private:
        std::uniform_real_distribution<RealType> fUni;
};

template <class RealType = double>
class mass2_distribution
{
    private:
        std::normal_distribution<double> kaon_mom_dist;
        uniform_theta_phi_distribution<double> muon_angle_dist;
        TLorentzVector p4_nominal_kaon;
        RealType nu_mass;

    public:
        mass2_distribution( RealType kaon_mom_mean,
                RealType kaon_mom_sigma, const TLorentzVector& nominal_kaon ,
                RealType neutrino_mass = 0 )
            :kaon_mom_dist{ kaon_mom_mean, kaon_mom_sigma}, 
            p4_nominal_kaon{ nominal_kaon }, nu_mass( neutrino_mass )
            {}

        template <class Generator>
            TLorentzVector getMuon( Generator& g )
            {
                //Produce kaon
                auto kaon_mom = kaon_mom_dist( g );
                auto kaon_energy = std::hypot( kaon_mom, mK );
                TLorentzVector p4_kaon( TVector3{ 0, 0, kaon_mom}, kaon_energy );


                //Produce muon
                RealType  muon_energy =
                    (pow( mK, 2) + pow ( mMu, 2) - pow(nu_mass, 2) ) / ( 2 * mK );

                RealType muon_mom_mag = 
                    std::sqrt( pow( muon_energy, 2) - pow( mMu, 2) );

                auto theta_phi = muon_angle_dist( g );

                TVector3 muon_mom{};
                muon_mom.SetMagThetaPhi ( muon_mom_mag , theta_phi.first, theta_phi.second );
                TLorentzVector p4_muon( muon_mom, muon_energy );

                //Boost muon
                auto boosted_muon = TBoost( p4_muon, mK, p4_kaon );
                return  boosted_muon;
            }

        template <class Generator>
            TLorentzVector operator()( Generator& g )
            {
                auto p4_boosted_muon = getMuon(g);
                return p4_boosted_muon;
            }
};

void run_hnu_masses( std::string output_file, Long64_t nEvents, std::default_random_engine& re )
{
    double mom_kaon = 74.0;

    auto tfOut = fn::make_unique<TFile>( output_file.c_str() , "RECREATE" );

    fn::HistStore hs;

    for ( int i = 0 ; i != 8 ; ++i )
    {
        double hNu = 0.05 * i;
        auto shNu = std::to_string(static_cast<int>(1000*hNu));

        std::cerr << "Simulating " << nEvents  << " events "
            << "for mhnu = " << hNu << std::endl;

        mass2_distribution<double> 
            m2_dist{ mom_kaon, 1.4 , p4_nominal_kaon, hNu };

        auto hM2Miss = hs.MakeTH1D( ("hM2M"+shNu).c_str(), 
                "Reconstructed missing mass", 
                1000, -0.7, 0.3, "Missing m^{2} / GeV^{2} ", "#Events" );

        auto nToDo = nEvents;

        while( nToDo -- )
        {
            auto p4_boosted_muon  = ( m2_dist( re ) ) ;
            hM2Miss->Fill( ( p4_nominal_kaon -  p4_boosted_muon).M2() );
        }
    }

    hs.Write();
}

void run_kaon_momenta( std::string output_file, Long64_t nEvents, std::default_random_engine& re )
{
    auto tgOut = fn::make_unique<TFile>( output_file.c_str(), "RECREATE" );
    tgOut->cd();

    fn::HistStore hs;

    for ( int kmom : { 20, 40, 50, 60, 80, 100, 120 } )
    {

        std::cerr << "Simulating " << nEvents << " events "
            << "for kmom = " << kmom << std::endl;

        mass2_distribution<double> m2_dist { static_cast<double>(kmom), 1.4, p4_nominal_kaon, 0 };

        auto hM2Miss = hs.MakeTH1D( ("hM2M"+std::to_string(kmom)).c_str(), 
                Form("Reconstructed missing mass : %d GeV", kmom), 
                1000, -0.7, 0.3, "Missing m^{2} / GeV^{2} ", "#Events" );

        auto hMomMass = hs.MakeTH2D( Form( "h_m2m_p_%d", kmom ),
                Form("p vs m^{2}_{miss} : %d GeV kaons" , kmom),
                1000, -0.7, 0.3, "m^{2}_{miss} (GeV^{2}/c^{4})",
                100, 0, 100, "p (GeV)" );

        auto h_m2m_t = hs.MakeTH2D( Form( "h_m2m_t_%d", kmom ),
                Form("t vs m^{2}_{miss} : %d GeV kaons" , kmom),
                1000, -0.7, 0.3, "m^{2}_{miss} (GeV^{2}/c^{4})",
                100, 0, 0.025, "t (rad)" );

        auto h_p_t = hs.MakeTH2D( Form( "h_p_t_%d", kmom ),
                Form("t vs P : %d GeV kaons" , kmom),
                100, 0, 100, "p (GeV)",
                100, 0, 0.025, "t (rad)" );

        auto nToDo = nEvents;

        while( nToDo -- )
        {
            auto p4_boosted_muon  = ( m2_dist( re ) ) ;
            auto m2m = ( p4_nominal_kaon -  p4_boosted_muon).M2() ;
            double mom_muon = p4_boosted_muon.Vect().Mag();
            double t = p4_boosted_muon.Vect().Angle( p4_nominal_kaon.Vect() );

            hM2Miss->Fill(m2m);
            hMomMass->Fill( m2m, mom_muon );
            h_m2m_t->Fill( m2m, t );
            h_p_t->Fill( mom_muon, t );
        }
    }
   hs.Write();
}

int main(int argc, char * argv[] )
{
    int nEvents = 10000;
    if ( argc == 2)
    { nEvents = atoi(argv[1]); }

    std::chrono::system_clock sc{};
    std::default_random_engine re {static_cast<unsigned long>( sc.now().time_since_epoch().count() )};

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "kaons,k", po::value<path>(),  "Run different kaon momenta")
        ( "massivenu,m", po::value<path>(),  "Run different neutrino masses")
        ( "number,n", po::value<Long64_t>(), "Number of events")
        ;

    po::options_description desc("Allowed options");
    desc.add( general );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    Long64_t nevt = 10000;

    if ( vm.count( "number" ) )
    {
        nevt = vm["number"].as<Long64_t>();
    }

    if ( vm.count( "kaons" ) )
    {
        run_kaon_momenta( vm["kaons"].as<path>().string(), nevt, re );
    }

    if ( vm.count( "massivenu" ) )
    {
        run_hnu_masses( vm["massivenu"].as<path>().string(), nevt, re );
    }
}
