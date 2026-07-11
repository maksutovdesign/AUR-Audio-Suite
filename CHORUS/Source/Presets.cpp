#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Subtle", { {ParamID::rate,0.6f}, {ParamID::depth,3.0f}, {ParamID::feedback,8.0f}, {ParamID::mix,30.0f} } },
            { "Lush", { {ParamID::rate,0.9f}, {ParamID::depth,6.0f}, {ParamID::feedback,15.0f}, {ParamID::mix,55.0f} } },
            { "Wide", { {ParamID::rate,0.4f}, {ParamID::depth,8.0f}, {ParamID::feedback,5.0f}, {ParamID::mix,50.0f} } },
            { "Vibrato", { {ParamID::rate,5.0f}, {ParamID::depth,4.0f}, {ParamID::feedback,0.0f}, {ParamID::mix,100.0f} } },
            { "Shimmer", { {ParamID::rate,1.4f}, {ParamID::depth,5.0f}, {ParamID::feedback,20.0f}, {ParamID::mix,45.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
