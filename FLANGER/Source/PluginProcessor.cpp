#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
FlangerProcessor::FlangerProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), apvts(*this,nullptr,"PARAMETERS",Params::createLayout()){}
void FlangerProcessor::prepareToPlay(double sr,int){ fx.prepare(sr,3.0,14.0); }
bool FlangerProcessor::isBusesLayoutSupported(const BusesLayout& l) const{ const auto& o=l.getMainOutputChannelSet(); if(o!=juce::AudioChannelSet::mono()&&o!=juce::AudioChannelSet::stereo())return false; return l.getMainInputChannelSet()==o; }
void FlangerProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){ juce::ScopedNoDenormals _nd;
  const auto tin=getTotalNumInputChannels(),tout=getTotalNumOutputChannels(); for(int c=tin;c<tout;++c) buffer.clear(c,0,buffer.getNumSamples());
  const int n=buffer.getNumSamples(); auto V=[&](const char* id){return apvts.getRawParameterValue(id)->load();}; (void)V;
  meters.pushInputPeak(buffer.getMagnitude(0,n));
  if(V("bypass")<0.5f){ fx.setParameters(V("rate"),V("depth"),V("mix"),V("feedback")); if(tout>1) fx.process(buffer.getWritePointer(0),buffer.getWritePointer(1),n); }
  meters.pushOutputPeak(buffer.getMagnitude(0,n)); }
juce::AudioProcessorEditor* FlangerProcessor::createEditor(){return new FlangerEditor(*this);}
int FlangerProcessor::getNumPrograms(){return (int)Presets::getFactoryPresets().size();}
void FlangerProcessor::setCurrentProgram(int i){ if(i<0||i>=getNumPrograms())return; cur=i; Presets::apply(i,apvts);}
const juce::String FlangerProcessor::getProgramName(int i){ const auto& p=Presets::getFactoryPresets(); return (i>=0&&i<(int)p.size())?p[(size_t)i].name:juce::String{};}
void FlangerProcessor::getStateInformation(juce::MemoryBlock& d){ if(auto s=apvts.copyState(); s.isValid()){ juce::MemoryOutputStream m(d,false); s.writeToStream(m);} }
void FlangerProcessor::setStateInformation(const void* d,int s){ auto t=juce::ValueTree::readFromData(d,(size_t)s); if(t.isValid()) apvts.replaceState(t);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new FlangerProcessor();}
