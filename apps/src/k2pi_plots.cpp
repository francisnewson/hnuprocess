#include "TMath.h"
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TH1F.h"
#include "TSystem.h"
#include "K2piVars.hh"
#include "Track.hh"
#include <iostream>
#include "HistStore.hh"
#include "Counter.hh"
#include "logging.hh"
#include "K2piPlots.hh"

using namespace fn;

int main( int argc, char * argv[] )
{
    //Open tree
    TFile * tfile = new TFile( "output/k2pi_neutral_vertex_k2pi_tree.root");
    TTree * k2pi = 0;
    tfile->GetObject("k2pi", k2pi);

    k2pi->SetCacheSize( 10000000 );
    k2pi->AddBranchToCache("*");

    double nEntries = k2pi->GetEntriesFast();

    //Connect event
    const fn::K2piVars * vars = new fn::K2piVars;
    k2pi->SetBranchAddress( "k2piVars", &vars );

    TFile tfout( "output/k2pi_plots_sep_extra.root", "RECREATE");
    fn::logger slg;
    fn::Counter counter( slg, nEntries);

    K2piPlots k2pi_plots( vars, tfout );

    for ( Int_t i = 0 ; i < nEntries ; ++i )
    {
        k2pi->GetEntry( i );
        counter.new_event();
        k2pi_plots.new_event();
    }

    tfout.cd();
    k2pi_plots.end_processing();
}
