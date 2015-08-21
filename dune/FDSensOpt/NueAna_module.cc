////////////////////////////////////////////////////////////////////////
// Class:       NueAna
// Module Type: analyzer
// File:        NueAna_module.cc
//
// dorota.stefan@cern.ch, robert.sulej@cern.ch, tjyang@fnal.gov
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h" 
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// LArSoft includes
#include "Geometry/Geometry.h"
#include "RecoBase/Track.h"
#include "RecoBase/Hit.h"
#include "RecoBase/Cluster.h"
#include "RecoBase/Vertex.h"
#include "Utilities/LArProperties.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/AssociationUtil.h"
#include "Utilities/TimeService.h"
#include "MCCheater/BackTracker.h"
#include "SimulationBase/MCTruth.h"

// ROOT includes
#include "TTree.h"
#include "TTimeStamp.h"

// C/C++ libraries
#include <memory>
#include <utility>

constexpr int kMaxTrack      = 1000;  //maximum number of tracks
constexpr int kMaxHits       = 25000; //maximum number of hits;
constexpr int kMaxVertices   = 100;    //max number of 3D vertices

namespace dunefd {
	class Hit2D;
  class NueAna;
	class IniSegAlg;
}

class dunefd::NueAna : public art::EDAnalyzer {
public:
  explicit NueAna(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  NueAna(NueAna const &) = delete;
  NueAna(NueAna &&) = delete;
  NueAna & operator = (NueAna const &) = delete;
  NueAna & operator = (NueAna &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

  void reconfigure(fhicl::ParameterSet const& p) override;

private:

  void ResetVars();
  bool insideFidVol(const TLorentzVector& pvtx); 

  // Declare member data here.
  TTree *fTree;
  
  // Run information
  int run;
  int subrun;
  int event;
  float evttime;
  float taulife;
  short isdata;

  int ntracks_reco;         //number of reconstructed tracks
  int   trkid[kMaxTrack];
  float trkstartx[kMaxTrack];
  float trkstarty[kMaxTrack];
  float trkstartz[kMaxTrack];
  float trkendx[kMaxTrack];
  float trkendy[kMaxTrack];
  float trkendz[kMaxTrack];
  float trkstartdcosx[kMaxTrack];
  float trkstartdcosy[kMaxTrack];
  float trkstartdcosz[kMaxTrack];
  float trkenddcosx[kMaxTrack];
  float trkenddcosy[kMaxTrack];
  float trkenddcosz[kMaxTrack];
  float trklen[kMaxTrack];

  int nhits;
  Short_t  hit_plane[kMaxHits];      //plane number
  Short_t  hit_wire[kMaxHits];       //wire number
  Int_t  hit_channel[kMaxHits];    //channel ID
  Float_t  hit_peakT[kMaxHits];      //peak time
  Float_t  hit_charge[kMaxHits];     //charge (area)
  Float_t  hit_startT[kMaxHits];     //hit start time
  Float_t  hit_endT[kMaxHits];       //hit end time
  
  // vertex information
  Short_t  nvtx;                     //number of vertices
  Float_t  vtx[kMaxVertices][3];     //vtx[3] 
  
  //mctruth information
  Int_t     mcevts_truth;    //number of neutrino Int_teractions in the spill
  Int_t     nuPDG_truth;     //neutrino PDG code
  Int_t     ccnc_truth;      //0=CC 1=NC
  Int_t     mode_truth;      //0=QE/El, 1=RES, 2=DIS, 3=Coherent production
  Float_t  enu_truth;       //true neutrino energy
  Float_t  Q2_truth;        //Momentum transfer squared
  Float_t  W_truth;         //hadronic invariant mass
  Float_t  X_truth;
  Float_t  Y_truth;
  Int_t     hitnuc_truth;    //hit nucleon
  Float_t  nuvtxx_truth;    //neutrino vertex x
  Float_t  nuvtxy_truth;    //neutrino vertex y
  Float_t  nuvtxz_truth;    //neutrino vertex z
  Float_t  nu_dcosx_truth;  //neutrino dcos x
  Float_t  nu_dcosy_truth;  //neutrino dcos y
  Float_t  nu_dcosz_truth;  //neutrino dcos z
  Float_t  lep_mom_truth;   //lepton momentum
  Float_t  lep_dcosx_truth; //lepton dcos x
  Float_t  lep_dcosy_truth; //lepton dcos y
  Float_t  lep_dcosz_truth; //lepton dcos z
  

  std::string fHitsModuleLabel;
	std::string fClusterModuleLabel;
  std::string fTrackModuleLabel;
  std::string fVertexModuleLabel;
  std::string fGenieGenModuleLabel;

  double fFidVolCut;
};


dunefd::NueAna::NueAna(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset)
{
  reconfigure(pset);
}

void dunefd::NueAna::analyze(art::Event const & evt)
{

	std::cout << " **************************************** analyze ************ " << std::endl;
  // Implementation of required member function here.
  ResetVars();
  art::ServiceHandle<geo::Geometry> geom;
  art::ServiceHandle<util::LArProperties> larprop;
  art::ServiceHandle<util::DetectorProperties> detprop;
  art::ServiceHandle<util::TimeService> timeservice;
  //fClock = timeservice->TPCClock();
  art::ServiceHandle<cheat::BackTracker> bktrk;

  run = evt.run();
  subrun = evt.subRun();
  event = evt.id().event();
  art::Timestamp ts = evt.time();
  TTimeStamp tts(ts.timeHigh(), ts.timeLow());
  evttime = tts.AsDouble();
  taulife = larprop->ElectronLifetime();
  isdata = evt.isRealData();

  // * hits
  art::Handle< std::vector<recob::Hit> > hitListHandle;
  std::vector<art::Ptr<recob::Hit> > hitlist;
  if (evt.getByLabel(fHitsModuleLabel,hitListHandle))
    art::fill_ptr_vector(hitlist, hitListHandle);

  // * tracks
  art::Handle< std::vector<recob::Track> > trackListHandle;
  std::vector<art::Ptr<recob::Track> > tracklist;
  if (evt.getByLabel(fTrackModuleLabel,trackListHandle))
    art::fill_ptr_vector(tracklist, trackListHandle);

  // * vertices
  art::Handle< std::vector<recob::Vertex> > vtxListHandle;
  std::vector<art::Ptr<recob::Vertex> > vtxlist;
  if (evt.getByLabel(fVertexModuleLabel,vtxListHandle))
    art::fill_ptr_vector(vtxlist, vtxListHandle);

  //hit information
  nhits = hitlist.size();
  for (int i = 0; i < nhits && i < kMaxHits ; ++i){//loop over hits
    hit_channel[i] = hitlist[i]->Channel();
    hit_plane[i]   = hitlist[i]->WireID().Plane;
    hit_wire[i]    = hitlist[i]->WireID().Wire;
    hit_peakT[i]   = hitlist[i]->PeakTime();
    hit_charge[i]  = hitlist[i]->Integral();
    hit_startT[i] = hitlist[i]->PeakTimeMinusRMS();
    hit_endT[i] = hitlist[i]->PeakTimePlusRMS();
  }

  //track information
  ntracks_reco=tracklist.size();

  double larStart[3];
  double larEnd[3];
  std::vector<double> trackStart;
  std::vector<double> trackEnd;
  for(int i=0; i<std::min(int(tracklist.size()),kMaxTrack);++i){
    trackStart.clear();
    trackEnd.clear();
    memset(larStart, 0, 3);
    memset(larEnd, 0, 3);
    tracklist[i]->Extent(trackStart,trackEnd); 
    tracklist[i]->Direction(larStart,larEnd);
    trkid[i]       = tracklist[i]->ID();
    trkstartx[i]      = trackStart[0];
    trkstarty[i]      = trackStart[1];
    trkstartz[i]      = trackStart[2];
    trkendx[i]        = trackEnd[0];
    trkendy[i]        = trackEnd[1];
    trkendz[i]        = trackEnd[2];
    trkstartdcosx[i]  = larStart[0];
    trkstartdcosy[i]  = larStart[1];
    trkstartdcosz[i]  = larStart[2];
    trkenddcosx[i]    = larEnd[0];
    trkenddcosy[i]    = larEnd[1];
    trkenddcosz[i]    = larEnd[2];
    trklen[i]         = tracklist[i]->Length();
  }

  //vertex information
  nvtx = vtxlist.size();
  for (int i = 0; i < nvtx && i < kMaxVertices ; ++i){//loop over hits
    Double_t xyz[3] = {};
    vtxlist[i]->XYZ(xyz);
    for (size_t j = 0; j<3; ++j) vtx[i][j] = xyz[j];
  }

  if (!isdata){

    // * MC truth information
    art::Handle< std::vector<simb::MCTruth> > mctruthListHandle;
    std::vector<art::Ptr<simb::MCTruth> > mclist;
    if (evt.getByLabel(fGenieGenModuleLabel,mctruthListHandle))
      art::fill_ptr_vector(mclist, mctruthListHandle);
    
    mcevts_truth=mclist.size();
    if (mcevts_truth){
      art::Ptr<simb::MCTruth> mctruth = mclist[0];
      nuPDG_truth  = mctruth->GetNeutrino().Nu().PdgCode();
      ccnc_truth   = mctruth->GetNeutrino().CCNC();
      mode_truth   = mctruth->GetNeutrino().Mode();
      Q2_truth     = mctruth->GetNeutrino().QSqr();
      W_truth      = mctruth->GetNeutrino().W();
      X_truth      = mctruth->GetNeutrino().X();
      Y_truth      = mctruth->GetNeutrino().Y();
      hitnuc_truth = mctruth->GetNeutrino().HitNuc();
      enu_truth    = mctruth->GetNeutrino().Nu().E();
      nuvtxx_truth = mctruth->GetNeutrino().Nu().Vx();
      nuvtxy_truth = mctruth->GetNeutrino().Nu().Vy();
      nuvtxz_truth = mctruth->GetNeutrino().Nu().Vz();
      if (mctruth->GetNeutrino().Nu().P()){
	nu_dcosx_truth = mctruth->GetNeutrino().Nu().Px()/mctruth->GetNeutrino().Nu().P();
	nu_dcosy_truth = mctruth->GetNeutrino().Nu().Py()/mctruth->GetNeutrino().Nu().P();
	nu_dcosz_truth = mctruth->GetNeutrino().Nu().Pz()/mctruth->GetNeutrino().Nu().P();
      }
      lep_mom_truth = mctruth->GetNeutrino().Lepton().P();
      if (mctruth->GetNeutrino().Lepton().P()){
	lep_dcosx_truth = mctruth->GetNeutrino().Lepton().Px()/mctruth->GetNeutrino().Lepton().P();
	lep_dcosy_truth = mctruth->GetNeutrino().Lepton().Py()/mctruth->GetNeutrino().Lepton().P();
	lep_dcosz_truth = mctruth->GetNeutrino().Lepton().Pz()/mctruth->GetNeutrino().Lepton().P();
      }
    }

  }

  fTree->Fill();
}

void dunefd::NueAna::beginJob()
{
  // Implementation of optional member function here.
  art::ServiceHandle<art::TFileService> tfs;
  fTree = tfs->make<TTree>("nueana","analysis tree");
  fTree->Branch("run",&run,"run/I");
  fTree->Branch("subrun",&subrun,"subrun/I");
  fTree->Branch("event",&event,"event/I");
  fTree->Branch("evttime",&evttime,"evttime/F");
  fTree->Branch("taulife",&taulife,"taulife/F");
  fTree->Branch("isdata",&isdata,"isdata/S");
  fTree->Branch("ntracks_reco",&ntracks_reco,"ntracks_reco/I");
  fTree->Branch("trkid",trkid,"trkid[ntracks_reco]/I");
  fTree->Branch("trkstartx",trkstartx,"trkstartx[ntracks_reco]/F");
  fTree->Branch("trkstarty",trkstarty,"trkstarty[ntracks_reco]/F");
  fTree->Branch("trkstartz",trkstartz,"trkstartz[ntracks_reco]/F");
  fTree->Branch("trkendx",trkendx,"trkendx[ntracks_reco]/F");
  fTree->Branch("trkendy",trkendy,"trkendy[ntracks_reco]/F");
  fTree->Branch("trkendz",trkendz,"trkendz[ntracks_reco]/F");
  fTree->Branch("trkstartdcosx",trkstartdcosx,"trkstartdcosx[ntracks_reco]/F");
  fTree->Branch("trkstartdcosy",trkstartdcosy,"trkstartdcosy[ntracks_reco]/F");
  fTree->Branch("trkstartdcosz",trkstartdcosz,"trkstartdcosz[ntracks_reco]/F");
  fTree->Branch("trkenddcosx",trkenddcosx,"trkenddcosx[ntracks_reco]/F");
  fTree->Branch("trkenddcosy",trkenddcosy,"trkenddcosy[ntracks_reco]/F");
  fTree->Branch("trkenddcosz",trkenddcosz,"trkenddcosz[ntracks_reco]/F");
  fTree->Branch("trklen",trklen,"trklen[ntracks_reco]/F");
  fTree->Branch("nhits",&nhits,"nhits/I");
  fTree->Branch("hit_plane",hit_plane,"hit_plane[nhits]/S");
  fTree->Branch("hit_wire",hit_wire,"hit_wire[nhits]/S");
  fTree->Branch("hit_channel",hit_channel,"hit_channel[nhits]/I");
  fTree->Branch("hit_peakT",hit_peakT,"hit_peakT[nhits]/F");
  fTree->Branch("hit_charge",hit_charge,"hit_charge[nhits]/F");
  fTree->Branch("hit_startT",hit_startT,"hit_startT[nhits]/F");
  fTree->Branch("hit_endT",hit_endT,"hit_endT[nhits]/F");
  fTree->Branch("nvtx",&nvtx,"nvtx/S");
  fTree->Branch("vtx",vtx,"vtx[nvtx][3]/F");
  fTree->Branch("mcevts_truth",&mcevts_truth,"mcevts_truth/I");
  fTree->Branch("nuPDG_truth",&nuPDG_truth,"nuPDG_truth/I");
  fTree->Branch("ccnc_truth",&ccnc_truth,"ccnc_truth/I");
  fTree->Branch("mode_truth",&mode_truth,"mode_truth/I");
  fTree->Branch("enu_truth",&enu_truth,"enu_truth/F");
  fTree->Branch("Q2_truth",&Q2_truth,"Q2_truth/F");
  fTree->Branch("W_truth",&W_truth,"W_truth/F");
  fTree->Branch("X_truth",&X_truth,"X_truth/F");
  fTree->Branch("Y_truth",&Y_truth,"Y_truth/F");
  fTree->Branch("hitnuc_truth",&hitnuc_truth,"hitnuc_truth/I");
  fTree->Branch("nuvtxx_truth",&nuvtxx_truth,"nuvtxx_truth/F");
  fTree->Branch("nuvtxy_truth",&nuvtxy_truth,"nuvtxy_truth/F");
  fTree->Branch("nuvtxz_truth",&nuvtxz_truth,"nuvtxz_truth/F");
  fTree->Branch("nu_dcosx_truth",&nu_dcosx_truth,"nu_dcosx_truth/F");
  fTree->Branch("nu_dcosy_truth",&nu_dcosy_truth,"nu_dcosy_truth/F");
  fTree->Branch("nu_dcosz_truth",&nu_dcosz_truth,"nu_dcosz_truth/F");
  fTree->Branch("lep_mom_truth",&lep_mom_truth,"lep_mom_truth/F");
  fTree->Branch("lep_dcosx_truth",&lep_dcosx_truth,"lep_dcosx_truth/F");
  fTree->Branch("lep_dcosy_truth",&lep_dcosy_truth,"lep_dcosy_truth/F");
  fTree->Branch("lep_dcosz_truth",&lep_dcosz_truth,"lep_dcosz_truth/F");

}

void dunefd::NueAna::ResetVars(){

  run = -9999;
  subrun = -9999;
  event = -9999;
  evttime = -9999;
  taulife = 0;
  isdata = -9999;

  ntracks_reco = 0;
  for (int i = 0; i < kMaxTrack; ++i){
    trkid[i] = -9999;
    trkstartx[i] = -9999;
    trkstarty[i] = -9999;
    trkstartz[i] = -9999;
    trkendx[i] = -9999;
    trkendy[i] = -9999;
    trkendz[i] = -9999;
    trkstartdcosx[i] = -9999;
    trkstartdcosy[i] = -9999;
    trkstartdcosz[i] = -9999;
    trkenddcosx[i] = -9999;
    trkenddcosy[i] = -9999;
    trkenddcosz[i] = -9999;
    trklen[i] = -9999;
  }

  nhits = 0;
  for (int i = 0; i<kMaxHits; ++i){
    hit_plane[i] = -9999;
    hit_wire[i] = -9999;
    hit_channel[i] = -9999;
    hit_peakT[i] = -9999;
    hit_charge[i] = -9999;
    hit_startT[i] = -9999;
    hit_endT[i] = -9999;
  }

  nvtx = 0;
  for (int i = 0; i<kMaxVertices; ++i){
    vtx[i][0] = -9999;
    vtx[i][1] = -9999;
    vtx[i][2] = -9999;
  }

  mcevts_truth = -9999; 
  nuPDG_truth = -9999;  
  ccnc_truth = -9999;    
  mode_truth = -9999;   
  enu_truth = -9999;     
  Q2_truth = -9999;       
  W_truth = -9999;        
  X_truth = -9999;
  Y_truth = -9999;
  hitnuc_truth = -9999;  
  nuvtxx_truth = -9999;   
  nuvtxy_truth = -9999;   
  nuvtxz_truth = -9999;   
  nu_dcosx_truth = -9999; 
  nu_dcosy_truth = -9999; 
  nu_dcosz_truth = -9999; 
  lep_mom_truth = -9999;  
  lep_dcosx_truth = -9999;
  lep_dcosy_truth = -9999;
  lep_dcosz_truth = -9999;
  
}

void dunefd::NueAna::endJob()
{
  // Implementation of optional member function here.
}

void dunefd::NueAna::reconfigure(fhicl::ParameterSet const & p)
{
  fHitsModuleLabel     =   p.get< std::string >("HitsModuleLabel");
  fTrackModuleLabel    =   p.get< std::string >("TrackModuleLabel");
	fClusterModuleLabel  =   p.get< std::string >("ClusterModuleLabel");
  fVertexModuleLabel   =   p.get< std::string >("VertexModuleLabel");
  fGenieGenModuleLabel =   p.get< std::string >("GenieGenModuleLabel");
  fFidVolCut           =   p.get< double >("FidVolCut");
  return;
}

/***********************************************************************/

bool dunefd::NueAna::insideFidVol(const TLorentzVector& pvtx) 
{
	art::ServiceHandle<geo::Geometry> geom;

	double vtx[3];
	vtx[0] = pvtx.X(); vtx[1] = pvtx.Y(); vtx[2] = pvtx.Z();

	bool inside = true;
	geo::TPCID idtpc = geom->FindTPCAtPosition(vtx);
	if (geom->HasTPC(idtpc))
	{
		const geo::TPCGeo& tpcgeo = geom->GetElement(idtpc); 
		if ((fabs(vtx[0] - tpcgeo.MinX()) < fFidVolCut) ||
				(fabs(tpcgeo.MaxX() - vtx[0]) < fFidVolCut) ||
				(fabs(vtx[1] - tpcgeo.MinY()) < fFidVolCut) ||
				(fabs(tpcgeo.MaxY() - vtx[1]) < fFidVolCut) ||
				(fabs(vtx[2] - tpcgeo.MinZ()) < fFidVolCut) ||
				(fabs(tpcgeo.MaxZ() - vtx[2]) < fFidVolCut)) inside = false;
	}
	else inside = false;
		
	return inside;
}

/***********************************************************************/


DEFINE_ART_MODULE(dunefd::NueAna)
