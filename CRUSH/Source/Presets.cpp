#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Lo-Fi 8-bit", { {ParamID::bits,8.0f}, {ParamID::downsample,2.0f}, {ParamID::mix,100.0f} } },
            { "Telephone", { {ParamID::bits,6.0f}, {ParamID::downsample,6.0f}, {ParamID::mix,100.0f} } },
            { "Gritty", { {ParamID::bits,4.0f}, {ParamID::downsample,3.0f}, {ParamID::mix,80.0f} } },
            { "Subtle", { {ParamID::bits,12.0f}, {ParamID::downsample,1.0f}, {ParamID::mix,40.0f} } },
            { "Destroy", { {ParamID::bits,2.0f}, {ParamID::downsample,10.0f}, {ParamID::mix,100.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
