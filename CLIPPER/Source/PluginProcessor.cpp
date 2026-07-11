#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
ClipperProcessor::ClipperProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), apvts(*this,nullptr,"PARAMETERS",Params::createLayout()){}
void ClipperProcessor::prepareToPlay(double sr,int){ fx.prepare(sr); }
bool ClipperProcessor::isBusesLayoutSupported(const BusesLayout& l) const{ const auto& o=l.getMainOutputChannelSet(); if(o!=juce::AudioChannelSet::mono()&&o!=juce::AudioChannelSet::stereo())return false; return l.getMainInputChannelSet()==o; }
void ClipperProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){ juce::ScopedNoDenormals _nd;
  const auto tin=getTotalNumInputChannels(),tout=getTotalNumOutputChannels(); for(int c=tin;c<tout;++c) buffer.clear(c,0,buffer.getNumSamples());
  const int n=buffer.getNumSamples(); auto V=[&](const char* id){return apvts.getRawParameterValue(id)->load();}; (void)V;
  meters.pushInputPeak(buffer.getMagnitude(0,n));
  if(V("bypass")<0.5f){ fx.setMode(V("soft")>0.5f?aur::Clipper::Mode::Soft:aur::Clipper::Mode::Hard); fx.setParameters(V("drive"),std::pow(10.f,V("ceiling")/20.f),V("mix")); fx.process(buffer.getArrayOfWritePointers(),tout,n); }
  meters.pushOutputPeak(buffer.getMagnitude(0,n)); }
juce::AudioProcessorEditor* ClipperProcessor::createEditor(){return new ClipperEditor(*this);}
int ClipperProcessor::getNumPrograms(){return (int)Presets::getFactoryPresets().size();}
void ClipperProcessor::setCurrentProgram(int i){ if(i<0||i>=getNumPrograms())return; cur=i; Presets::apply(i,apvts);}
const juce::String ClipperProcessor::getProgramName(int i){ const auto& p=Presets::getFactoryPresets(); return (i>=0&&i<(int)p.size())?p[(size_t)i].name:juce::String{};}
void ClipperProcessor::getStateInformation(juce::MemoryBlock& d){ if(auto s=apvts.copyState(); s.isValid()){ juce::MemoryOutputStream m(d,false); s.writeToStream(m);} }
void ClipperProcessor::setStateInformation(const void* d,int s){ auto t=juce::ValueTree::readFromData(d,(size_t)s); if(t.isValid()) apvts.replaceState(t);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ClipperProcessor();}
