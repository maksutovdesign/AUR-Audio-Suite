#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Transparent", { {ParamID::drive,10.0f}, {ParamID::ceiling,-0.3f}, {ParamID::mix,100.0f} } },
            { "Loud", { {ParamID::drive,45.0f}, {ParamID::ceiling,-0.1f}, {ParamID::mix,100.0f} } },
            { "Crunch", { {ParamID::drive,70.0f}, {ParamID::ceiling,-1.0f}, {ParamID::mix,100.0f} } },
            { "Parallel", { {ParamID::drive,80.0f}, {ParamID::ceiling,-2.0f}, {ParamID::mix,40.0f} } },
            { "Safety", { {ParamID::drive,0.0f}, {ParamID::ceiling,-0.3f}, {ParamID::mix,100.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
