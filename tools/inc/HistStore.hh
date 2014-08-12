#ifndef HISTORE_HH
#define HISTORE_HH

#include <memory>
#include <string>
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TFile.h"

#include <boost/filesystem/path.hpp>

namespace fn
{
	//Helper functions to produce histograms
	std::unique_ptr<TH1D>
		makeTH1D(
				std::string name, std::string title,
				int nBins, double xmin, double xmax,
				std::string xtitle, std::string ytitle);

	std::unique_ptr<TH2D>
		makeTH2D(
				std::string name, std::string title,
				int nxBins, double xmin, double xmax,
				std::string xtitle,
				int nyBins, double ymin, double ymax,
				std::string ytitle
				);

	std::unique_ptr<TH3D>
		makeTH3D(
				std::string name, std::string title,
				int nxBins, double xmin, double xmax,
				std::string xtitle,
				int nyBins, double ymin, double ymax,
				std::string ytitle,
				int nzBins, double zmin, double zmax,
				std::string ztitle
				);

	//Class to store histograms
	class HistStore
	{
		public:
            typedef std::vector<std::unique_ptr<TH1>>
                ::iterator iterator;

            typedef std::vector<std::unique_ptr<TH1>>
                ::const_iterator const_iterator;

            iterator begin(){ return std::begin(fHists); }
            iterator end(){ return std::end(fHists); }

            const_iterator begin() const
            { return std::begin(fHists); }

            const_iterator end() const
            { return std::end(fHists); }

			HistStore() {}
			void AddHistogram(std::unique_ptr<TH1> h );
			void Write();

			TH1D * MakeTH1D(
					std::string name, std::string title,
					int nBins, double xmin, double xmax,
					std::string xtitle, std::string ytitle);

			TH2D * MakeTH2D(
					std::string name, std::string title,
					int nxBins, double xmin, double xmax,
					std::string xtitle,
					int nyBins, double ymin, double ymax,
					std::string ytitle
					);

		TH3D * MakeTH3D(
				std::string name, std::string title,
				int nxBins, double xmin, double xmax,
				std::string xtitle,
				int nyBins, double ymin, double ymax,
				std::string ytitle,
				int nzBins, double zmin, double zmax,
				std::string ztitle
				);

		private:
			std::vector<std::unique_ptr<TH1>> fHists;
	};

	void cd_p( TFile * f , boost::filesystem::path p );
}

#endif
