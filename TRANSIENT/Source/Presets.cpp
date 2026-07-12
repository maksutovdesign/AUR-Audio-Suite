#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Punch", { {ParamID::attack,40.0f}, {ParamID::sustain,-10.0f} } },
            { "Snap", { {ParamID::attack,70.0f}, {ParamID::sustain,-30.0f} } },
            { "Fat", { {ParamID::attack,-10.0f}, {ParamID::sustain,40.0f} } },
            { "Soften", { {ParamID::attack,-40.0f}, {ParamID::sustain,20.0f} } },
            { "Room Kill", { {ParamID::attack,20.0f}, {ParamID::sustain,-60.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
