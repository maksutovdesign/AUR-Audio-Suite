#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"
using namespace aur::ui;
MultiEditor::MultiEditor(MultiProcessor& p):AudioProcessorEditor(p),ap(p){ setLookAndFeel(&lnf); auto& a=ap.getAPVTS();
  knobs[0]=std::make_unique<LabeledKnob>(a,ParamID::xlo,"X-LO"); addAndMakeVisible(*knobs[0]);
  knobs[1]=std::make_unique<LabeledKnob>(a,ParamID::thrL,"LO THR"); addAndMakeVisible(*knobs[1]);
  knobs[2]=std::make_unique<LabeledKnob>(a,ParamID::ratL,"LO RAT"); addAndMakeVisible(*knobs[2]);
  knobs[3]=std::make_unique<LabeledKnob>(a,ParamID::thrM,"MD THR"); addAndMakeVisible(*knobs[3]);
  knobs[4]=std::make_unique<LabeledKnob>(a,ParamID::ratM,"MD RAT"); addAndMakeVisible(*knobs[4]);
  knobs[5]=std::make_unique<LabeledKnob>(a,ParamID::thrH,"HI THR"); addAndMakeVisible(*knobs[5]);
  knobs[6]=std::make_unique<LabeledKnob>(a,ParamID::ratH,"HI RAT"); addAndMakeVisible(*knobs[6]);
  knobs[7]=std::make_unique<LabeledKnob>(a,ParamID::xhi,"X-HI"); addAndMakeVisible(*knobs[7]);
  addAndMakeVisible(presetBox); refreshPresetBox(); presetBox.onChange=[this]{ const auto i=presetBox.getSelectedId()-1; if(i>=0) ap.setCurrentProgram(i);};
  bypassButton.setClickingTogglesState(true); addAndMakeVisible(bypassButton);
  bypassAtt=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(a,ParamID::bypass,bypassButton);
  addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);
  setSize(1060,400);
}
MultiEditor::~MultiEditor(){ setLookAndFeel(nullptr);}
void MultiEditor::refreshPresetBox(){ presetBox.clear(juce::dontSendNotification); int id=1; for(const auto& pr:Presets::getFactoryPresets()) presetBox.addItem(pr.name,id++); presetBox.setSelectedId(ap.getCurrentProgram()+1,juce::dontSendNotification);}
void MultiEditor::paint(juce::Graphics& g){ const auto& t=theme(); g.fillAll(t.ground);
  const auto cx=(float)getWidth()*0.5f; juce::ColourGradient gl(t.accent.withAlpha(0.12f),cx,150.0f,t.ground.withAlpha(0.0f),cx,320.0f,true); g.setGradientFill(gl); g.fillRect(getLocalBounds());
  drawBrandHeader(g,{20,16,360,60},"MULTI","3-band compressor"); }
void MultiEditor::resized(){ auto area=getLocalBounds().reduced(18); auto header=area.removeFromTop(56); header.removeFromLeft(180);
  bypassButton.setBounds(header.removeFromRight(88).reduced(4,12));
  for(auto& bt:toggleBtns) bt->setBounds(header.removeFromRight(90).reduced(4,12));
  presetBox.setBounds(header.removeFromRight(130).reduced(4,14));
  area.removeFromTop(6); auto mc=area.removeFromRight(76); auto mi=mc.reduced(6); const auto mw=mi.getWidth()/2; inMeter.setBounds(mi.removeFromLeft(mw).reduced(4)); outMeter.setBounds(mi.reduced(4)); area.removeFromRight(8);
  auto row=area; const int kw=row.getWidth()/8; for(auto& k:knobs) k->setBounds(row.removeFromLeft(kw).reduced(8)); }
