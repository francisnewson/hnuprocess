#ifndef KM2CLUSTERS_HH
#define KM2CLUSTERS_HH
#include <vector>
#include "Event.hh"
#include "Subscriber.hh"
#include "CorrCluster.hh"
#include "SingleTrack.hh"
#if 0
/*
 *  _  __          ____  
 * | |/ /_ __ ___ |___ \ 
 * | ' /| '_ ` _ \  __) |
 * | . \| | | | | |/ __/ 
 * |_|\_\_| |_| |_|_____|
 *   ____ _           _           
 *  / ___| |_   _ ___| |_ ___ _ __
 * | |   | | | | / __| __/ _ \ '__\
 * | |___| | |_| \__ \ ||  __/ |  
 *  \____|_|\__,_|___/\__\___|_|  
 *
 *                       
 * 
 */
#endif
namespace fn
{
    enum class cluster_type{ IGN, ASS, BAD};

    class Km2RecoClusters
    {
        private:
            std::vector<const fne::RecoCluster *> bad_clusters_;
            std::vector<const fne::RecoCluster *> associate_clusters_;
            std::vector<const fne::RecoCluster *> ignored_clusters_;
            std::vector<const fne::RecoCluster *> all_clusters_;
            bool mc_;
            const ClusterCorrector& cluster_corrector_;

        public:
            Km2RecoClusters( const ClusterCorrector& cluster_corrector_);
            void reset( bool mc);
            void add_cluster( cluster_type ct, const fne::RecoCluster * cd );

            bool is_mc() const { return mc_;}
            const ClusterCorrector& get_cluster_corrector() const { return  cluster_corrector_;}

            typedef std::vector<const fne::RecoCluster *>::size_type size_type;
            typedef std::vector<const fne::RecoCluster *>::iterator iterator;
            typedef std::vector<const fne::RecoCluster *>::const_iterator const_iterator;

            size_type bad_size() const 
            { return bad_clusters_.size() ;}

            size_type associate_size() const
            { return associate_clusters_.size() ;}

            size_type ignored_size() const 
            { return ignored_clusters_.size(); }

            size_type all_size() const 
            { return all_clusters_.size(); }

            const_iterator bad_begin() const
            { return bad_clusters_.begin() ; }

            const_iterator associate_begin() const 
            { return associate_clusters_.begin() ; }

            const_iterator ignored_begin() const 
            { return ignored_clusters_.begin() ; }

            const_iterator all_begin() const 
            { return all_clusters_.begin() ; }

            const_iterator bad_end() const 
            { return bad_clusters_.end() ; }

            const_iterator associate_end() const 
            { return associate_clusters_.end() ; }

            const_iterator ignored_end() const 
            { return ignored_clusters_.end() ; }

            const_iterator all_end() const 
            { return all_clusters_.end() ; }
    };

    class Km2Clusters : public Subscriber
    {
        public:
            Km2Clusters( const fne::Event* e, const SingleTrack& st ,
                    double noise_energy, double noise_time, 
                    double brehm_radius, double track_cluster_radius, 
                    bool do_extra_lkr_check,
                    const ClusterCorrector& cluster_corrector, bool is_mc );

            void new_event();
            const Km2RecoClusters& get_reco_clusters() const;
            virtual cluster_type id_cluster( const fne::RecoCluster* rc ) const;

            bool is_mc(){ return mc_;}

        private:
            virtual void process_clusters() const;
            const fne::Event * e_;
            const SingleTrack& st_;
            mutable Km2RecoClusters km2rc_;


            double noise_energy_;
            double noise_time_;
            double brehm_radius_;
            double track_cluster_radius_;

            double margin_parameter_;

            bool do_extra_lkr_check_;


            const ClusterCorrector& cluster_corrector_;
            bool mc_;

            mutable bool dirty_;

            REG_DEC_SUB( Km2Clusters );
    };

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<Km2Clusters>
        (YAML::Node& instruct, RecoFactory& rf );

    Km2Clusters * get_km2_clusters
        ( YAML::Node& instruct, RecoFactory& rf );
}
#endif
