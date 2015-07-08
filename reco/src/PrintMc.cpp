#include "PrintMc.hh"
#include "Mc.hh"
#include "Event.hh"
#include "yaml_help.hh"
#include <iomanip>
#include <cassert>
#include "RecoFactory.hh"

namespace fn
{

    REG_DEF_SUB( PrintMc );

    PrintMc::PrintMc( const Selection& sel, const fne::Event* e ,
            std::ostream& os )
        :Analysis( sel ), e_ ( e ), os_ ( os )
    {
        os_ << "PrintMC constructor!" << std::endl;
    }

    void PrintMc::process_event()
    {
        const fne::Mc & mc = e_->mc;

        auto& particles = mc.particles;
        Int_t npart = mc.npart;

        os_ << "kaon: " << std::setw(10) << mc.decay.kaon_type 
            << " " << mc.decay.decay_type << "\n";

        for ( Int_t ipart = 0 ; ipart != npart ; ++ipart )
        {
            fne::McParticle * part = static_cast<fne::McParticle*>( particles.At(ipart ) );
            os_ << "[" << ipart << "]"  << std::setw(10) << part->type << "\n";
        }

        os_ << "--------------------------------------------------\n";
    }

    template<>
        Subscriber * create_subscriber<PrintMc>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            std::cerr << "Create PrintMC" << std::endl;

            const fne::Event * e = rf.get_event_ptr();
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            std::ostream& os = rf.get_ostream( 
                    get_yaml<std::string>( instruct, "ostream" ));

            return new PrintMc( *sel, e, os );
        }
}

