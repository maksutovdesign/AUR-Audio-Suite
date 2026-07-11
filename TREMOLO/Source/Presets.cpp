#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Gentle", { {ParamID::rate,4.0f}, {ParamID::depth,40.0f}, {ParamID::stereo,0.0f} } },
            { "Choppy", { {ParamID::rate,8.0f}, {ParamID::depth,90.0f}, {ParamID::stereo,0.0f} } },
            { "Auto-Pan", { {ParamID::rate,2.0f}, {ParamID::depth,80.0f}, {ParamID::stereo,100.0f} } },
            { "Slow", { {ParamID::rate,1.0f}, {ParamID::depth,50.0f}, {ParamID::stereo,0.0f} } },
            { "Wide Pan", { {ParamID::rate,3.0f}, {ParamID::depth,70.0f}, {ParamID::stereo,100.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
