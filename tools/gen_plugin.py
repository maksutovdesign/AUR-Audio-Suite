#!/usr/bin/env python3
# Config-driven generator for AUR plugins (CMake + Parameters + Processor +
# Editor + Presets) that reuse the shared AurvedaDSP/AurvedaUI. Kept in the repo
# so it survives scratchpad wipes. Run from anywhere: python3 tools/gen_plugin.py
import os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def fl(x):
    s = str(x)
    return (s if ('.' in s or 'e' in s or 'E' in s) else s + '.0') + 'f'

CMAKE = '''cmake_minimum_required(VERSION 3.22)
project(%%NAME%% VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(%%NAME%%
    COMPANY_NAME "Aurveda Audio" BUNDLE_ID com.aurveda.aur.%%LOWER%%
    PLUGIN_MANUFACTURER_CODE Aurv PLUGIN_CODE %%CODE%%
    FORMATS AU VST3 Standalone AU_MAIN_TYPE kAudioUnitType_Effect
    PRODUCT_NAME "AUR %%NAME%%" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE COPY_PLUGIN_AFTER_BUILD TRUE)
juce_generate_juce_header(%%NAME%%)
target_sources(%%NAME%% PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/Presets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/AurLookAndFeel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/MeterComponent.cpp)
target_compile_definitions(%%NAME%% PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_include_directories(%%NAME%% PRIVATE Source ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaDSP ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI)
target_link_libraries(%%NAME%% PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
'''

def fmt_fn(kind):
    if kind == "pct": return 'AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})'
    if kind == "ms":  return 'AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" ms";})'
    if kind == "db":  return 'AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})'
    if kind == "hz":  return 'AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return v>=1000.f?String(v/1000.f,2)+" kHz":String(v,2)+" Hz";})'
    if kind == "int": return 'AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v);})'
    return 'AudioParameterFloatAttributes()'

def params_h(cfg):
    ids = "\n".join(['    static constexpr auto %s = "%s";' % (p["id"], p["id"]) for p in cfg["params"]] +
                    ['    static constexpr auto %s = "%s";' % (b["id"], b["id"]) for b in cfg.get("bools",[])] +
                    ['    static constexpr auto bypass = "bypass";'])
    body = ""
    for p in cfg["params"]:
        body += ('        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::%s,1},"%s",\n'
                 '            NormalisableRange<float>(%s,%s,%s%s),%s,%s));\n' %
                 (p["id"], p["name"], fl(p["min"]), fl(p["max"]), fl(p.get("step","0.01")),
                  (","+fl(p["skew"])) if "skew" in p else "", fl(p["def"]), fmt_fn(p["kind"])))
    for b in cfg.get("bools",[]):
        body += '        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::%s,1},"%s",%s));\n' % (b["id"], b["name"], "true" if b["def"] else "false")
    body += '        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));\n'
    return ('#pragma once\n#include <juce_audio_processors/juce_audio_processors.h>\nnamespace ParamID{\n%s\n}\n'
            'namespace Params{\n  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){\n'
            '    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;\n%s'
            '    return {p.begin(),p.end()};\n  }\n}\n') % (ids, body)

def presets_cpp(cfg):
    rows = ""
    for pr in cfg["presets"]:
        vals = ", ".join(['{ParamID::%s,%s}' % (k,fl(v)) for k,v in pr[1].items()])
        rows += '            { "%s", { %s } },\n' % (pr[0], vals)
    return ('#include "Presets.h"\n#include "Parameters.h"\nnamespace Presets{\n'
            '  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={\n%s'
            '  }; return p; }\n'
            '  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();\n'
            '    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }\n}\n') % rows

PRESETS_H = ('#pragma once\n#include <juce_audio_processors/juce_audio_processors.h>\n#include <vector>\n'
    'namespace Presets{ struct Preset{ juce::String name; std::vector<std::pair<juce::String,float>> values; };\n'
    '  const std::vector<Preset>& getFactoryPresets(); void apply(int,juce::AudioProcessorValueTreeState&); }\n')

def proc_h(cfg):
    return ('#pragma once\n#include <juce_audio_processors/juce_audio_processors.h>\n#include "Parameters.h"\n%s\n#include "Metering.h"\n'
        'class %sProcessor : public juce::AudioProcessor{\npublic:\n  %sProcessor();\n  ~%sProcessor() override=default;\n'
        '  void prepareToPlay(double,int) override; void releaseResources() override {}\n'
        '  bool isBusesLayoutSupported(const BusesLayout&) const override;\n  void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;\n'
        '  juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override {return true;}\n'
        '  const juce::String getName() const override {return JucePlugin_Name;}\n  bool acceptsMidi() const override {return false;} bool producesMidi() const override {return false;} bool isMidiEffect() const override {return false;} double getTailLengthSeconds() const override {return 0.0;}\n'
        '  int getNumPrograms() override; int getCurrentProgram() override {return cur;} void setCurrentProgram(int) override; const juce::String getProgramName(int) override; void changeProgramName(int,const juce::String&) override {}\n'
        '  void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;\n'
        '  juce::AudioProcessorValueTreeState& getAPVTS(){return apvts;} aur::MeterState& getMeterState(){return meters;}\nprivate:\n'
        '  juce::AudioProcessorValueTreeState apvts; aur::MeterState meters; %s int cur=0;\n'
        '  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(%sProcessor)\n};\n') % (
        "\n".join('#include "%s"' % i for i in cfg["includes"]), cfg["prefix"], cfg["prefix"], cfg["prefix"], cfg["members"], cfg["prefix"])

def proc_cpp(cfg):
    return ('#include "PluginProcessor.h"\n#include "PluginEditor.h"\n#include "Presets.h"\n'
        '%sProcessor::%sProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), apvts(*this,nullptr,"PARAMETERS",Params::createLayout()){}\n'
        'void %sProcessor::prepareToPlay(double sr,int spb){ (void)spb; %s }\n'
        'bool %sProcessor::isBusesLayoutSupported(const BusesLayout& l) const{ const auto& o=l.getMainOutputChannelSet(); if(o!=juce::AudioChannelSet::mono()&&o!=juce::AudioChannelSet::stereo())return false; return l.getMainInputChannelSet()==o; }\n'
        'void %sProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){ juce::ScopedNoDenormals _nd;\n'
        '  const auto tin=getTotalNumInputChannels(),tout=getTotalNumOutputChannels(); for(int c=tin;c<tout;++c) buffer.clear(c,0,buffer.getNumSamples());\n'
        '  const int n=buffer.getNumSamples(); auto V=[&](const char* id){return apvts.getRawParameterValue(id)->load();}; (void)V;\n'
        '  meters.pushInputPeak(buffer.getMagnitude(0,n));\n'
        '  if(V("bypass")<0.5f){ %s %s }\n'
        '  meters.pushOutputPeak(buffer.getMagnitude(0,n)); }\n'
        'juce::AudioProcessorEditor* %sProcessor::createEditor(){return new %sEditor(*this);}\n'
        'int %sProcessor::getNumPrograms(){return (int)Presets::getFactoryPresets().size();}\n'
        'void %sProcessor::setCurrentProgram(int i){ if(i<0||i>=getNumPrograms())return; cur=i; Presets::apply(i,apvts);}\n'
        'const juce::String %sProcessor::getProgramName(int i){ const auto& p=Presets::getFactoryPresets(); return (i>=0&&i<(int)p.size())?p[(size_t)i].name:juce::String{};}\n'
        'void %sProcessor::getStateInformation(juce::MemoryBlock& d){ if(auto s=apvts.copyState(); s.isValid()){ juce::MemoryOutputStream m(d,false); s.writeToStream(m);} }\n'
        'void %sProcessor::setStateInformation(const void* d,int s){ auto t=juce::ValueTree::readFromData(d,(size_t)s); if(t.isValid()) apvts.replaceState(t);}\n'
        'juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new %sProcessor();}\n') % (
        cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prepare"],cfg["prefix"],cfg["prefix"],cfg["setparams"],cfg["process"],
        cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"])

def editor_h(cfg):
    nk = len(cfg["knobs"])
    return ('#pragma once\n#include <juce_audio_processors/juce_audio_processors.h>\n#include "PluginProcessor.h"\n#include "AurLookAndFeel.h"\n#include "Knob.h"\n#include "MeterComponent.h"\n'
        'class %sEditor : public juce::AudioProcessorEditor{\npublic:\n  explicit %sEditor(%sProcessor&); ~%sEditor() override;\n  void paint(juce::Graphics&) override; void resized() override;\nprivate:\n'
        '  void refreshPresetBox();\n  %sProcessor& ap; aur::ui::AurLookAndFeel lnf;\n'
        '  std::array<std::unique_ptr<aur::ui::LabeledKnob>,%d> knobs;\n'
        '  std::vector<std::unique_ptr<juce::TextButton>> toggleBtns;\n'
        '  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> toggleAtt;\n'
        '  juce::ComboBox presetBox; juce::TextButton bypassButton{"BYPASS"};\n'
        '  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;\n'
        '  aur::ui::MeterComponent inMeter{ap.getMeterState(),aur::ui::MeterComponent::Which::input,"IN"};\n'
        '  aur::ui::MeterComponent outMeter{ap.getMeterState(),aur::ui::MeterComponent::Which::output,"OUT"};\n'
        '  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(%sEditor)\n};\n') % (
        cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],cfg["prefix"],nk,cfg["prefix"])

def editor_cpp(cfg):
    knob_init = ""
    for i,(pid,cap) in enumerate(cfg["knobs"]):
        knob_init += '  knobs[%d]=std::make_unique<LabeledKnob>(a,ParamID::%s,"%s"); addAndMakeVisible(*knobs[%d]);\n' % (i,pid,cap,i)
    tog_init = ""
    for b in cfg.get("bools",[]):
        tog_init += ('  { auto bt=std::make_unique<juce::TextButton>("%s"); bt->setClickingTogglesState(true); addAndMakeVisible(*bt);\n'
                     '    toggleAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(a,ParamID::%s,*bt)); toggleBtns.push_back(std::move(bt)); }\n') % (b["name"].upper(), b["id"])
    nk = len(cfg["knobs"]); w = cfg.get("width", 220+nk*105)
    T = ('#include "PluginEditor.h"\n#include "Parameters.h"\n#include "Presets.h"\n#include "Theme.h"\n#include "Branding.h"\nusing namespace aur::ui;\n'
        '@PEditor::@PEditor(@PProcessor& p):AudioProcessorEditor(p),ap(p){ setLookAndFeel(&lnf); auto& a=ap.getAPVTS();\n'
        '@KNOB@TOG'
        '  addAndMakeVisible(presetBox); refreshPresetBox(); presetBox.onChange=[this]{ const auto i=presetBox.getSelectedId()-1; if(i>=0) ap.setCurrentProgram(i);};\n'
        '  bypassButton.setClickingTogglesState(true); addAndMakeVisible(bypassButton);\n'
        '  bypassAtt=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(a,ParamID::bypass,bypassButton);\n'
        '  addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);\n  setSize(@W,400);\n}\n'
        '@PEditor::~@PEditor(){ setLookAndFeel(nullptr);}\n'
        'void @PEditor::refreshPresetBox(){ presetBox.clear(juce::dontSendNotification); int id=1; for(const auto& pr:Presets::getFactoryPresets()) presetBox.addItem(pr.name,id++); presetBox.setSelectedId(ap.getCurrentProgram()+1,juce::dontSendNotification);}\n'
        'void @PEditor::paint(juce::Graphics& g){ const auto& t=theme(); g.fillAll(t.ground);\n'
        '  const auto cx=(float)getWidth()*0.5f; juce::ColourGradient gl(t.accent.withAlpha(0.12f),cx,150.0f,t.ground.withAlpha(0.0f),cx,320.0f,true); g.setGradientFill(gl); g.fillRect(getLocalBounds());\n'
        '  drawBrandHeader(g,{20,16,360,60},"@TITLE","@SUB"); }\n'
        'void @PEditor::resized(){ auto area=getLocalBounds().reduced(18); auto header=area.removeFromTop(56); header.removeFromLeft(180);\n'
        '  bypassButton.setBounds(header.removeFromRight(88).reduced(4,12));\n'
        '  for(auto& bt:toggleBtns) bt->setBounds(header.removeFromRight(90).reduced(4,12));\n'
        '  presetBox.setBounds(header.removeFromRight(130).reduced(4,14));\n'
        '  area.removeFromTop(6); auto mc=area.removeFromRight(76); auto mi=mc.reduced(6); const auto mw=mi.getWidth()/2; inMeter.setBounds(mi.removeFromLeft(mw).reduced(4)); outMeter.setBounds(mi.reduced(4)); area.removeFromRight(8);\n'
        '  auto row=area; const int kw=row.getWidth()/@NK; for(auto& k:knobs) k->setBounds(row.removeFromLeft(kw).reduced(8)); }\n')
    return (T.replace("@PEditor", cfg["prefix"]+"Editor").replace("@PProcessor", cfg["prefix"]+"Processor")
             .replace("@KNOB", knob_init).replace("@TOG", tog_init).replace("@W", str(w))
             .replace("@TITLE", cfg["name"]).replace("@SUB", cfg["subtitle"]).replace("@NK", str(nk)))

def repl(s,cfg):
    return s.replace("%%NAME%%",cfg["name"]).replace("%%LOWER%%",cfg["name"].lower()).replace("%%CODE%%",cfg["code"])

def gen(cfg):
    d = os.path.join(ROOT, cfg["name"], "Source"); os.makedirs(d, exist_ok=True)
    open(os.path.join(ROOT,cfg["name"],"CMakeLists.txt"),"w").write(repl(CMAKE,cfg))
    open(os.path.join(d,"Parameters.h"),"w").write(params_h(cfg))
    open(os.path.join(d,"Presets.h"),"w").write(PRESETS_H)
    open(os.path.join(d,"Presets.cpp"),"w").write(presets_cpp(cfg))
    open(os.path.join(d,"PluginProcessor.h"),"w").write(proc_h(cfg))
    open(os.path.join(d,"PluginProcessor.cpp"),"w").write(proc_cpp(cfg))
    open(os.path.join(d,"PluginEditor.h"),"w").write(editor_h(cfg))
    open(os.path.join(d,"PluginEditor.cpp"),"w").write(editor_cpp(cfg))
    print("generated", cfg["name"])

CONFIGS = [
 dict(name="PITCH",code="Apt1",prefix="Pitch",subtitle="Pitch shifter",includes=["PitchShifter.h"],
   members="aur::PitchShifter fx;",prepare="fx.prepare(sr,getTotalNumOutputChannels());",
   setparams='fx.setParameters(V("pitch"),V("mix"));',
   process='fx.process(buffer.getArrayOfWritePointers(),tout,n);',
   params=[dict(id="pitch",name="Pitch",min="-24",max="24",kind="plain",step="0.1",**{"def":"0"}),
           dict(id="mix",name="Mix",min="0",max="100",kind="pct",step="0.1",**{"def":"100"})],
   knobs=[("pitch","PITCH"),("mix","MIX")],
   presets=[("Unison",{"pitch":"0","mix":"100"}),("Octave Up",{"pitch":"12","mix":"100"}),
            ("Octave Down",{"pitch":"-12","mix":"100"}),("Fifth",{"pitch":"7","mix":"100"}),
            ("Detune",{"pitch":"0.2","mix":"50"})]),
 dict(name="GAIN",code="Agn1",prefix="Gain",subtitle="Gain · balance · width · M/S",includes=["GainUtil.h"],
   members="aur::GainUtil fx;",prepare="fx.prepare(sr,getTotalNumOutputChannels());",
   setparams='fx.setParameters(V("gain"),V("balance"),V("width"),V("mono")>0.5f,V("phase")>0.5f);',
   process='fx.process(buffer.getArrayOfWritePointers(),tout,n);',
   params=[dict(id="gain",name="Gain",min="-24",max="24",kind="db",step="0.01",**{"def":"0"}),
           dict(id="balance",name="Balance",min="-1",max="1",kind="plain",step="0.01",**{"def":"0"}),
           dict(id="width",name="Width",min="0",max="200",kind="pct",step="1",**{"def":"100"})],
   bools=[dict(id="mono",name="Mono",**{"def":False}),dict(id="phase",name="Phase",**{"def":False})],
   knobs=[("gain","GAIN"),("balance","BALANCE"),("width","WIDTH")],
   presets=[("Unity",{"gain":"0","balance":"0","width":"100"}),("Trim -6",{"gain":"-6","balance":"0","width":"100"}),
            ("Narrow",{"gain":"0","balance":"0","width":"60"}),("Wide",{"gain":"0","balance":"0","width":"150"}),
            ("Boost +3",{"gain":"3","balance":"0","width":"100"})]),
 dict(name="TRANSIENT",code="Ats1",prefix="Transient",subtitle="Transient shaper",includes=["TransientShaper.h"],
   members="aur::TransientShaper fx;",prepare="fx.prepare(sr);",
   setparams='fx.setParameters(V("attack"),V("sustain"));',
   process='fx.process(buffer.getArrayOfWritePointers(),tout,n);',
   params=[dict(id="attack",name="Attack",min="-100",max="100",kind="plain",step="0.1",**{"def":"0"}),
           dict(id="sustain",name="Sustain",min="-100",max="100",kind="plain",step="0.1",**{"def":"0"})],
   knobs=[("attack","ATTACK"),("sustain","SUSTAIN")],
   presets=[("Punch",{"attack":"40","sustain":"-10"}),("Snap",{"attack":"70","sustain":"-30"}),
            ("Fat",{"attack":"-10","sustain":"40"}),("Soften",{"attack":"-40","sustain":"20"}),
            ("Room Kill",{"attack":"20","sustain":"-60"})]),
 dict(name="MULTI",code="Amb1",prefix="Multi",subtitle="3-band compressor",includes=["MultibandComp.h"],
   members="aur::MultibandComp fx;",prepare="fx.prepare(sr, getTotalNumOutputChannels(), spb);",
   setparams='fx.setParameters(V("xlo"),V("xhi"),V("thrL"),V("ratL"),V("thrM"),V("ratM"),V("thrH"),V("ratH"));',
   process='fx.process(buffer.getArrayOfWritePointers(),tout,n); meters.pushGainReduction(fx.getGainReductionDb());',
   params=[dict(id="xlo",name="X-Low",min="50",max="500",skew=0.5,kind="hz",step="1",**{"def":"200"}),
           dict(id="xhi",name="X-High",min="1000",max="8000",skew=0.5,kind="hz",step="1",**{"def":"2500"}),
           dict(id="thrL",name="Low Thr",min="-60",max="0",kind="db",step="0.1",**{"def":"-18"}),
           dict(id="ratL",name="Low Ratio",min="1",max="10",kind="plain",step="0.1",**{"def":"3"}),
           dict(id="thrM",name="Mid Thr",min="-60",max="0",kind="db",step="0.1",**{"def":"-18"}),
           dict(id="ratM",name="Mid Ratio",min="1",max="10",kind="plain",step="0.1",**{"def":"3"}),
           dict(id="thrH",name="High Thr",min="-60",max="0",kind="db",step="0.1",**{"def":"-18"}),
           dict(id="ratH",name="High Ratio",min="1",max="10",kind="plain",step="0.1",**{"def":"3"})],
   knobs=[("xlo","X-LO"),("thrL","LO THR"),("ratL","LO RAT"),("thrM","MD THR"),("ratM","MD RAT"),("thrH","HI THR"),("ratH","HI RAT"),("xhi","X-HI")],
   presets=[("Master Glue",{"xlo":"180","xhi":"3000","thrL":"-16","ratL":"2","thrM":"-14","ratM":"2","thrH":"-16","ratH":"2"}),
            ("Punchy",{"xlo":"150","xhi":"2500","thrL":"-20","ratL":"4","thrM":"-18","ratM":"3","thrH":"-18","ratH":"3"}),
            ("Tame Highs",{"xlo":"250","xhi":"4000","thrL":"-40","ratL":"1","thrM":"-30","ratM":"2","thrH":"-20","ratH":"5"}),
            ("Bass Tight",{"xlo":"120","xhi":"2500","thrL":"-24","ratL":"6","thrM":"-40","ratM":"1","thrH":"-40","ratH":"1"}),
            ("Gentle",{"xlo":"200","xhi":"3000","thrL":"-14","ratL":"1.5","thrM":"-14","ratM":"1.5","thrH":"-14","ratH":"1.5"})]),
 dict(name="EXCITE",code="Aex1",prefix="Excite",subtitle="Harmonic exciter",includes=["Exciter.h"],
   members="aur::Exciter fx;",prepare="fx.prepare(sr, getTotalNumOutputChannels(), spb);",
   setparams='fx.setParameters(V("freq"),V("amount"),V("mix"));',
   process='fx.process(buffer.getArrayOfWritePointers(),tout,n);',
   params=[dict(id="freq",name="Freq",min="1500",max="12000",skew=0.5,kind="hz",step="1",**{"def":"3000"}),
           dict(id="amount",name="Amount",min="0",max="100",kind="pct",step="0.1",**{"def":"40"}),
           dict(id="mix",name="Mix",min="0",max="100",kind="pct",step="0.1",**{"def":"35"})],
   knobs=[("freq","FREQ"),("amount","AMOUNT"),("mix","MIX")],
   presets=[("Air",{"freq":"8000","amount":"35","mix":"30"}),("Presence",{"freq":"4000","amount":"45","mix":"35"}),
            ("Sheen",{"freq":"10000","amount":"30","mix":"25"}),("Aggressive",{"freq":"3000","amount":"70","mix":"45"}),
            ("Subtle",{"freq":"6000","amount":"25","mix":"20"})]),
]

if __name__ == "__main__":
    for c in CONFIGS: gen(c)
    print("done")
