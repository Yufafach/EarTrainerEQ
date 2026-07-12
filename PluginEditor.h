#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
    Интерфейс плагина:
    - Сетка из 28 кнопок с частотами.
    - Кнопка "Новый раунд".
    - Текстовая метка с результатом и счётом.
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

    EarTrainerAudioProcessor& processor;

    juce::OwnedArray<juce::TextButton> freqButtons;
    juce::TextButton newRoundButton { "Новый раунд" };

    juce::Label resultLabel;
    juce::Label scoreLabel;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarTrainerAudioProcessorEditor)
};
