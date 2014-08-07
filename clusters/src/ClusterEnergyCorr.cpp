#include "ClusterEnergyCorr.hh"
#include <stdexcept>

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

    double ClusterEnergyCorr::operator()(fne::RecoCluster& rc) const
    {
        return correct_energy(rc);
    }

    double ClusterEnergyCorr::correct_energy(fne::RecoCluster& rc) const
    {
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

        //apply correction
        return rc.energy / correction;
    }

}
