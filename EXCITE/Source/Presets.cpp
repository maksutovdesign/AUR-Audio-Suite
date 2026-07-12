#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Air", { {ParamID::freq,8000.0f}, {ParamID::amount,35.0f}, {ParamID::mix,30.0f} } },
            { "Presence", { {ParamID::freq,4000.0f}, {ParamID::amount,45.0f}, {ParamID::mix,35.0f} } },
            { "Sheen", { {ParamID::freq,10000.0f}, {ParamID::amount,30.0f}, {ParamID::mix,25.0f} } },
            { "Aggressive", { {ParamID::freq,3000.0f}, {ParamID::amount,70.0f}, {ParamID::mix,45.0f} } },
            { "Subtle", { {ParamID::freq,6000.0f}, {ParamID::amount,25.0f}, {ParamID::mix,20.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
