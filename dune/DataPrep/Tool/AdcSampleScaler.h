// AdcSampleScaler.h
//
// Tool to scale the samples in AdcSampleData.
//
// Configuration:
//   LogLevel: 0=silent, 1=init, 2=each event, >2=more
//   ScaleFactor: Factor by which samples are scaled.
//   OutputUnit: Value assigned for sampleUnit.
//   InputUnit: If non-blank, then this is compared with sampleUnit and a warning
//               broadcast if they differ.

#ifndef AdcSampleScaler_H
#define AdcSampleScaler_H

#include "art/Utilities/ToolMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "dune/DuneInterface/Tool/AdcChannelTool.h"

class AdcSampleScaler : AdcChannelTool {

public:

  AdcSampleScaler(fhicl::ParameterSet const& ps);

  ~AdcSampleScaler() override =default;

  DataMap update(AdcChannelData& acd) const override;

private:

  // Configuration data.
  int                m_LogLevel;
  float              m_ScaleFactor;
  std::string        m_OutputUnit;
  std::string        m_InputUnit;

};

DEFINE_ART_CLASS_TOOL(AdcSampleScaler)

#endif
