#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Jet", { {ParamID::rate,0.3f}, {ParamID::depth,3.0f}, {ParamID::feedback,80.0f}, {ParamID::mix,50.0f} } },
            { "Soft", { {ParamID::rate,0.5f}, {ParamID::depth,2.0f}, {ParamID::feedback,40.0f}, {ParamID::mix,40.0f} } },
            { "Metallic", { {ParamID::rate,0.2f}, {ParamID::depth,4.0f}, {ParamID::feedback,90.0f}, {ParamID::mix,55.0f} } },
            { "Fast", { {ParamID::rate,2.5f}, {ParamID::depth,2.0f}, {ParamID::feedback,50.0f}, {ParamID::mix,45.0f} } },
            { "Wide", { {ParamID::rate,0.4f}, {ParamID::depth,3.5f}, {ParamID::feedback,60.0f}, {ParamID::mix,50.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
