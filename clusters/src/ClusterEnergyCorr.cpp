#include "ClusterEnergyCorr.hh"
#include <stdexcept>
#include "GlobalStatus.hh"

namespace fn
{
    ClusterEnergyCorr::ClusterEnergyCorr( std::string filename )
    {
        //Declarations for lkr E/p correction
        CELLlength = 1.975;
        CPDlength = 8 * CELLlength;

        // E/p corrections for each cell
        int i, j, k;
        int l, m, n;
        int cpd, cell;

        // Define the positions for CPDs and Cells and store them 
        for (i=0; i<16; i++)
            for (j=0; j<16; j++)
            {
                k = i*16 + j;   
                // LKr RF is left-handed  --> the x sign has to be changed !!! 
                CPDpos_leftDownCorner[k][0] = (-1)*(7-i)*CPDlength; 
                CPDpos_leftDownCorner[k][1] = (7-j)*CPDlength;
                for (m=0; m<8; m++)
                    for (n=0; n<8; n++)
                    {
                        l = m*8 + n;
                        CELLpos_leftDownCorner[k][l][0] =
                            CPDpos_leftDownCorner[k][0] - (7-m)*CELLlength;

                        CELLpos_leftDownCorner[k][l][1] =
                            CPDpos_leftDownCorner[k][1] + (7-n)*CELLlength;
                    }
            }

        // Read E/p corrections for each cell from file
        FILE * EopCorrfile = fopen (filename.c_str(), "r");

        for (i=0; i<256; i++)
            for (j=0; j<64; j++)
            {    
                fscanf (EopCorrfile, "%i %i %f\n",
                        &cpd, &cell, &EopCorr[i][j]);        
            }
    }

    //--------------------------------------------------

    int ClusterEnergyCorr::GetCpdCellIndex(double pos_x, double pos_y,
            int *cpd_index, int *cell_index) const
    {
        int i, j, k;
        int l, m, n;

        // protection
        *cpd_index = -1; 
        *cell_index = -1; 

        // Define CPD index
        for (i=0; i<16; i++) {
            for (j=0; j<16; j++) {
                k = i*16 + j;
                if (pos_x <= CPDpos_leftDownCorner[k][0] &&
                        pos_x > (CPDpos_leftDownCorner[k][0]
                            -CPDlength) &&
                        pos_y >= CPDpos_leftDownCorner[k][1] &&
                        pos_y < (CPDpos_leftDownCorner[k][1]+
                            CPDlength)) {
                    *cpd_index = k;
                    break;
                }   
            }   
        }

        if (cpd_index<0) return 0;

        // Define cell index
        for (m=0; m<8; m++) {
            for (n=0; n<8; n++) {
                l = m*8 + n;
                if (pos_x <= CELLpos_leftDownCorner[*cpd_index][l][0] &&
                        pos_x > (CELLpos_leftDownCorner[*cpd_index][l][0]-CELLlength) &&
                        pos_y >= CELLpos_leftDownCorner[*cpd_index][l][1] &&
                        pos_y < (CELLpos_leftDownCorner[*cpd_index][l][1]+CELLlength)) {
                    *cell_index = l;
                }   
            }   
        }
        return 0;
    }

    //--------------------------------------------------

    double ClusterEnergyCorr::operator()(const fne::RecoCluster& rc, bool is_mc) const
    {
        return correct_energy(rc, is_mc);
    }

    double ClusterEnergyCorr::correct_energy(const fne::RecoCluster& rc, bool is_mc ) const
    {

        if ( is_mc )
        { return rc.energy; }

        Long64_t run = global_status().get_run();
        double corr_energy = user_lkrcalcor_SC( rc.energy, run, 1);

        //ints to receive cell information
        int cpd;
        int cell;

        //extracted raw positions
        double pos_x = rc.x;
        double pos_y = rc.y;

        //get cell
        bool cellCheck =  GetCpdCellIndex( pos_x , pos_y, &cpd, &cell );
        if ( cellCheck)
        {
            throw std::runtime_error( 
                    std::string( "Couldn't find CpdCellIndex ") + __FILE__ );
        }

        //get correction
        double correction =  EopCorr[cpd][cell];

        corr_energy /= correction;

        //apply correction
        return corr_energy;
    }

    double correct_eop_energy( const fne::RecoCluster& re, bool is_mc )
    {
        static ClusterEnergyCorr
            cec { "/afs/cern.ch/user/f/fnewson/work/hnu"
                "/gopher/data/detector/eopCorrfile.dat" };

        return  cec( re, is_mc );
    }

    std::pair<int, int> get_cpd_cell_index( double pos_x, double pos_y)
    {
        static ClusterEnergyCorr
            cec { "/afs/cern.ch/user/f/fnewson/work/hnu"
                "/gopher/data/detector/eopCorrfile.dat" };

        int cpd;
        int cell;
        cec.GetCpdCellIndex( pos_x, pos_y, &cpd, &cell );
        return std::make_pair( cpd, cell );
    }

    double user_lkrcalcor_SC(double energy, Long64_t run , int iflag) {
        double ecorr;
        /*
         * energy scale
         */
        ecorr = 0.;
        if(run>=13534 && run<13662) ecorr =  0.00062;
        if(run>=13662 && run<13676) ecorr = -0.00038;
        if(run>=13676 && run<13704) ecorr =  0.00076;
        if(run>=13704 && run<13766) ecorr =  0.00019;
        if(run>=13766 && run<13996) ecorr =  0.00071;
        if(run>=13996 && run<14063) ecorr =  0.00090;
        if(run>=14063 && run<14100) ecorr =  0.00088;
        if(run>=14100 && run<14200) ecorr =  0.00080;

        double result = energy *( 1.+ecorr );

        /*
         * DC threshold correction
         */
            double x= energy;
            if(x <11.) {
                ecorr=-0.00101009+0.000219228*x-1.61845E-05*x*x+4.17065E-07*x*x*x;
                if(iflag==2) {
                    ecorr=-0.00205335+0.000547472*x-4.8985E-05*x*x+1.4965E-06*x*x*x;
                }
                result *= (1.-19.*ecorr);
            }
        return result;
    }


}
