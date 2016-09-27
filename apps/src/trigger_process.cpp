#include "TriggerApp.hh"
#include <iostream>

int main( int argc, char * argv [] )
{
    auto keystring = std::string("");
    auto key = keystring.c_str();

    std::string trig_filename = "tdata/staging/full/all.shuffle_plots.root" ;
    TFile tfin_trig{ trig_filename.c_str() };

    fn::TriggerApp trig( tfin_trig );
    trig.set_num_denom( "htrig_num", "htrig_denom" );
    trig.set_sels( { 
            Form("pos/signal_upper_%splots/h_m2m_kmu", key ),
            Form("neg/signal_upper_%splots/h_m2m_kmu", key ),
            Form("pos/signal_lower_%splots/h_m2m_kmu", key ),
            Form("neg/signal_lower_%splots/h_m2m_kmu", key ),
            } );
    trig.init();


    auto eff_err = trig.get_eff_err( 0.06, 0.09 );
    std::cout << eff_err.first << " " << eff_err.second << std::endl;


}
