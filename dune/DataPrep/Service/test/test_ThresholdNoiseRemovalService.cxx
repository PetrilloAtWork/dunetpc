// test_ThresholdNoiseRemovalService.cxx
//
// David Adams
// May 2016
//
// Test ThresholdNoiseRemovalService.
//

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "dune/ArtSupport/ArtServiceHelper.h"
#include "dune/DuneInterface/AdcChannelNoiseRemovalService.h"

#undef NDEBUG
#include <cassert>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::istringstream;
using std::ofstream;
using std::setw;
using std::setprecision;
using std::fixed;
using art::ServiceHandle;

//**********************************************************************

int test_ThresholdNoiseRemovalService(int a_LogLevel =1) {
  const string myname = "test_ThresholdNoiseRemovalService: ";
#ifdef NDEBUG
  cout << myname << "NDEBUG must be off." << endl;
  abort();
#endif
  string line = "-----------------------------";

  cout << myname << line << endl;
  cout << myname << "Create top-level FCL." << endl;
  AdcSignal threshold = 4.0;
  std::ostringstream oss;
  oss << "services.AdcChannelNoiseRemovalService: {" << endl;
  oss << "  service_provider: ThresholdNoiseRemovalService" << endl;
  oss << "  Threshold:    " << threshold <<  endl;
  oss << "  LogLevel:       " << a_LogLevel << endl;
  oss << "}" << endl;
  ArtServiceHelper::load_services(oss.str());

  const unsigned int nsig = 100;
  AdcChannelData acd;
  acd.channel = 100;
  for ( unsigned int isig=0; isig<nsig; ++isig ) {
    acd.samples.push_back(20*sin(0.47*isig));
    acd.flags.push_back(AdcGood);
  }
  AdcSignalVector& sigs = acd.samples;
  AdcSignalVector sigsin = sigs;
  AdcSignalVector sigsexp = sigs;
  for ( AdcSignal& sig : sigsexp ) if ( fabs(sig) < threshold ) sig = 0.0;

  cout << myname << line << endl;
  cout << myname << "Fetch the noise removal." << endl;
  ServiceHandle<AdcChannelNoiseRemovalService> hanr;
  hanr->print(cout, myname);

  cout << myname << line << endl;
  cout << myname << "Remove noise." << endl;
  assert( hanr->update(acd) == 0 );
  for ( unsigned int isig=0; isig<nsig; ++isig ) {
    cout << myname << setw(4) << isig << ": "
         << fixed << setprecision(1) << setw(10) << sigsin[isig]
         << fixed << setprecision(1) << setw(10) << sigs[isig]
         << endl;
    assert( sigs[isig] == sigsexp[isig] );
  }

  cout << myname << line << endl;
  cout << myname << "Done." << endl;
  return 0;
}

//**********************************************************************

int main(int argc, char* argv[]) {
  int a_LogLevel = -1;
  if ( argc > 1 ) {
    istringstream ssarg(argv[1]);
    ssarg >> a_LogLevel;
  }
  return test_ThresholdNoiseRemovalService(a_LogLevel);
}

//**********************************************************************
