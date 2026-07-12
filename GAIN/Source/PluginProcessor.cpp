#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
GainProcessor::GainProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), apvts(*this,nullptr,"PARAMETERS",Params::createLayout()){}
void GainProcessor::prepareToPlay(double sr,int spb){ (void)spb; fx.prepare(sr,getTotalNumOutputChannels()); }
bool GainProcessor::isBusesLayoutSupported(const BusesLayout& l) const{ const auto& o=l.getMainOutputChannelSet(); if(o!=juce::AudioChannelSet::mono()&&o!=juce::AudioChannelSet::stereo())return false; return l.getMainInputChannelSet()==o; }
void GainProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){ juce::ScopedNoDenormals _nd;
  const auto tin=getTotalNumInputChannels(),tout=getTotalNumOutputChannels(); for(int c=tin;c<tout;++c) buffer.clear(c,0,buffer.getNumSamples());
  const int n=buffer.getNumSamples(); auto V=[&](const char* id){return apvts.getRawParameterValue(id)->load();}; (void)V;
  meters.pushInputPeak(buffer.getMagnitude(0,n));
  if(V("bypass")<0.5f){ fx.setParameters(V("gain"),V("balance"),V("width"),V("mono")>0.5f,V("phase")>0.5f); fx.process(buffer.getArrayOfWritePointers(),tout,n); }
  meters.pushOutputPeak(buffer.getMagnitude(0,n)); }
juce::AudioProcessorEditor* GainProcessor::createEditor(){return new GainEditor(*this);}
int GainProcessor::getNumPrograms(){return (int)Presets::getFactoryPresets().size();}
void GainProcessor::setCurrentProgram(int i){ if(i<0||i>=getNumPrograms())return; cur=i; Presets::apply(i,apvts);}
const juce::String GainProcessor::getProgramName(int i){ const auto& p=Presets::getFactoryPresets(); return (i>=0&&i<(int)p.size())?p[(size_t)i].name:juce::String{};}
void GainProcessor::getStateInformation(juce::MemoryBlock& d){ if(auto s=apvts.copyState(); s.isValid()){ juce::MemoryOutputStream m(d,false); s.writeToStream(m);} }
void GainProcessor::setStateInformation(const void* d,int s){ auto t=juce::ValueTree::readFromData(d,(size_t)s); if(t.isValid()) apvts.replaceState(t);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new GainProcessor();}
