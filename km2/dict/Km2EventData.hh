#ifndef KM2EVENTDATA_HH
#define KM2EVENTDATA_HH
#include "TVector3.h"
#if 0
/*
 *  _  __          ____  _____                 _   ____        _
 * | |/ /_ __ ___ |___ \| ____|_   _____ _ __ | |_|  _ \  __ _| |_ __ _
 * | ' /| '_ ` _ \  __) |  _| \ \ / / _ \ '_ \| __| | | |/ _` | __/ _` |
 * | . \| | | | | |/ __/| |___ \ V /  __/ | | | |_| |_| | (_| | || (_| |
 * |_|\_\_| |_| |_|_____|_____| \_/ \___|_| |_|\__|____/ \__,_|\__\__,_|
 *
 *
 */
#endif
namespace fn
{
    class Km2EventData : public TObject
    {
        public:
            TVector3  vertex;
            TVector3  mu_mom;
            TVector3  kpoint;
            TVector3  k_mom;

            double p;
            double z;
            double t;
            double cda;
            double m2m;

            double event_wgt;
            double muv_wgt;

            double tx;
            double ty;

            double xcoll;
            double ycoll;

            double xdch4;
            double ydch4;

            virtual ~Km2EventData(){}
            ClassDef ( Km2EventData, 1 );
    };
}
#endif
