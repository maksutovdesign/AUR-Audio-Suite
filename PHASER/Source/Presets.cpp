#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Classic", { {ParamID::rate,0.4f}, {ParamID::depth,70.0f}, {ParamID::feedback,40.0f}, {ParamID::stages,4.0f}, {ParamID::mix,50.0f} } },
            { "Deep", { {ParamID::rate,0.3f}, {ParamID::depth,90.0f}, {ParamID::feedback,60.0f}, {ParamID::stages,6.0f}, {ParamID::mix,55.0f} } },
            { "Fast", { {ParamID::rate,2.0f}, {ParamID::depth,60.0f}, {ParamID::feedback,30.0f}, {ParamID::stages,4.0f}, {ParamID::mix,45.0f} } },
            { "Vintage", { {ParamID::rate,0.5f}, {ParamID::depth,80.0f}, {ParamID::feedback,50.0f}, {ParamID::stages,8.0f}, {ParamID::mix,50.0f} } },
            { "Subtle", { {ParamID::rate,0.6f}, {ParamID::depth,40.0f}, {ParamID::feedback,20.0f}, {ParamID::stages,2.0f}, {ParamID::mix,35.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
