namespace fn
{
    struct muv_burst
    {
        Long64_t run;
        Long64_t burst_time;

        Double_t total;
        Double_t pass;
    };
}

void bursts()
{
    TFile tf("/home/fon/work/hnu/data/thesis/muv_bursts/all.muv_bursts.root");
    TTree * tt = 0;
    tf.GetObject( "p5.data.q11t.v3.neg/muv_bursts/T", tt );

    int nE =  tt->GetEntries();
    std::cout << "Entries: " << nE << "\n";

    fn::muv_burst mb;
    std::cout << tt->SetBranchAddress( "muv_burst", &mb) << std::endl;
    tt->Print();

    std::ofstream ofs( "output/bursts.dat" );

    for ( Long64_t ient = 0 ; ient != 10 ; ++ient )
    {
        tt->GetEntry( ient );
        std::cout << std::setw(10) << mb.run << std::setw(15) << mb.burst_time << "\n";
    }
}
