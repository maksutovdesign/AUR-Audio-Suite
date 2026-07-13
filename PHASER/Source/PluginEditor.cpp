#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"
using namespace aur::ui;
PhaserEditor::PhaserEditor(PhaserProcessor& p):AudioProcessorEditor(p),ap(p){ setLookAndFeel(&lnf); auto& a=ap.getAPVTS();
  knobs[0]=std::make_unique<LabeledKnob>(a,ParamID::rate,"RATE"); addAndMakeVisible(*knobs[0]);
  knobs[1]=std::make_unique<LabeledKnob>(a,ParamID::depth,"DEPTH"); addAndMakeVisible(*knobs[1]);
  knobs[2]=std::make_unique<LabeledKnob>(a,ParamID::feedback,"FDBK"); addAndMakeVisible(*knobs[2]);
  knobs[3]=std::make_unique<LabeledKnob>(a,ParamID::stages,"STAGES"); addAndMakeVisible(*knobs[3]);
  knobs[4]=std::make_unique<LabeledKnob>(a,ParamID::mix,"MIX"); addAndMakeVisible(*knobs[4]);
  addAndMakeVisible(presetBox); refreshPresetBox(); presetBox.onChange=[this]{ const auto i=presetBox.getSelectedId()-1; if(i>=0) ap.setCurrentProgram(i);};
  bypassButton.setClickingTogglesState(true); addAndMakeVisible(bypassButton);
  bypassAtt=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(a,ParamID::bypass,bypassButton);
  addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);
  setSize(750,400);
}
PhaserEditor::~PhaserEditor(){ setLookAndFeel(nullptr);}
void PhaserEditor::applyThemeChoice(int i){ switch(i){case 1: aur::ui::setTheme(aur::ui::obsidianTheme());break; case 2: aur::ui::setTheme(aur::ui::fluxTheme());break; default: aur::ui::setTheme(aur::ui::moltenTheme());} lnf.applyTheme(); sendLookAndFeelChange(); repaint(); }
void PhaserEditor::refreshPresetBox(){ presetBox.clear(juce::dontSendNotification); int id=1; for(const auto& pr:Presets::getFactoryPresets()) presetBox.addItem(pr.name,id++); presetBox.setSelectedId(ap.getCurrentProgram()+1,juce::dontSendNotification);}
void PhaserEditor::paint(juce::Graphics& g){ const auto& t=theme(); g.fillAll(t.ground);
  const auto cx=(float)getWidth()*0.5f; juce::ColourGradient gl(t.accent.withAlpha(0.12f),cx,150.0f,t.ground.withAlpha(0.0f),cx,320.0f,true); g.setGradientFill(gl); g.fillRect(getLocalBounds());
  drawBrandHeader(g,{20,16,360,60},"PHASER","Analog phaser"); }
void PhaserEditor::resized(){ auto area=getLocalBounds().reduced(18); auto header=area.removeFromTop(56); header.removeFromLeft(180);
  bypassButton.setBounds(header.removeFromRight(88).reduced(4,12));
  for(auto& bt:toggleBtns) bt->setBounds(header.removeFromRight(90).reduced(4,12));
  presetBox.setBounds(header.removeFromRight(130).reduced(4,14));
  area.removeFromTop(6); auto mc=area.removeFromRight(76); auto mi=mc.reduced(6); const auto mw=mi.getWidth()/2; inMeter.setBounds(mi.removeFromLeft(mw).reduced(4)); outMeter.setBounds(mi.reduced(4)); area.removeFromRight(8);
  auto row=area; const int kw=row.getWidth()/5; for(auto& k:knobs) k->setBounds(row.removeFromLeft(kw).reduced(8)); }
