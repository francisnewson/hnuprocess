#ifndef K2PI_RECO_FUNCTIONS_HH
#define K2PI_RECO_FUNCTIONS_HH
#if 0
/*
 *  _    ____        _                       
 * | | _|___ \ _ __ (_)  _ __ ___  ___ ___   
 * | |/ / __) | '_ \| | | '__/ _ \/ __/ _ \  
 * |   < / __/| |_) | | | | |  __/ (_| (_) | 
 * |_|\_\_____| .__/|_| |_|  \___|\___\___/  
 *            |_|
 *  __                  _   _
 * / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 *| |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 *|  _| |_| | | | | (__| |_| | (_) | | | \__ \
 *|_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *
 *
 */
#endif
namespace fn
{

    class K2piLkrData;
    class K2piDchData;
    class K2piEventData;

    double extract_eop( K2piEventData& event, K2piDchData& dch );
    double extract_photon_sep ( K2piLkrData &raw_lkr );
    double extract_min_track_cluster_sep ( K2piLkrData &raw_lkr, K2piDchData& dch );

    double extract_min_photon_radius( K2piLkrData& raw_lkr );
}
#endif
