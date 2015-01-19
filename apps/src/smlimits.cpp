#include "easy_app.hh"
#include "TF1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TRandom.h"
#include "TMath.h"
#include <boost/progress.hpp>

class gaus_par
{
    public:
        double f;
        double m;
        double s;
};

class tester
{
    public:
        TH1D hm2m;
        TH1D hm2m_gen1;
        TH1D hm2m_gen2;
        TH1D hm2m_cheat_fit;

        gaus_par gp1; 
        gaus_par gp2;

        TF1 gen_1;
        TF1 gen_2;

        TF1 fit_null;
        TF1 fit_double;
        double null_prob;
        double null_chi2;

        tester( double f1, double m1, double s1, double f2, double m2, double s2  )
            :
                hm2m( "hm2m", "Simulated missing mass distirbution", 10000, -10, 10 ),
                hm2m_gen1( "hm2m_gen1", "Simulated missing mass distirbution", 10000, -10, 10 ),
                hm2m_gen2( "hm2m_gen2", "Simulated missing mass distirbution", 10000, -10, 10 ),
                hm2m_cheat_fit( "hm2m_cheat_fit", "Simulated missing mass distirbution", 10000, -10, 10 ),
                gp1{ f1, m1, s1}, gp2{ f2, m2, s2 },
                gen_1( "g1", "gaus(0)", -10, 10 ),
                gen_2( "g2", "gaus(0)", -10, 10 )
                {
                    double factor = 1 / TMath::Sqrt( 2.0 * TMath::Pi() );

                    gen_1.SetParameters( factor / gp1.s, gp1.m, gp1.s );
                    gen_2.SetParameters( factor / gp2.s, gp2.m, gp2.s );

                    fit_double.SetParameters( factor / gp1.s * gp1.f, gp1.m, gp1.s ,
                            factor / gp2.s * gp2.f, gp2.m, gp2.s );

                    for ( int i = 1 ; i!= hm2m.GetNbinsX() + 1 ; ++i )
                    {
                        double xlow = hm2m.GetBinLowEdge( i );
                        double xwidth = hm2m.GetBinWidth( i );

                        double mean_1 = gp1.f * gen_1.Integral( xlow, xlow + xwidth ) ;
                        hm2m_gen1.SetBinContent( i, mean_1 );

                        double mean_2 = gp2.f * gen_2.Integral( xlow, xlow + xwidth ) ;
                        hm2m_gen2.SetBinContent( i, mean_2 );

                        hm2m_cheat_fit.SetBinContent( i, (gp1.f + gp2.f) / gp1.f * mean_1 );
                    }
                }

        void gen()
        {
            for ( int i = 1 ; i!= hm2m.GetNbinsX() + 1 ; ++i )
            {
                double mean = hm2m_gen1.GetBinContent( i ) + hm2m_gen2.GetBinContent(i );
                double error = TMath::Sqrt( mean );
                double hit = 0;

                if ( mean < 200 ) { hit = gRandom->Poisson( mean ) ; }
                else{ hit = gRandom->Gaus( mean, error); }

                hm2m.SetBinContent( i, hit );
            }
        }

        void do_cheat_null()
        {
            double min = gp1.m - 3*gp1.s;
            double max  = gp1.m + 3*gp1.s;

            double imin = hm2m.FindBin( min );
            double imax = hm2m.FindBin( max );

            double chi2 = 0;

            for ( int ibin = imin ; ibin != imax ; ++ibin )
            {
                chi2 += TMath::Power( hm2m.GetBinContent( ibin ) 
                        - hm2m_cheat_fit.GetBinContent( ibin ), 2)
                    / TMath::Power(hm2m.GetBinError( ibin ), 2 );

#if 0
                std::cerr << ibin << "  "
                    <<  hm2m.GetBinContent( ibin ) << " " 
                    << hm2m_cheat_fit.GetBinContent( ibin ) << " " 
                    << hm2m.GetBinError( ibin ) << std::endl;
#endif
            }

            null_chi2 = chi2;
            null_prob = TMath::Prob( chi2, imax + 1 - imin );
        }

        void do_fit_null( std::string opt = "" ) 
        {
            double min = gp1.m - 3*gp1.s;
            double max  = gp1.m + 3*gp1.s;

            fit_null = TF1( "fit_null", "gaus(0)", min, max);
            fit_null.SetParameters( gp1.f, gp1.m, gp1.s );
            hm2m.Fit( &fit_null, opt.c_str(), "" );

            null_prob = TMath::Prob( fit_null.GetChisquare(), fit_null.GetNDF() );
        }

        void do_fit_double( std::string opt = "" ) 
        {
            hm2m.Fit( &fit_double, opt.c_str(), "", gp1.m - 2* gp1.s, gp2.m + 2*gp2.s );
            fit_double = TF1( "fit_double", "gaus(0)+gaus(3)");
        }

        void write()
        {
            hm2m.Write();
            fit_null.Write();
            fit_double.Write();
        }
};

int main()
{
    std::cout << "SM NU MASS LIMITS" << std::endl;

    TFile tfout( "output/smnumass.root", "RECREATE" );

#if 0
    tester t( 1e9, 0, 1, 1e9, 3, 1);
    t.gen();
    t.do_cheat_null();

    t.hm2m.Write();
    t.hm2m_gen1.Write();
    t.hm2m_gen2.Write();
    t.hm2m_cheat_fit.Write();

    std::cout << t.null_chi2 <<  " " << t.null_prob << std::endl;

#endif
    int max_loops = 1000;

    TH1D hmean( "hmeanprob", "H0 Prob", 100,-0.05,0.95  );

#if 0
    for( const int& mu : {  1,2, 3, 4, 5, 6,7, 8,9, 10 } )
    {
        std::cout << "MU: " << TMath::Exp( 0.1* mu) << std::endl;
        tester t( 1e6, 0, 1, 1e6, 0.001 * mu, 1);
        TH1D h( "hprob", "H0 Prob", 1000,0, 1 );

        boost::progress_display show_progress( max_loops );

        for ( int i = 0 ; i !=max_loops ; ++i, ++show_progress )
        {
            t.gen();
            //t.do_fit_null("Q,0");
            t.do_cheat_null();
            h.Fill( t.null_prob );
        }

        h.Write(Form("hprob%d", mu));
    }
#endif

    std::vector<double> stats;

    for ( int expon = 6 ; expon != 15; ++expon )
    {
        for ( double mant : { 1.0, 1.77, 3.13 , 5.57 })
        {
            stats.push_back( mant * TMath::Power(10, expon ) );
        }
    }

    std::vector<double> mus;

    for ( int expon = -6 ; expon != -2; ++expon )
    {
        for ( double mant : { 1.0, 1.77, 3.13 , 5.57 })
        {
            mus.push_back( mant * TMath::Power(10, expon ) );
        }
    }

    std::ostream& os( std::cout);

    int swidth = 15;

    os << std::setw(swidth) << "mean";
    for ( const double& stat :  stats )
    {
        os  << std::setw(swidth) << stat ;
    }

    os << std::endl;

    auto prec = os.precision( 3 );

    for ( const double& mu : mus )
    {
        os << std::setw(swidth) << mu;

        for ( const double& stat :  stats )
        {

            tester t ( stat, 0, 1, stat, mu, 1 );

            double total_prob = 0;
            int count = 0;

            for ( int i = 0 ; i != max_loops ; ++i  )
            {
                t.gen();
                t.do_cheat_null();
                total_prob += t.null_prob;
                ++count;
            }

            os << std::setw(swidth) << total_prob  / float(count) << std::flush;

        }

        os << std::endl;
    }
}
