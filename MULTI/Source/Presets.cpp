#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Master Glue", { {ParamID::xlo,180.0f}, {ParamID::xhi,3000.0f}, {ParamID::thrL,-16.0f}, {ParamID::ratL,2.0f}, {ParamID::thrM,-14.0f}, {ParamID::ratM,2.0f}, {ParamID::thrH,-16.0f}, {ParamID::ratH,2.0f} } },
            { "Punchy", { {ParamID::xlo,150.0f}, {ParamID::xhi,2500.0f}, {ParamID::thrL,-20.0f}, {ParamID::ratL,4.0f}, {ParamID::thrM,-18.0f}, {ParamID::ratM,3.0f}, {ParamID::thrH,-18.0f}, {ParamID::ratH,3.0f} } },
            { "Tame Highs", { {ParamID::xlo,250.0f}, {ParamID::xhi,4000.0f}, {ParamID::thrL,-40.0f}, {ParamID::ratL,1.0f}, {ParamID::thrM,-30.0f}, {ParamID::ratM,2.0f}, {ParamID::thrH,-20.0f}, {ParamID::ratH,5.0f} } },
            { "Bass Tight", { {ParamID::xlo,120.0f}, {ParamID::xhi,2500.0f}, {ParamID::thrL,-24.0f}, {ParamID::ratL,6.0f}, {ParamID::thrM,-40.0f}, {ParamID::ratM,1.0f}, {ParamID::thrH,-40.0f}, {ParamID::ratH,1.0f} } },
            { "Gentle", { {ParamID::xlo,200.0f}, {ParamID::xhi,3000.0f}, {ParamID::thrL,-14.0f}, {ParamID::ratL,1.5f}, {ParamID::thrM,-14.0f}, {ParamID::ratM,1.5f}, {ParamID::thrH,-14.0f}, {ParamID::ratH,1.5f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
