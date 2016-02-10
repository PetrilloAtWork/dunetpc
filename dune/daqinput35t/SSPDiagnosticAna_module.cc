// -*- mode: c++; c-basic-offset: 2; -*-
////////////////////////////////////////////////////////////////////////
// Class:       SSPDiagnosticAna
// Module Type: producer
// File:        SSPDiagnosticAna_module.cc
//
// Quickly analyze raw data trigger rate in the SSP
// and plot pulse amplitudes
//
// Jonathan Insler jti3@fnal.gov
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TH2.h"
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <iomanip>

//lbne-artdaq includes
#include "lbne-raw-data/Overlays/SSPFragment.hh"
#include "lbne-raw-data/Overlays/anlTypes.hh"
#include "artdaq-core/Data/Fragments.hh"

//larsoft includes
#include "RawData/raw.h"
#include "RawData/OpDetWaveform.h"
#include "Geometry/Geometry.h"
//#include "Utilities/TimeService.h"

//daqinput35t includes

#include "utilities/UnpackFragment.h"
#include "SSPReformatterAlgs.h"

namespace DAQToOffline {
  class SSPDiagnosticAna;
}

class DAQToOffline::SSPDiagnosticAna : public art::EDAnalyzer {
public:
  explicit SSPDiagnosticAna(fhicl::ParameterSet const & pset);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.
  
  void analyze(art::Event const & evt);
  void reconfigure(fhicl::ParameterSet const& pset);
  void printParameterSet();


   // Plugins should not be copied or assigned.
  SSPDiagnosticAna(SSPDiagnosticAna const &) = delete;
  SSPDiagnosticAna(SSPDiagnosticAna &&) = delete;
  SSPDiagnosticAna & operator = (SSPDiagnosticAna const &) = delete;
  SSPDiagnosticAna & operator = (SSPDiagnosticAna &&) = delete;
 
private:
  void beginJob() override;
  void endJob  () override;
  void beginEvent(art::EventNumber_t eventNumber);
  void endEvent  (art::EventNumber_t eventNumber);

  std::string fFragType;
  std::string fRawDataLabel;
  std::string fOutputDataLabel;
  double fSampleFreq;        // Sampling frequency in MHz 
  const int m1_window = 10;
  const int i1_window = 500;
  const int i2_window = 500;

  // Map to store how many waveforms are on one optical channel
  // std::map< int, TH1D* > averageWaveforms;
  // std::map< int, int   > waveformCount;


  TH1D *fPulseAmplitude;
  std::map< int, TH1D* > PulseAmplitudePerChannel;
  std::map< int, TH1D* > IntegratedChargePerChannel;
  std::map< int, TH2D* > PulseAmplitudeVsIntegratedChargePerChannel;

  SSPReformatterAlgs sspReform;

  unsigned long int firstTime;
  unsigned long int lastTime;
  std::map<int, long int> triggerCount;


};


DAQToOffline::SSPDiagnosticAna::SSPDiagnosticAna(fhicl::ParameterSet const & pset)
                                         : art::EDAnalyzer(pset),
                                           sspReform(pset.get<fhicl::ParameterSet>("SSPReformatter"))
{

  this->reconfigure(pset);

  //first_FirstSample = -1;
  //first_TimeStamp = -1;
}

void DAQToOffline::SSPDiagnosticAna::reconfigure(fhicl::ParameterSet const& pset){

  fFragType           = pset.get<std::string>("FragType");
  fRawDataLabel       = pset.get<std::string>("RawDataLabel");
  // m1_window           = pset.get<int>("SSPm1");
  // i1_window           = pset.get<int>("SSPi1");
  // i2_window           = pset.get<int>("SSPi2");

  //fDebug = pset.get<bool>("Debug");
  //fZeroThreshold=0;
  //fCompression=raw::kNone;
       // Obtain parameters from TimeService
        // art::ServiceHandle< util::TimeService > timeService;
        // fSampleFreq = timeService->OpticalClock().Frequency();


  printParameterSet();
  
}

void DAQToOffline::SSPDiagnosticAna::printParameterSet(){

  mf::LogDebug("SSPDiagnosticAna") << "===================================="   << "\n"
			       << "Parameter Set"                          << "\n"
			       << "===================================="   << "\n"
			       << "fFragType:        " << fFragType        << "\n"
			       << "fRawDataLabel:    " << fRawDataLabel    << "\n"
			       << "===================================="   << "\n";
}

void DAQToOffline::SSPDiagnosticAna::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  //adc_values_ = tfs->make<TH1D>("adc_values","adc_values",4096,-0.5,4095.5);

  fPulseAmplitude = tfs->make<TH1D>("pulseamplitude","Pulse Amplitude",125,-50,200);

  firstTime = (((unsigned long int)1)<<63);
  lastTime = 0;
}

void DAQToOffline::SSPDiagnosticAna::beginEvent(art::EventNumber_t /*eventNumber*/)
{
  //reset ADC histogram
  //adc_values_->Reset();
  //reset counters
  //n_adc_counter_  = 0;
  //adc_cumulative_ = 0;
}

void DAQToOffline::SSPDiagnosticAna::endEvent(art::EventNumber_t eventNumber)
{
  //write the ADC histogram for the given event
  //if(n_adc_counter_)
    //  adc_values_->Write(Form("adc_values:event_%d", eventNumber));
}

void DAQToOffline::SSPDiagnosticAna::endJob()
{
  //delete adc_values_;
  
  long int deltaT = lastTime-firstTime;
  double deltaTus =  ((double)deltaT)/sspReform.ClockFrequency();


  mf::LogVerbatim("SSPDiagnosticAna") << "!! Diagnostic Rate Report." << std::endl;
  mf::LogVerbatim("SSPDiagnosticAna") << "!! Time: " << deltaTus / 60.e6 << " minutes." << std::endl;

  for (auto itr = triggerCount.begin(); itr != triggerCount.end(); itr++) {
    double freq = ((double)itr->second) / deltaTus * 1000.;
    mf::LogVerbatim("SSPDiagnosticAna") << "!!    Channel " << std::setw(3) << itr->first << ": " << freq << " kHz" << std::endl;
  }
  
  
  /*
  mf::LogInfo("SSPDiagnosticAna") << "firstSample:  " << firstTime << " samples\n"
                               << "lastSample:   " << lastTime  << " samples\n"
                               << "totalSamples: " << deltaT    << " samples\n"
                               << "totalTime:    " << deltaTus  << " us\n"
                               << "# Channels:   " << channels.size() << "\n"
                               << "# Triggers:   " << triggerCount << "\n"
                               << "Frequency:    " << freq << "kHz\n";
  */

  // for (auto iter = averageWaveforms.begin(); iter != averageWaveforms.end(); iter++)
  //   {
  //     mf::LogInfo("Scaling down channel ") << iter->first << " by 1./" << waveformCount[iter->first] << std::endl;
  //     iter->second->Scale(1./waveformCount[iter->first]);
  //   }



}



void DAQToOffline::SSPDiagnosticAna::analyze(art::Event const & evt)
{

  art::ServiceHandle<art::TFileService> tfs;
  //art::ServiceHandle<geo::Geometry> geo;

  art::Handle<artdaq::Fragments> rawFragments;
  evt.getByLabel(fRawDataLabel, fFragType, rawFragments);

  mf::LogInfo("SSPDiagnosticAna") << "Starting event analysis";

  // Check if there is SSP data in this event
  // Don't crash code if not present, just don't save anything
  try { rawFragments->size(); }
  catch(std::exception e) {
    mf::LogWarning("SSPDiagnosticAna") << "WARNING: Raw SSP data not found in event " << evt.event();
    return;
  }

  // Check that the data is valid
  if(!rawFragments.isValid()){
    mf::LogError("SSPDiagnosticAna") << "Run: " << evt.run()
				 << ", SubRun: " << evt.subRun()
				 << ", Event: " << evt.event()
				 << " is NOT VALID";
    throw cet::exception("raw NOT VALID");
    return;
  }

  mf::LogInfo("SSPDiagnosticAna") << "Data is valid!";

  // // Get OpDetWaveforms from the event
  // art::Handle< std::vector< raw::OpDetWaveform > > waveformHandle;
  // evt.getByLabel(fRawDataLabel, fFragType, waveformHandle);

  // for (size_t i = 0; i < waveformHandle->size(); i++)
  //   {
      
  //     // This was probably required to overcome the "const" problem 
  //     // with OpDetPulse::Waveform()
  //     art::Ptr< raw::OpDetWaveform > waveformPtr(waveformHandle, i);
  //     raw::OpDetWaveform pulse = *waveformPtr;
  //     int channel = pulse.ChannelNumber();

  //     // Create the TH1 if it doesn't exist
  //     auto waveform = averageWaveforms.find( channel );
  //     if ( waveform == averageWaveforms.end() ) {
  // 	TString histName = TString::Format("avgwaveform_channel_%03i", channel);
  // 	averageWaveforms[channel] =  tfs->make< TH1D >(histName, ";t (us);", pulse.size(), 0, double(pulse.size()) / fSampleFreq);
  //     }

  //     // Add this waveform to this histogram
  //     for (size_t tick = 0; tick < pulse.size(); tick++) {
  // 	averageWaveforms[channel]->Fill(double(tick)/fSampleFreq, pulse[tick]);

  //     }
  //     // Count number of waveforms on each channel
  //     waveformCount[channel]++;
      
	    
  //   }


  unsigned int numFragments = rawFragments->size();
  mf::LogInfo("SSPDiagnosticAna") << "Number of fragments = " << numFragments;

  for (size_t idx = 0; idx < numFragments; ++idx) {

     mf::LogInfo("SSPDiagnosticAna") << "Number of fragments = " << idx;

    const auto& frag((*rawFragments)[idx]);
    lbne::SSPFragment sspf(frag);

    unsigned int nTriggers = sspReform.CheckAndGetNTriggers(frag, sspf);

    mf::LogInfo("SSPDiagnosticAna") << "Triggers = " << nTriggers;
      
    const unsigned int* dataPointer = sspf.dataBegin();

        
    for (unsigned int triggersProcessed = 0;
         (nTriggers==0 || triggersProcessed < nTriggers) && dataPointer < sspf.dataEnd();
         ++triggersProcessed) {

      //
      // The elements of the OpDet Pulse
      //
      unsigned short     OpChannel = -1;       ///< Derived Optical channel
      unsigned long      FirstSample = 0;      ///< first sample time in ticks
        

      // Load the event header, advance the pointer
      const SSPDAQ::EventHeader* daqHeader=reinterpret_cast<const SSPDAQ::EventHeader*>(dataPointer);
      dataPointer += sizeof(SSPDAQ::EventHeader)/sizeof(unsigned int);

      // Get ADC Count, create pointer to adcs
      unsigned int nADC=(daqHeader->length-sizeof(SSPDAQ::EventHeader)/sizeof(unsigned int))*2;

      //get the information from the header
      try {
        OpChannel = sspReform.GetOpChannel(daqHeader);

        FirstSample = sspReform.GetGlobalFirstSample(daqHeader);
        //TimeStamp = ((double)FirstSample)/fNOvAClockFrequency;

	double peakSum = sspReform.GetPeakSum(daqHeader);
	double prerise = sspReform.GetBaselineSum(daqHeader);
	double integratedSum = sspReform.GetIntegratedSum(daqHeader);

	double PulseAmplitude = -prerise/i2_window*1.0 + peakSum/m1_window*1.0;

	int channel = (int) OpChannel; 

	if(channel==94){
	  if(PulseAmplitude>50&&PulseAmplitude<55)//event with 5 p.e. peak in channel 94
	    std::cout << "Channel 94 has 5 p.e. peak in event" << evt.event() << std::endl;
	  
	  if(PulseAmplitude>75&&PulseAmplitude<85)//event with 5 p.e. peak in channel 94
	    std::cout << "Channel 94 has 7 p.e. peak in event" << evt.event() << std::endl;
	}

	double IntegratedCharge = prerise/i2_window*i1_window*1.0 - integratedSum;

	fPulseAmplitude->Fill(PulseAmplitude);
	mf::LogInfo("SSPDiagnosticAna") << "Pulse Amplitude: " << PulseAmplitude;
	

	auto pulse_amplitude_per_channel = PulseAmplitudePerChannel.find( channel);
	if( pulse_amplitude_per_channel == PulseAmplitudePerChannel.end() ) {
	  TString histName = TString::Format("pulse_amplitude_channel_%03i", channel);
	  TString histTitle = TString::Format("Pulse Amplitude for OP Channel %03i", channel);
	  PulseAmplitudePerChannel[channel] =  tfs->make< TH1D >(histName, histTitle ,125,-50,200);
	}
	
	PulseAmplitudePerChannel[channel]->Fill(PulseAmplitude);

	auto integrated_charge_per_channel = IntegratedChargePerChannel.find( channel);
	if( integrated_charge_per_channel == IntegratedChargePerChannel.end() ) {
	  TString histName = TString::Format("integrated_charge_channel_%03i", channel);
	  TString histTitle = TString::Format("Integrated Charge on OP Channel %03i", channel);
	  IntegratedChargePerChannel[channel] =  tfs->make< TH1D >(histName, histTitle ,125,0,1e4);
	}
	
	IntegratedChargePerChannel[channel]->Fill(IntegratedCharge);

	auto pulse_amplitude_vs_integrated_charge_per_channel = PulseAmplitudeVsIntegratedChargePerChannel.find( channel);
	if( pulse_amplitude_vs_integrated_charge_per_channel == PulseAmplitudeVsIntegratedChargePerChannel.end() ) {
	  TString histName = TString::Format("pulse_amplitude_vs_integrated_charge_channel_%03i", channel);
	  TString histTitle = TString::Format("Pulse Amplitude vs. Integrated Charge on OP Channel %03i", channel);
	  PulseAmplitudeVsIntegratedChargePerChannel[channel] =  tfs->make< TH2D >(histName, histTitle ,125,0,1e4,125,-50,200);
	}
	
	PulseAmplitudeVsIntegratedChargePerChannel[channel]->Fill(IntegratedCharge,PulseAmplitude);

	

        if (FirstSample < 1e16) {
          sspReform.PrintHeaderInfo(daqHeader);
          mf::LogInfo("SSPDiagnosticAna") << "Problem timestamp at " << FirstSample << std::endl;
          continue;
        }
      } 
      catch (cet::exception e) {
        continue;
      }

      firstTime = std::min(firstTime, FirstSample);
      lastTime  = std::max(lastTime,  FirstSample);
      triggerCount[OpChannel]++;
      
      // Advance the dataPointer to the next header
      dataPointer+=nADC/2;
      
    } // End of loop over triggers
  } // End of loop over fragments (rawFragments)

    
}

DEFINE_ART_MODULE(DAQToOffline::SSPDiagnosticAna)


