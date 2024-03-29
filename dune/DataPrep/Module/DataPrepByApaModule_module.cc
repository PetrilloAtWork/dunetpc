// DataPrepByApaModule_module.cc

// David Adams
// October 2019
//
// Module that obtain RawDigit and TimeStamp containers from a decoder tool, builds
// AdcChannelData containers and runs RawDigitPrepService on those.
// It may write all the decoded digits and/or time stamps and selected wires.
//
// Configuration parameters:
//             LogLevel - Usual logging level.
//          DecoderTool - Name of the tool used to fetch the raw digits.
//      OutputDigitName - Name for the output digit container. If blank, digits are not written.
//  OutputTimeStampName - Name for the output wire container. If blank, time stamps are not written.
//       OutputWireName - Name for the output wire container. If blank, wires are not written.
//        ChannelGroups - Process channels appearing in these groups or ranges.
//                        The group for each name is obtained from the tool channelGroups.
//                        If that fails, a range with that name is obtained from the tool
//                        channelRanges.
//                        If the entry "all" appears, then all channels are decoded at once and all
//                        channels are processed together.
//                        Otherwise each APA is read in and processed individually.
//                        The group "apas" should be defined to include all APAs.
//       BeamEventLabel - Label for the BeamEvent (trigger) data product. If blank, it is not read.
//     ApaChannelCounts - Number of channels in each APA. Last value is used for subsequent APAs.
// OnlineChannelMapTool - Name of the tool that converts offline to online channel number.

#include "art/Framework/Core/ModuleMacros.h" 
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "dune/Protodune/singlephase/RawDecoding/data/RDStatus.h"
#include "dune/Protodune/singlephase/RawDecoding/PDSPTPCDataInterfaceParent.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RecoBase/Wire.h"
#include "lardata/Utilities/AssociationUtil.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"
#include "dune/DuneInterface/RawDigitPrepService.h"
#include "dune/DuneInterface/Tool/IndexMapTool.h"
#include "dune/DuneInterface/Tool/IndexRangeTool.h"
#include "dune/DuneInterface/Tool/IndexRangeGroupTool.h"
#include "dune/DuneCommon/DuneTimeConverter.h"
#include "dune/ArtSupport/DuneToolManager.h"
#include "TTimeStamp.h"
#include "lardataobj/RawData/RDTimeStamp.h"
#include "dune/DuneObj/ProtoDUNEBeamEvent.h"
#include <iomanip>
#include <set>
#include <sstream>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::move;
using std::setw;
using art::ServiceHandle;
using art::Timestamp;
using raw::RDStatus;
using recob::Wire;
using std::istringstream;
using std::ostringstream;

//**********************************************************************

class DataPrepByApaModule : public art::EDProducer {

public:
    
  using Index = unsigned int;
  using Name = std::string;
  using NameVector = std::vector<Name>;
  using ApaNamesMap = std::map<int, NameVector>;
  using ApaChannelSet = std::set<Index>;
  using ApaChannelSetMap = std::map<int, ApaChannelSet>;

  // Ctor.
  explicit DataPrepByApaModule(fhicl::ParameterSet const& pset); 

  // Dtor.
  ~DataPrepByApaModule();
    
  // Producer methods.
  void produce(art::Event& evt);
  void beginJob();
  void endJob();
  void reconfigure(fhicl::ParameterSet const& p);
    
private:
    
  // Configuration parameters.
  int m_LogLevel;
  Name m_DecoderTool; // Name for the decoder tool
  Name m_OutputTimeStampName;    // Label for the output RDTimeStamp conainer
  Name m_OutputDigitName;  // Label for the output raw::RawDigit conainer
  Name m_OutputWireName;    ///< Second field in full label for the output wire container.
  NameVector m_ChannelGroups;
  std::string m_BeamEventLabel;
  AdcChannelVector m_KeepChannels;
  AdcChannelVector m_SkipChannels;
  AdcChannelVector m_ApaChannelCounts;

  // Split label into producer and name: PRODUCER or PRODUCER:NAME
  std::string m_DigitProducer;
  std::string m_DigitName;

  // Accessed services.
  const lariov::ChannelStatusProvider* m_pChannelStatusProvider;
  RawDigitPrepService* m_pRawDigitPrepService = nullptr;

  // Tools.
  std::string m_OnlineChannelMapTool;
  std::unique_ptr<PDSPTPCDataInterfaceParent> m_pDecoderTool;
  std::unique_ptr<IndexMapTool> m_onlineChannelMapTool;

  // Channel range names and channel sets indexed by APA.
  ApaNamesMap m_apacrns;
  ApaChannelSetMap m_apachsets;

  // Maximum size for the output containers.
  Index m_maxOutputDigitChannelCount =0;
  Index m_maxOutputTimeStampChannelCount =0;
  Index m_maxOutputWireChannelCount =0;

  // Processed event count.
  Index m_nproc =0;

  // Skipped event count.
  Index m_nskip =0;

};

DEFINE_ART_MODULE(DataPrepByApaModule)
  
//**********************************************************************

DataPrepByApaModule::DataPrepByApaModule(fhicl::ParameterSet const& pset) : EDProducer{pset} {
  const Name myname = "DataPrepByApaModule::ctor: ";
  this->reconfigure(pset);
  if ( m_OutputTimeStampName.size() ) {
    if ( m_LogLevel > 0 ) {
      cout << myname << "Module will produce RDTimeStamps with name " << m_OutputTimeStampName << endl;
    }
    produces<std::vector<raw::RDTimeStamp>>(m_OutputTimeStampName);
  } else if ( m_LogLevel > 0 ) {
    cout << myname << "Module will not produce RDTimeStamps." << endl;
  }
  if ( m_OutputDigitName.size() ) {
    if ( m_LogLevel > 0 ) {
      cout << myname << "Module will produce digits with name " << m_OutputDigitName << endl;
    }
    produces<std::vector<raw::RawDigit>>(m_OutputDigitName);
  } else if ( m_LogLevel > 0 ) {
    cout << myname << "Module will not produce RawDigits." << endl;
  }
  if ( m_OutputWireName.size() ) {
    if ( m_LogLevel > 0 ) {
      cout << myname << "Module will produce Wires with name " << m_OutputWireName << endl;
    }
    produces<std::vector<recob::Wire>>(m_OutputWireName);
  } else if ( m_LogLevel > 0 ) {
    cout << myname << "Module will not produce Wires." << endl;
  }
}
  
//**********************************************************************

DataPrepByApaModule::~DataPrepByApaModule() { }

//**********************************************************************

void DataPrepByApaModule::reconfigure(fhicl::ParameterSet const& pset) {
  const string myname = "DataPrepByApaModule::reconfigure: ";
  m_LogLevel            = pset.get<int>("LogLevel");
  m_DecoderTool         = pset.get<Name>("DecoderTool");
  m_OutputTimeStampName = pset.get<Name>("OutputTimeStampName");
  m_OutputDigitName     = pset.get<Name>("OutputDigitName");
  m_OutputWireName      = pset.get<Name>("OutputWireName", "");
  m_ChannelGroups       = pset.get<NameVector>("ChannelGroups");
  m_BeamEventLabel      = pset.get<string>("BeamEventLabel");
  m_KeepChannels        = pset.get<AdcChannelVector>("KeepChannels");
  m_SkipChannels        = pset.get<AdcChannelVector>("SkipChannels");
  m_ApaChannelCounts    = pset.get<AdcChannelVector>("ApaChannelCounts");
  pset.get_if_present<std::string>("OnlineChannelMapTool", m_OnlineChannelMapTool);

  // Display configuration.
  if ( m_LogLevel >= 1 ) {
    cout << myname << "             LogLevel: " << m_LogLevel << endl;
    cout << myname << "          DecoderTool: " << m_DecoderTool << endl;
    cout << myname << "  OutputTimeStampName: " << m_OutputTimeStampName << endl;
    cout << myname << "      OutputDigitName: " << m_OutputDigitName << endl;
    cout << myname << "       OutputWireName: " << m_OutputWireName << endl;
    cout << myname << "        ChannelGroups: [";
    bool first = true;
    for ( Name rnam : m_ChannelGroups ) {
      if ( first ) first = false;
      else cout << ", ";
      cout << rnam;
    }
    cout << "]" << endl;
    cout << myname << "       BeamEventLabel: " << m_BeamEventLabel << endl;
    cout << myname << " OnlineChannelMapTool: " << m_OnlineChannelMapTool << endl;
    cout << myname << "         KeepChannels: " << "[";
    first = true;
    for ( AdcChannel ich : m_KeepChannels ) {
      if ( first ) first = false;
      else cout << ", ";
      cout << ich;
    }
    cout << "]" << endl;
    cout << myname << "         SkipChannels: " << "[";
    first = true;
    for ( AdcChannel ich : m_SkipChannels ) {
      if ( first ) first = false;
      else cout << ", ";
      cout << ich;
    }
    cout << "]" << endl;
    cout << myname << "     ApaChannelCounts: " << "[";
    first = true;
    for ( AdcChannel ich : m_ApaChannelCounts ) {
      if ( first ) first = false;
      else cout << ", ";
      cout << ich;
    }
    cout << "]" << endl;
  }

  // Fetch services.
  m_pChannelStatusProvider = &art::ServiceHandle<lariov::ChannelStatusService>()->GetProvider();
  if ( m_pChannelStatusProvider == nullptr ) {
    cout << myname << "WARNING: Channel status provider not found." << endl;
  }
  m_pRawDigitPrepService = &*ServiceHandle<RawDigitPrepService>();

  // Fetch tools.
  DuneToolManager* ptm = DuneToolManager::instance();
  if ( ptm == nullptr ) {
    cout << myname << "ERROR: Unable to retrieve tool manager." << endl;
  }
  if ( m_DecoderTool.size() ) {
    m_pDecoderTool = ptm->getPrivate<PDSPTPCDataInterfaceParent>(m_DecoderTool);
    if ( ! m_pDecoderTool ) {
      cout << myname << "ERROR: Decoder tool not found: " << m_DecoderTool << endl;
      return;
    }
  }
  if ( m_OnlineChannelMapTool.size() ) {
    m_onlineChannelMapTool = ptm->getPrivate<IndexMapTool>(m_OnlineChannelMapTool);
  }
  const IndexRangeTool* pcrt = ptm->getShared<IndexRangeTool>("channelRanges");
  if ( pcrt == nullptr ) {
    cout << myname << "ERROR: Channel range tool channelRanges not found." << endl;
    return;
  }
  const IndexRangeGroupTool* pcgt = ptm->getShared<IndexRangeGroupTool>("channelGroups");
  if ( pcgt == nullptr ) {
    cout << myname << "WARNING: Channel group tool channelGroups not found." << endl;
  }

  // Build the vector of channel range names from the vector of channel group names.
  // If a group name is valid, add that group's ranges.
  // Otherwise, assume the group name is a range name.
  bool useGroups = pcgt != nullptr;
  NameVector rangeNames;
  for ( Name groupName : m_ChannelGroups ) {
    IndexRangeGroup grp;
    if ( useGroups ) grp = pcgt->get(groupName);
    if ( grp.isValid() ) {
      for ( const IndexRange& ran : grp.ranges ) {
        rangeNames.push_back(ran.name);
      }
    } else {
      rangeNames.push_back(groupName);
    }
  }

  // Copy the channels to skip to a set for fast lookup.
  ApaChannelSet skipChans;
  for ( Index icha : m_SkipChannels ) skipChans.insert(icha);
  ApaChannelSet keepChans;
  for ( Index icha : m_KeepChannels ) keepChans.insert(icha);

  // Sort channel ranges by APA and record the channels for each..
  int ncrn = 0;
  AdcChannel nchaAll = 0;
  for ( Name crn : rangeNames ) {
    // If any range is named "all", we keep it only.
    int iapa = -99;
    if ( crn == "all" ) {
      m_apacrns.clear();
      m_apachsets.clear();
      iapa = -1;
      ncrn = -1;
    } else {
      string::size_type ipos = string::npos;
      if ( crn.substr(0,3) == "apa" ) ipos = 3;
      if ( crn.substr(0,4) == "femb" ) ipos = 4;
      if ( ipos == string::npos ) {
        cout << myname << "WARNING: Skipping no-APA channel range: " << crn << endl;
        continue;
      }
      std::istringstream ssapa(crn.substr(ipos,1));
      ssapa >> iapa;
      if ( iapa <= 0 ) {
        cout << myname << "ERROR: Unable to extract APA index from channel range " << crn << endl;
        continue;
      }
    }
    const IndexRange& ran = pcrt->get(crn);
    if ( ! ran.isValid() ) {
      cout << myname << "WARNING: No channels taken from invalid channel range " << ran.name << endl;
      continue;
    }
    m_apacrns[iapa].push_back(crn);
    for ( Index icha=ran.begin; icha<ran.end; ++icha ) {
      bool keep = (keepChans.size() == 0 || keepChans.count(icha)) && skipChans.count(icha) == 0;
      if ( keep ) m_apachsets[iapa].insert(icha);
    }
    if ( iapa == -1 ) {
      nchaAll = ran.size();
      break;
    }
    ++ncrn;
  }

  // Display the APAs to be processed.
  if ( m_apachsets.size() == 0 ) {
    cout << myname << "WARNING: No channels will be processed." << endl;
  } else if ( m_LogLevel >= 1 ) {
    cout << myname << "The following APAs will be processed." << endl;
    cout << myname << "-------------------------------------" << endl;
    cout << myname << " APA   # chan  Channel ranges" << endl;
    for ( ApaChannelSetMap::value_type iapa : m_apachsets ) {
      cout << myname << setw(4);
      if ( iapa.first == -1 ) cout << "all";
      else cout << iapa.first;
      cout << setw(9) << iapa.second.size() << "  {";
      bool first = true;
      for ( Name crn : m_apacrns[iapa.first] ) {
        if ( first ) first = false;
        else cout << ", ";
        cout << crn;
      }
      cout << "}" << endl;
    }
    cout << myname << "-------------------------------------" << endl;
  }
    
  // Evaluate the maximum number of channels in the output containers.
  AdcChannel nchaDefault = 2560;
  AdcChannel nchaIn = 0;
  AdcChannel nchaOut = 0;
  if ( m_ApaChannelCounts.size() == 0 ) {
    cout << myname << "WARNING: No APA channel counts have been provided. Using " << nchaDefault << endl;
  } else {
    nchaDefault = m_ApaChannelCounts.back();
  }
  for ( const ApaChannelSetMap::value_type& csmPair : m_apachsets ) {
    int iapaSigned = csmPair.first;
    AdcChannel nchaApa = nchaAll;
    if ( iapaSigned >= 0 ) {
      Index iapa = iapaSigned;
      nchaApa = iapa < m_ApaChannelCounts.size() ? m_ApaChannelCounts[iapa] : nchaDefault;
    }
    nchaIn += nchaApa;
    nchaOut += csmPair.second.size();
  }
  m_maxOutputDigitChannelCount     = m_OutputDigitName.size() == 0      ? 0 : nchaIn;
  m_maxOutputTimeStampChannelCount = m_OutputTimeStampName.size() == 0  ? 0 : nchaIn;
  m_maxOutputWireChannelCount      = m_OutputWireName.size() == 0       ? 0 : nchaOut;
  if ( m_LogLevel >= 1 ) {
    cout << myname << "          Max # read channels: " << nchaIn << endl;
    cout << myname << "     Max # processed channels: " << nchaOut << endl;
    cout << myname << "          Max # output digits: " << m_maxOutputDigitChannelCount << endl;
    cout << myname << "      Max # output timestamps: " << m_maxOutputTimeStampChannelCount << endl;
    cout << myname << "           Max # output wires: " << m_maxOutputWireChannelCount << endl;
  }

}

//**********************************************************************

void DataPrepByApaModule::beginJob() {
  const string myname = "DataPrepByApaModule::beginJob: ";
  if ( m_LogLevel >= 2 ) cout << myname << "Starting job." << endl;
  m_nproc = 0;
  m_nskip = 0;
}

//**********************************************************************

void DataPrepByApaModule::endJob() {
  const string myname = "DataPrepByApaModule::endJob: ";
  if ( m_LogLevel >= 1 ) {
    cout << myname << "# events processed: " << m_nproc << endl;
    cout << myname << "  # events skipped: " << m_nskip << endl;
  }
}
  
//**********************************************************************

void DataPrepByApaModule::produce(art::Event& evt) {      
  const string myname = "DataPrepByApaModule::produce: ";

  // Flag indicating that non-verbose info level messages should be logged.
  bool logInfo = m_LogLevel >= 2;

  // Control flags.
  bool skipAllEvents = false;
  bool skipEventsWithCorruptDataDropped = false;

  // Decode the event time.
  Timestamp beginTime = evt.time();
  time_t itim = beginTime.timeHigh();
  int itimrem = beginTime.timeLow();
  // Older protoDUNE data has time in low field.
  if ( itim == 0 && itimrem != 0 ) {
    itimrem = itim;
    itim = beginTime.timeLow();
  }

  // Log event processing header.
  if ( logInfo ) {
    cout << myname << "Run " << evt.run();
    if ( evt.subRun() ) cout << "-" << evt.subRun();
    cout << ", event " << evt.event();
    cout << ", nproc=" << m_nproc;
    if ( m_nskip ) cout << ", nskip=" << m_nskip;
    cout << endl;
    if ( m_LogLevel >= 3 ) cout << myname << "Reading raw digits for producer, name: " << m_DigitProducer << ", " << m_DigitName << endl;
    if ( evt.isRealData() ) {
      TTimeStamp rtim(itim, itimrem);
      string stim = string(rtim.AsString("s")) + " UTC";
      cout << myname << "Real data event time: " << itim << " (" << stim << ")" << endl;
    } else {
      cout << myname << "Sim data event time: " << DuneTimeConverter::toString(beginTime) << endl;
    }
  }
  if ( m_LogLevel >= 3 ) {
    cout << myname << "Event time high, low: " << beginTime.timeHigh() << ", " << beginTime.timeLow() << endl;
  }

  // Fetch the event trigger and timing clock.
  AdcIndex trigFlag = 0;
  AdcLongIndex timingClock = 0;
  using TimeStampVector  = std::vector<raw::RDTimeStamp>;
  art::Handle<TimeStampVector> htims;
  const TimeStampVector* ptims = nullptr;
  if ( true ) {
    evt.getByLabel("timingrawdecoder", "daq", htims);
    if ( htims.isValid() ) {
      ptims = &*htims;
      if ( ptims->size() != 1 ) {
        cout << myname << "WARNING: Unexpected timing clocks size: " << ptims->size() << endl;
        for ( unsigned int itim=0; itim<ptims->size() && itim<50; ++itim ) {
          cout << myname << "  " << ptims->at(itim).GetTimeStamp() << endl;
        }
      } else {
        const raw::RDTimeStamp& tim = ptims->at(0);
        if ( logInfo ) cout << myname << "Timing clock: " << tim.GetTimeStamp() << endl;
        timingClock = tim.GetTimeStamp();
        // See https://twiki.cern.ch/twiki/bin/view/CENF/TimingSystemAdvancedOp#Reference_info
        trigFlag = tim.GetFlags();
        if ( m_LogLevel >= 2 ) cout << myname << "Trigger flag: " << trigFlag << " (";
        bool isBeam = trigFlag == 0xc;
        bool isCrt = trigFlag == 13;
        bool isFake = trigFlag >= 0x8 && trigFlag <= 0xb;
        if ( logInfo ) {
          if ( isBeam ) cout << "Beam";
          else if ( isCrt ) cout << "CRT";
          else if ( isFake ) cout << "Fake";
          else cout << "Unexpected";
          cout << ")" << endl;
        }
      }
    } else {
      if ( logInfo ) {
        cout << myname << "WARNING: Event timing clocks product not found." << endl;
      }
    }
  }

  // Fetch beam information
  float beamTof = 0.0;    // Time of flight.
  if ( m_BeamEventLabel.size() ) {
    art::Handle< std::vector<beam::ProtoDUNEBeamEvent> > pdbeamHandle;
    std::vector< art::Ptr<beam::ProtoDUNEBeamEvent> > beaminfo;
    if ( evt.getByLabel(m_BeamEventLabel, pdbeamHandle) ) {
      art::fill_ptr_vector(beaminfo, pdbeamHandle);
      if ( beaminfo.size() == 0 ) {
        cout << myname << "WARNING: Beam event vector is empty." << endl;
      } else {
        if ( beaminfo.size() > 1 ) {
          cout << myname << "WARNING: Beam event vector has size " << beaminfo.size() << endl;
        }
        AdcIndex beamTrigFlag = beaminfo[0]->GetTimingTrigger();
        if ( beamTrigFlag != trigFlag ) {
          if ( logInfo ) cout << myname << "Beam event and timing trigger flags differ: " << beamTrigFlag << " != " << trigFlag << endl;
        } else if ( beamTrigFlag != 12 ) {
          if ( logInfo ) cout << myname << "Beam event trigger is not beam: it is " << beamTrigFlag << endl;
        //} else if ( ! beaminfo[0]->CheckIsMatched() ) {
        //  cout << myname << "Beam event is not matched." << endl;
        } else if ( beaminfo[0]->GetTOFChan() == -1 ) {
          if ( logInfo ) cout << myname << "Beam event index does not indicate match." << endl;
        } else {
          int beamChan = beaminfo[0]->GetTOFChan();
          beamTof = beaminfo[0]->GetTOF();
          if ( logInfo ) cout << myname << "Beam event TOF[" << beamChan << "]: " << beamTof << endl;
        }
      }
    } else {
      if ( logInfo ) cout << myname << "Beam event data product not found: " << m_BeamEventLabel << endl;
    }
  }
            
  // Create containers for data to be written to the event data store.
  using DigitVector = std::vector<raw::RawDigit>;
  std::unique_ptr<DigitVector> pdigitsAll;
  if ( m_maxOutputDigitChannelCount ) {
    pdigitsAll.reset(new DigitVector);
    pdigitsAll->reserve(m_maxOutputDigitChannelCount);
  }
  std::unique_ptr<TimeStampVector> ptimsAll;
  if ( m_maxOutputTimeStampChannelCount ) {
    ptimsAll.reset(new TimeStampVector);
    ptimsAll->reserve(m_maxOutputTimeStampChannelCount);
  }
  using WireVector = std::vector<recob::Wire>;
  std::unique_ptr<std::vector<recob::Wire>> pwires;
  if ( m_maxOutputWireChannelCount ) {
    pwires.reset(new WireVector);
    pwires->reserve(m_maxOutputWireChannelCount);
  }

  // Notify data preparation service of start of event.
  DuneEventInfo devt;
  devt.run = evt.run();
  devt.subRun = evt.subRun();
  devt.event = evt.event();
  devt.triggerClock = timingClock;
  int bstat = m_pRawDigitPrepService->beginEvent(devt);
  if ( bstat ) cout << myname << "WARNING: Event initialization failed." << endl;

  // Loop over channel ranges.
  DuneToolManager* ptm = DuneToolManager::instance();
  if ( ptm == nullptr ) {
    cout << myname << "ERROR: Tool manager not found." << endl;
    return;
  }
  const IndexRangeTool* pcrt = ptm->getShared<IndexRangeTool>("channelRanges");
  if ( pcrt == nullptr ) {
    cout << myname << "ERROR: Channel range tool channelRanges not found." << endl;
    return;
  }
  // Loop over APAs or all.
  for ( const ApaChannelSetMap::value_type& csmPair : m_apachsets ) {
    int iapa = csmPair.first;
    ostringstream ssapa;
    if ( iapa == -1 ) {
      ssapa << "All";
    } else {
      ssapa << "APA " << iapa;
    }
    Name sapa = ssapa.str();
    std::vector<int> apas = {iapa};
    // Fetch the digits, clocks and status for the channel range.
    if ( logInfo ) cout << myname << "Fetching digits and clocks for " << sapa << "." << endl;
    using StatVector  = std::vector<raw::RDStatus>;
    TimeStampVector timsCrn;
    StatVector statsCrn;
    DigitVector digitsCrn;
    // Fetch the data for this APA.
    int decodeStat = m_pDecoderTool->
      retrieveDataForSpecifiedAPAs(evt, digitsCrn, timsCrn, statsCrn, apas);
    if ( m_LogLevel >= 3 ) {    // Decoder tool can return any value for success 
      cout << myname << "WARNING: Decoder tool for APA " << iapa << " returned " << decodeStat << endl;
    }
    if ( logInfo ) {
      cout << myname << "  " << sapa << " digit count from tool: " << digitsCrn.size() << endl;
      cout << myname << "  " << sapa << " stats count from tool: " << statsCrn.size() << endl;
      cout << myname << "  " << sapa << " clock count from tool: " << timsCrn.size() << endl;
    }
    // Check read status.
    string srdstat;
    bool skipEvent = skipAllEvents;
    if ( statsCrn.size() != 1 ) cout << myname << "WARNING: Read status has unexpected size: "
                                     << statsCrn.size() << endl;
    const RDStatus rdstat = statsCrn.at(0);
    if ( false ) { 
      cout << myname << "Raw data status: " << rdstat.GetStatWord();
      if ( rdstat.GetCorruptDataDroppedFlag() ) cout << " (Corrupt data was dropped.)";
      if ( rdstat.GetCorruptDataKeptFlag() ) cout << " (Corrupt data was retained.)";
      cout << endl;
    }
    cout << myname << "Raw data read status: " << std::to_string(rdstat.GetStatWord()) << endl;
    srdstat = "rdstat=" + std::to_string(rdstat.GetStatWord());
    skipEvent |= skipEventsWithCorruptDataDropped && rdstat.GetCorruptDataDroppedFlag();
    // Check the channel clocks from the tool.
    vector<AdcLongIndex> channelClocks;
    vector<ULong64_t> tzeroClockCandidates;   // Candidates for t0 = 0.
    float trigTickOffset = -500.5;
    using ClockCounter = std::map<ULong64_t, AdcIndex>;
    ClockCounter clockCounts;
    ULong64_t chClock = 0;
    long chClockDiff = 0;
    float tickdiff = 0.0;
    for ( raw::RDTimeStamp chts : timsCrn ) {
      chClock = chts.GetTimeStamp();
      channelClocks.push_back(chClock);
      chClockDiff = chClock > timingClock ?  (chClock - timingClock)
                                               : -(timingClock - chClock);
      bool nearTrigger = fabs(tickdiff - trigTickOffset) < 1.0;
      tickdiff = chClockDiff/25.0;
      if ( clockCounts.find(chClock) == clockCounts.end() ) {
        clockCounts[chClock] = 1;
        if ( nearTrigger ) tzeroClockCandidates.push_back(chClock);
      } else {
        ++clockCounts[chClock];
      }
      if ( ! nearTrigger ) {
        if ( m_LogLevel >= 3 ) {
          cout << myname << "WARNING: Channel timing difference: " << chClockDiff
               << " (" << tickdiff << " ticks)." << endl;
        }
      }
    }
    if ( clockCounts.size() > 1 ) {
      if ( logInfo ) {
        cout << myname << "WARNING: Channel clocks for " << sapa << " are not consistent." << endl;
        cout << myname << "WARNING:     Clock     ticks   count" << endl;
      }
      for ( ClockCounter::value_type iclk : clockCounts ) {
        ULong64_t chClock = iclk.first;
        AdcIndex count = iclk.second;
        long chClockDiff = chClock > timingClock ?  (chClock - timingClock)
                                                 : -(timingClock - chClock);
        float tickdiff = chClockDiff/25.0;
        if ( logInfo ) {
          cout << myname << "WARNING:" << setw(10) << chClockDiff << setw(10) << tickdiff
               << setw(8) << count << endl;
        }
      }
    } else {
      if ( logInfo ) cout << myname << "Channel counts for " << sapa
                          << " are consistent with an offset of "
                          << tickdiff << " ticks." << endl;
    }
    // Build the AdcChannelData objects.
    AdcChannelDataMap datamap;
    if ( m_LogLevel >= 3 ) {
      cout << myname << "# digits read for " << sapa << ": " << digitsCrn.size() << endl;
    }
    // Create the transient data map and copy the digits there.
    const ApaChannelSet& csmKeepChans = csmPair.second;
    unsigned int nproc = 0;
    unsigned int nkeep = 0;
    unsigned int nskip = 0;
    unsigned int ndigi = digitsCrn.size();
    for ( unsigned int idig=0; idig<ndigi; ++idig ) {
      raw::RawDigit& dig = digitsCrn[idig];
      AdcChannel chan = dig.Channel();
      if ( csmKeepChans.count(chan) == 0 ) {
        ++nskip;
        continue;
      }
      // Create AdcChannelData for this channel.
      auto its = datamap.emplace(chan, AdcChannelData());
      if ( ! its.second ) {
        cout << myname << "ERROR: Ignoring duplicate digit for channel " << chan << "." << endl;
        ++nskip;
        continue;
      }
      AdcChannelData& acd = its.first->second;
      ++nproc;
      // Fetch the channel status.
      Index chanStat = AdcChannelStatusGood;
      if ( m_pChannelStatusProvider != nullptr ) {
        if ( m_pChannelStatusProvider->IsNoisy(chan) ) chanStat = AdcChannelStatusNoisy;
        if ( m_pChannelStatusProvider->IsBad(chan)   ) chanStat = AdcChannelStatusBad;
      }
      // Fetch the online ID.
      bool haveFemb = false;
      AdcChannel fembID = -1;
      AdcChannel fembChannel = -1;
      if ( m_onlineChannelMapTool ) {
        unsigned int ichOn = m_onlineChannelMapTool->get(chan);
        if ( ichOn != IndexMapTool::badIndex() ) {
          fembID = ichOn/128;
          fembChannel = ichOn % 128;
          haveFemb = true;
        }
      }
      // Build the channel data.
      acd.run = evt.run();
      acd.subRun = evt.subRun();
      acd.event = evt.event();
      acd.time = itim;
      acd.timerem = itimrem;
      acd.channel = chan;
      acd.channelStatus = chanStat;
      acd.digitIndex = idig;
      acd.digit = &dig;
      if ( haveFemb ) {
        acd.fembID = fembID;
        acd.fembChannel = fembChannel;
      }
      acd.triggerClock = timingClock;
      acd.trigger = trigFlag;
      if ( channelClocks.size() > idig ) acd.channelClock = channelClocks[idig];
      acd.metadata["ndigi"] = ndigi;
      if ( m_BeamEventLabel.size() ) {
        acd.metadata["beamTof"] = beamTof;
      }
      ++nkeep;
    }
    if ( logInfo ) {
      cout << myname << "              " << sapa << " # input digits: " << ndigi << endl;
      cout << myname << "         " << sapa << " # channels selected: " << nkeep << endl;
      cout << myname << "          " << sapa << " # channels skipped: " << nskip << endl;
      cout << myname << "  " << sapa << " # channels to be processed: " << nproc << endl;
    }

    Index nacd = datamap.size();
    if ( nacd == 0 ) {
      if ( m_LogLevel >= 3 ) {
        if ( logInfo ) cout << myname << "Skipping empty data map." << endl;
      }
      continue;
    }

    // Make sure created wires are deleted or stored in event.
    WireVector wiresTmp;
    WireVector* pwiresCrn = pwires ? pwires.get() : &wiresTmp;

    // Use the data preparation service to build the wires.
    if ( m_LogLevel >= 3 ) {
      cout << myname << "Preparing " << nacd << " channel" << (nacd == 1 ? "" : "s") << "." << endl;
    }
    int rstat = m_pRawDigitPrepService->prepare(datamap, pwiresCrn, nullptr);
    if ( rstat != 0 ) {
      cout << myname << "ERROR: Data preparation service returned error " << rstat;
      continue;
    }

    // Transfer the larsoft digits to the output container.
    if ( pdigitsAll ) for ( raw::RawDigit& dig : digitsCrn ) pdigitsAll->emplace_back(dig);
    if ( ptimsAll ) for ( raw::RDTimeStamp& tst : timsCrn ) ptimsAll->emplace_back(tst);

  }  // End loop over channel ranges.

  // Notify data preparation service of end of event.
  int estat = m_pRawDigitPrepService->endEvent(devt);
  if ( estat ) cout << myname << "WARNING: Event finalization failed." << endl;

  // Record wires and associations in the event.
  if ( pwires ) {
    if ( logInfo ) cout << myname << "Created wire count: " << pwires->size() << endl;
    if ( pwires->size() == 0 ) cout << myname << "WARNING: No wires made for this event." << endl;
    evt.put(std::move(pwires), m_OutputWireName);
  } else {
    if ( logInfo ) cout << myname << "Wire output was not requested." << endl;
  }

  // Record decoder containers.
  if ( m_OutputDigitName.size() ) {
    if ( logInfo ) cout << myname << "Created digit count: " << pdigitsAll->size() << endl;
    evt.put(std::move(pdigitsAll), m_OutputDigitName);
  } else {
    if ( logInfo ) cout << myname << "Digit output was not requested." << endl;
  }

  if ( m_OutputTimeStampName.size() ) {
    if ( logInfo ) cout << myname << "Created time stamp count: " << ptimsAll->size() << endl;
    evt.put(std::move(ptimsAll), m_OutputTimeStampName);
  } else {
    if ( logInfo ) cout << myname << "Time stamp output was not requested." << endl;
  }

  ++m_nproc;
  return;
}
