#include "HistStore.hh"
#include "TFile.h"

namespace fn
{

	using std::string;
	using std::unique_ptr;

	std::unique_ptr<TH1D> makeTH1D(
			string name, string title,
			int nBins, double xmin, double xmax,
			string xtitle, string ytitle
			)
	{
		unique_ptr<TH1D> h{ new TH1D( name.c_str(), title.c_str(),
				nBins, xmin, xmax) };

		h->SetDirectory(0);
		h->GetXaxis()->SetTitle( xtitle.c_str() );
		h->GetYaxis()->SetTitle( ytitle.c_str() );
		return h;
	}

	unique_ptr<TH2D> makeTH2D(
			std::string name, std::string title,
			int nxBins, double xmin, double xmax,
			std::string xtitle,
			int nyBins, double ymin, double ymax,
			std::string ytitle
			)
	{
		unique_ptr<TH2D>h { new TH2D( name.c_str(), title.c_str(),
				nxBins, xmin, xmax,
				nyBins, ymin, ymax) };

		h->SetDirectory(0);
		h->GetXaxis()->SetTitle( xtitle.c_str() );
		h->GetYaxis()->SetTitle( ytitle.c_str() );

		return h;
	}

	std::unique_ptr<TH3D>
		makeTH3D(
				std::string name, std::string title,
				int nxBins, double xmin, double xmax,
				std::string xtitle,
				int nyBins, double ymin, double ymax,
				std::string ytitle,
				int nzBins, double zmin, double zmax,
				std::string ztitle
				)
		{
			unique_ptr<TH3D> h {
				new TH3D( name.c_str(), title.c_str(),
						nxBins, xmin, xmax,
						nyBins, ymin, ymax,
						nzBins, zmin, zmax)
			};

			h->SetDirectory(0);
			h->GetXaxis()->SetTitle( xtitle.c_str() );
			h->GetYaxis()->SetTitle( ytitle.c_str() );
			h->GetZaxis()->SetTitle( ztitle.c_str() );

			return h;
		}

	void cd_p( TFile * f , boost::filesystem::path p )
	{
		f->cd();

		TDirectory * dir = static_cast<TDirectory*>
			(f->Get(p.string().c_str()) );

		if ( !dir )
		{ f->mkdir(p.string().c_str() ); }

		f->cd( p.string().c_str() );
	}

	void HistStore::AddHistogram( unique_ptr<TH1> h )
	{
		fHists.push_back(std::move(h));
	}

	void HistStore::Write()
	{
		for ( auto& h : fHists )
		{ h->Write(); }
	}

	TH1D * HistStore::MakeTH1D(
			std::string name, std::string title,
			int nBins, double xmin, double xmax,
			std::string xtitle, std::string ytitle)
	{
		auto h = makeTH1D( name, title, nBins, 
				xmin, xmax, xtitle, ytitle);

		TH1D * hp = h.get();
		AddHistogram( std::move(h) );
		return hp;
	}

	TH2D * HistStore::MakeTH2D(
			std::string name, std::string title,
			int nxBins, double xmin, double xmax,
			std::string xtitle,
			int nyBins, double ymin, double ymax,
			std::string ytitle)
	{
		auto h = makeTH2D( name, title, 
				nxBins, xmin, xmax, xtitle,
				nyBins, ymin, ymax, ytitle);

		TH2D * hp = h.get();
		AddHistogram( std::move(h) );
		return hp;
	}

	TH3D * HistStore::MakeTH3D(
			std::string name, std::string title,
			int nxBins, double xmin, double xmax,
			std::string xtitle,
			int nyBins, double ymin, double ymax,
			std::string ytitle,
			int nzBins, double zmin, double zmax,
			std::string ztitle
			)
	{
		auto h = makeTH3D( name, title, 
				nxBins, xmin, xmax, xtitle,
				nyBins, ymin, ymax, ytitle,
				nzBins, zmin, zmax, ztitle );

		TH3D * hp = h.get();
		AddHistogram( std::move(h) );
		return hp;
	}
}

