void k2pi()
{
    TFile * tfile = new TFile( "output/k2pi_neutral_vertex_k2pi_tree.root");
    TTree * k2pi = static_cast<TTree*>( tfile->Get("k2pi") );
    TFile tfout( "output/k2pi_plots_sep.root", "RECREATE");

    k2pi->Draw("data.p4pip.M2()>>hm2miss(1000, -0.1, 0.3)");
    TH1F * hm2miss = static_cast<TH1F*>( gDirectory->Get("hm2miss") );
    hm2miss->Write();

    k2pi->Draw("( data.neutral_vertex.Z()-mc.vertex.Z() )>>hdz(1000, -1000, 1000 )");
    TH1F * hdz = static_cast<TH1F*>( gDirectory->Get("hdz") );
    hdz->Write();

    k2pi->Draw("( data.neutral_vertex.Z() )>>hdataz(1000, -5000, 10000 )");
    TH1F * hdataz = static_cast<TH1F*>( gDirectory->Get("hdataz") );
    hdataz->Write();

    k2pi->Draw("( mc.vertex.Z() )>>hmcz(1000, -5000, 10000 )");
    TH1F * hmcz = static_cast<TH1F*>( gDirectory->Get("hmcz") );
    hmcz->Write();
}
