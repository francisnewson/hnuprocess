#ifndef MOMWEIGHT_HH
#define MOMWEIGHT_HH
#include "Selection.hh"
#include "Event.hh"
#include <boost/filesystem/path.hpp>

#if 0
/*
 *  _  __
 * | |/ /__ _  ___  _ __
 * | ' // _` |/ _ \| '_ \
 * | . \ (_| | (_) | | | |
 * |_|\_\__,_|\___/|_| |_|
 *  __  __              __        __   _       _     _
 * |  \/  | ___  _ __ __\ \      / /__(_) __ _| |__ | |_
 * | |\/| |/ _ \| '_ ` _ \ \ /\ / / _ \ |/ _` | '_ \| __|
 * | |  | | (_) | | | | | \ V  V /  __/ | (_| | | | | |_
 * |_|  |_|\___/|_| |_| |_|\_/\_/ \___|_|\__, |_| |_|\__|
 *                                       |___/
 */
#endif
namespace fn
{
    //inclusive range for ints
    typedef std::pair<int, int> irange;
    bool in_range( int val, irange range );
	std::istream& operator>>(std::istream& is, irange& ir);

    //exclusive range for doubles
    typedef std::pair<double, double> erange;
    bool in_range( double val, erange range );
	std::istream& operator>>(std::istream& is, erange& er);

    struct crude_weight_record
    {
        irange run_range;
        double slope;
    };

	std::istream& operator>>
        (std::istream& is, crude_weight_record& cwr);

    struct fine_weight_record
    {
        irange run_range;
        erange mom_range;

        double slope;
        double central_mom;
    };

	std::istream& operator>>
        (std::istream& is, fine_weight_record& fwr);

    class KaonMomWeight : public CachedSelection
    {
        public:
            KaonMomWeight( 
                    const fne::Event * e,
                    boost::filesystem::path crude_weights,
                    boost::filesystem::path fine_weights,
                    bool mc
                    );

        private:
            bool do_check() const { return true; }
            double get_weight() const; 

            const fne::Event * e_;
            bool mc_;

            std::vector<crude_weight_record> crude_weights_;
            std::vector<fine_weight_record> fine_weights_;

            REG_DEC_SUB( KaonMomWeight);
    };

    template<>
        Subscriber * create_subscriber<KaonMomWeight>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
