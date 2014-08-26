#include "TMath.h"
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TH1F.h"
#include "TSystem.h"
#include <iostream>

void k2pi()
{
    gSystem->AddLinkedLibs( "-L /afs/cern.ch/user/f/fnewson/"
            "work/hnu/gopher/code/process/build03/k2pi"
            " -lk2pivar");

    double mpi = 0.139570;
    double m2pi = std::pow( mpi, 2 );

    TFile * tfile = new TFile( "output/k2pi_neutral_vertex_k2pi_tree.root");
    TTree * k2pi = static_cast<TTree*>( tfile->Get("k2pi") );
    TFile tfout( "output/k2pi_plots_sep.root", "RECREATE");

    std::cerr << "Missing mass ... " << std::endl;
    k2pi->Draw("data.p4pip.M2()>>hm2miss(1000, -0.3, 0.2)");
    TH1F * hm2miss = static_cast<TH1F*>( gDirectory->Get("hm2miss") );
    hm2miss->Write();

    std::cerr << "Neutral vertex ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z()>>hdz_neutral(1000, -1000, 1000 )");
    TH1F * hdz_neutral = static_cast<TH1F*>( gDirectory->Get("hdz_neutral") );
    hdz_neutral->Write();

    std::cerr << "Neutral vertex (cut) ... " << std::endl;
    TCut low_mass = "data.p4pip.M2() > 0.019479785 - 0.002";
    TCut high_mass = "data.p4pip.M2() > 0.019479785 + 0.002";
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z()>>hdz_neutral_cut(1000, -1000, 1000 )",
            low_mass && high_mass);
    TH1F * hdz_neutral_cut = static_cast<TH1F*>( gDirectory->Get("hdz_neutral_cut") );
    hdz_neutral_cut->Write();

    std::cerr << "neutral vertex (2d) ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z():mc.vertex.Z()>>hdzvsz_neutral(100, -5000, 10000, 100, -1000, 1000 )");
    TH1F * hdzvsz_neutral = static_cast<TH1F*>( gDirectory->Get("hdzvsz_neutral") );
    hdzvsz_neutral->Write();

    std::cerr << "neutral vertex (E1) ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z():mc.p4g1.E()>>hdzvsE1_neutral(100, 0, 100, 100, -1000, 1000 )");
    TH1F * hdzvsE1_neutral = static_cast<TH1F*>( gDirectory->Get("hdzvsE1_neutral") );
    hdzvsE1_neutral->Write();

    std::cerr << "neutral vertex (E2) ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z():mc.p4g2.E()>>hdzvsE2_neutral(100, 0, 100, 100, -1000, 1000 )");
    TH1F * hdzvsE2_neutral = static_cast<TH1F*>( gDirectory->Get("hdzvsE2_neutral") );
    hdzvsE2_neutral->Write();

    std::cerr << "neutral vertex (C1) ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z():data.E1>>hdzvsC1_neutral(100, 0, 100, 100, -1000, 1000 )");
    TH1F * hdzvsC1_neutral = static_cast<TH1F*>( gDirectory->Get("hdzvsC1_neutral") );
    hdzvsC1_neutral->Write();

    std::cerr << "neutral vertex (c2) ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z():data.E2>>hdzvsC2_neutral(100, 0, 100, 100, -1000, 1000 )");
    TH1F * hdzvsC2_neutral = static_cast<TH1F*>( gDirectory->Get("hdzvsC2_neutral") );
    hdzvsC2_neutral->Write();

    std::cerr << "cluster 1 ... " << std::endl;
    k2pi->Draw("data.E1 - mc.p4g1.E()>>hdE1( 1000, -50, 50)");
    TH1F * hdE1 = static_cast<TH1F*>( gDirectory->Get("hdE1") );
    hdE1->Write();

    std::cerr << "cluster 2 ... " << std::endl;
    k2pi->Draw("data.E2 - mc.p4g2.E()>>hdE2( 1000, -50, 50)");
    TH1F * hdE2 = static_cast<TH1F*>( gDirectory->Get("hdE2") );
    hdE2->Write();

    std::cerr << "Cluster 1 and neutral dz... " << std::endl;
    k2pi->Draw("data.E1 - mc.p4g1.E():data.neutral_vertex.Z()-mc.vertex.Z()"
            ">>hdE1vsdz( 100, -1000, 1000,  100, -50, 50)");
    TH1F * hdE1vsdz = static_cast<TH1F*>( gDirectory->Get("hdE1vsdz") );
    hdE1vsdz->Write();

    std::cerr << "Cluster 2 and neutral dz... " << std::endl;
    k2pi->Draw("data.E2 - mc.p4g2.E():data.neutral_vertex.Z()-mc.vertex.Z()"
            ">>hdE2vsdz( 100, -1000, 1000,  100, -50, 50)");
    TH1F * hdE2vsdz = static_cast<TH1F*>( gDirectory->Get("hdE2vsdz") );
    hdE2vsdz->Write();

    std::cerr << "Charged vertex ... " << std::endl;
    k2pi->Draw("data.charged_vertex.Z()-mc.vertex.Z()>>hdz_charged(1000, -1000, 1000 )");
    TH1F * hdz_charged = static_cast<TH1F*>( gDirectory->Get("hdz_charged") );
    hdz_charged->Write();

    std::cerr << "Transverse momentum ... " << std::endl;
    k2pi->Draw("data.dch_pt>>hdch_pt(1000, -0.1, 0.9 )");
    TH1F * hdch_pt = static_cast<TH1F*>( gDirectory->Get("hdch_pt") );
    hdch_pt->Write();

    std::cerr << "Transverse momentum squared... " << std::endl;
    k2pi->Draw("data.dch_pt*data.dch_pt>>hdch_pt2(1000, -0.1, 0.9 )");
    TH1F * hdch_pt2 = static_cast<TH1F*>( gDirectory->Get("hdch_pt2") );
    hdch_pt2->Write();

    std::cerr << "Vertex and low energy clusters ... " << std::endl;
    k2pi->Draw("data.neutral_vertex.Z()-mc.vertex.Z()>>hdz_dEcut( 100, -1000, 1000 )", 
            "( data.E1 + data.E2 - mc.p4g1.E() - mc.p4g2.E() ) <0" );
    TH1F * hdz_dEcut = static_cast<TH1F*>( gDirectory->Get("hdz_dEcut") );
    hdz_dEcut->Write();
}
