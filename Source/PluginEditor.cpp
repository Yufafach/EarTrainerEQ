#include "PluginProcessor.h"
#include "PluginEditor.h"

EarTrainerAudioProcessorEditor::EarTrainerAudioProcessorEditor (EarTrainerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    titleLabel.setText ("EarTrainerEQ - Ear Training Tool", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    instructionLabel.setText ("1. Pick a frequency below   2. Say whether it was Boosted or Cut",
                               juce::dontSendNotification);
    instructionLabel.setJustificationType (juce::Justification::centred);
    instructionLabel.setFont (juce::Font (13.0f));
    addAndMakeVisible (instructionLabel);

    // Create one button per frequency in the fixed list.
    for (size_t i = 0; i < EarTrainerAudioProcessor::frequencyList.size(); ++i)
    {
        auto freq = EarTrainerAudioProcessor::frequencyList[i];

        juce::String label;
        if (freq >= 1000.0f)
        {
            float khz = freq / 1000.0f;
            bool isWholeNumber = (khz == (float) (int) khz);
            label = juce::String (khz, isWholeNumber ? 0 : 1) + " kHz";
        }
        else
        {
            label = juce::String ((int) freq) + " Hz";
        }

        auto* btn = new juce::TextButton (label);
        btn->setComponentID (juce::String ((int) i)); // store the frequency index in the button ID
        btn->setClickingTogglesState (false);
        btn->addListener (this);
        addAndMakeVisible (btn);
        freqButtons.add (btn);
    }

    newRoundButton.addListener (this);
    addAndMakeVisible (newRoundButton);

    boostButton.addListener (this);
    addAndMakeVisible (boostButton);

    cutButton.addListener (this);
    addAndMakeVisible (cutButton);

    allowCutsToggle.setToggleState (processor.getAllowCutRounds(), juce::dontSendNotification);
    allowCutsToggle.addListener (this);
    addAndMakeVisible (allowCutsToggle);

    gainStepCaption.setText ("Difficulty (dB step):", juce::dontSendNotification);
    gainStepCaption.setFont (juce::Font (13.0f));
    addAndMakeVisible (gainStepCaption);

    // One radio-style button per available gain step (3 / 6 / 9 / 12 dB).
    for (size_t i = 0; i < EarTrainerAudioProcessor::gainStepOptions.size(); ++i)
    {
        float stepDb = EarTrainerAudioProcessor::gainStepOptions[i];

        auto* btn = new juce::TextButton (juce::String ((int) stepDb) + " dB");
        btn->setRadioGroupId (1001, juce::dontSendNotification);
        btn->setClickingTogglesState (true);
        btn->setComponentID ("gainstep_" + juce::String ((int) stepDb));
        btn->setToggleState (juce::approximatelyEqual (stepDb, processor.getGainStepDb()),
                             juce::dontSendNotification);
        btn->addListener (this);
        addAndMakeVisible (btn);
        gainStepButtons.add (btn);
    }

    resultLabel.setJustificationType (juce::Justification::centred);
    resultLabel.setFont (juce::Font (16.0f));
    addAndMakeVisible (resultLabel);

    scoreLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (scoreLabel);

    refreshLabels();

    setSize (760, 560);
}

EarTrainerAudioProcessorEditor::~EarTrainerAudioProcessorEditor() = default;

void EarTrainerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2b2b2b));
}

void EarTrainerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    titleLabel.setBounds (area.removeFromTop (28));
    instructionLabel.setBounds (area.removeFromTop (20));
    area.removeFromTop (6);

    resultLabel.setBounds (area.removeFromTop (26));
    scoreLabel.setBounds (area.removeFromTop (20));
    area.removeFromTop (6);

    allowCutsToggle.setBounds (area.removeFromTop (24).reduced (220, 0));
    area.removeFromTop (4);

    // Difficulty row: caption + 4 buttons side by side.
    auto diffRow = area.removeFromTop (28);
    gainStepCaption.setBounds (diffRow.removeFromLeft (140));
    auto diffButtonsArea = diffRow.reduced (150, 0);
    int diffButtonWidth = diffButtonsArea.getWidth() / (int) gainStepButtons.size();
    for (int i = 0; i < gainStepButtons.size(); ++i)
        gainStepButtons[i]->setBounds (diffButtonsArea.removeFromLeft (diffButtonWidth).reduced (3, 0));
    area.removeFromTop (8);

    // Bottom row: New Round + direction answer buttons.
    auto bottomRow = area.removeFromBottom (36);
    newRoundButton.setBounds (bottomRow.removeFromLeft (bottomRow.getWidth() / 3).reduced (4, 0));
    cutButton.setBounds (bottomRow.removeFromRight (bottomRow.getWidth() / 2).reduced (4, 0));
    boostButton.setBounds (bottomRow.reduced (4, 0));
    area.removeFromBottom (8);

    // 7 x 4 grid of frequency buttons.
    const int columns = 7;
    const int rows    = 4;
    const int gap     = 6;

    auto cellWidth  = (area.getWidth()  - gap * (columns - 1)) / columns;
    auto cellHeight = (area.getHeight() - gap * (rows - 1)) / rows;

    for (int i = 0; i < freqButtons.size(); ++i)
    {
        int col = i % columns;
        int row = i / columns;

        int x = area.getX() + col * (cellWidth + gap);
        int y = area.getY() + row * (cellHeight + gap);

        freqButtons[i]->setBounds (x, y, cellWidth, cellHeight);
    }
}

void EarTrainerAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &allowCutsToggle)
    {
        processor.setAllowCutRounds (allowCutsToggle.getToggleState());
        return;
    }

    // Gain step (difficulty) buttons.
    if (button->getComponentID().startsWith ("gainstep_"))
    {
        float stepDb = button->getComponentID().fromFirstOccurrenceOf ("gainstep_", false, false).getFloatValue();
        processor.setGainStepDb (stepDb);
        return;
    }

    if (button == &newRoundButton)
    {
        processor.startNewRound();
        selectedFreqIndex = -1;

        for (auto* b : freqButtons)
            b->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);

        resultLabel.setText ("Listen... then pick a frequency and a direction.", juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::white);
        refreshLabels();
        return;
    }

    if (button == &boostButton)
    {
        submitAnswer (true);
        return;
    }

    if (button == &cutButton)
    {
        submitAnswer (false);
        return;
    }

    // Otherwise it must be one of the frequency buttons.
    if (! processor.isRoundActive())
        return;

    selectedFreqIndex = button->getComponentID().getIntValue();

    for (auto* b : freqButtons)
        b->setColour (juce::TextButton::buttonColourId,
                      (b->getComponentID().getIntValue() == selectedFreqIndex)
                          ? juce::Colours::steelblue
                          : juce::Colours::transparentBlack);

    resultLabel.setText ("Frequency selected. Now choose Boosted or Cut.", juce::dontSendNotification);
    resultLabel.setColour (juce::Label::textColourId, juce::Colours::white);
}

void EarTrainerAudioProcessorEditor::submitAnswer (bool guessedBoost)
{
    if (! processor.isRoundActive())
        return;

    if (selectedFreqIndex < 0)
    {
        resultLabel.setText ("Please pick a frequency first.", juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
        return;
    }

    bool correct = processor.checkAnswer (selectedFreqIndex, guessedBoost);

    juce::String correctFreqText = juce::String (processor.getCorrectFreqValue(), 0) + " Hz";
    juce::String direction = processor.wasBoost() ? "boosted" : "cut";
    juce::String stepText = juce::String ((int) processor.getGainStepDb()) + "dB";

    if (correct)
    {
        resultLabel.setText ("Correct! It was " + correctFreqText + ", " + direction + " (" + stepText + ")",
                              juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    }
    else
    {
        resultLabel.setText ("Wrong. It was " + correctFreqText + ", " + direction + " (" + stepText + ")",
                              juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::orangered);
    }

    refreshLabels();
}

void EarTrainerAudioProcessorEditor::refreshLabels()
{
    scoreLabel.setText ("Score: " + juce::String (processor.getScoreCorrect()) + " / "
                             + juce::String (processor.getScoreTotal()),
                         juce::dontSendNotification);
}
