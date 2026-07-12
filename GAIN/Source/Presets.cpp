#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Unity", { {ParamID::gain,0.0f}, {ParamID::balance,0.0f}, {ParamID::width,100.0f} } },
            { "Trim -6", { {ParamID::gain,-6.0f}, {ParamID::balance,0.0f}, {ParamID::width,100.0f} } },
            { "Narrow", { {ParamID::gain,0.0f}, {ParamID::balance,0.0f}, {ParamID::width,60.0f} } },
            { "Wide", { {ParamID::gain,0.0f}, {ParamID::balance,0.0f}, {ParamID::width,150.0f} } },
            { "Boost +3", { {ParamID::gain,3.0f}, {ParamID::balance,0.0f}, {ParamID::width,100.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
