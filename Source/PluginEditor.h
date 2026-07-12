#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
    Plugin UI:
    - A grid of 28 frequency buttons (select one - it highlights).
    - Two direction buttons: "Boosted" and "Cut" - clicking one submits the
      current frequency selection + that direction as the answer.
    - A "New Round" button.
    - A checkbox to enable/disable cut rounds.
    - Four difficulty buttons (3/6/9/12 dB) to choose the gain step magnitude.
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
    juce::TextButton boostButton     { "Boosted" };
    juce::TextButton cutButton       { "Cut" };
    juce::ToggleButton allowCutsToggle { "Include cuts" };

    juce::OwnedArray<juce::TextButton> gainStepButtons;
    juce::Label gainStepCaption;

    juce::Label resultLabel;
    juce::Label scoreLabel;
    juce::Label titleLabel;
    juce::Label instructionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarTrainerAudioProcessorEditor)
};
