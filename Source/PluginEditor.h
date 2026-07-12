#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
    Plugin UI:
    - A grid of 28 frequency buttons (select one - it highlights).
    - Two direction buttons: "Boosted (+6dB)" and "Cut (-6dB)" - clicking one
      submits the current frequency selection + that direction as the answer.
    - A "New Round" button.
    - A checkbox to enable/disable cut (-6dB) rounds.
    - A result label and a score label.
*/
class EarTrainerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Button::Listener
{
public:
    explicit EarTrainerAudioProcessorEditor (EarTrainerAudioProcessor&);
    ~EarTrainerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked (juce::Button* button) override;
    void refreshLabels();
    void submitAnswer (bool guessedBoost);

    EarTrainerAudioProcessor& processor;

    juce::OwnedArray<juce::TextButton> freqButtons;
    int selectedFreqIndex = -1;

    juce::TextButton newRoundButton  { "New Round" };
    juce::TextButton boostButton     { "Boosted (+6dB)" };
    juce::TextButton cutButton       { "Cut (-6dB)" };
    juce::ToggleButton allowCutsToggle { "Include cuts (-6dB)" };

    juce::Label resultLabel;
    juce::Label scoreLabel;
    juce::Label titleLabel;
    juce::Label instructionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarTrainerAudioProcessorEditor)
};
