#pragma once

#include <JuceHeader.h>
#include <array>
#include <random>

/**
    Main audio processor.

    Game logic:
    - startNewRound() picks a random frequency from the fixed list and a
      random direction (+6dB boost or -6dB cut, unless cuts are disabled),
      then configures a peaking filter with Q = 4.3 at that frequency.
    - Audio passes through the filter in processBlock().
    - checkAnswer(freqIndex, guessedBoost) compares the user's guess
      (both frequency AND direction) against the correct answer.
*/
class EarTrainerAudioProcessor : public juce::AudioProcessor
{
public:
    EarTrainerAudioProcessor();
    ~EarTrainerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    //==============================================================================
    // Fixed frequency list (Hz). DO NOT change the order/contents.
    static const std::array<float, 28> frequencyList;

    static constexpr float qFactor = 4.3f;

    // The gain step magnitudes the user can choose between as a difficulty level.
    static constexpr std::array<float, 4> gainStepOptions { 3.0f, 6.0f, 9.0f, 12.0f };

    // Start a new round: pick a random frequency + random direction (respecting
    // allowCutRounds) using the currently selected gain step magnitude, and
    // recompute the filter coefficients.
    void startNewRound();

    // Check the user's answer: freqIndex is the guessed index in frequencyList,
    // guessedBoost is true if the user thinks it was boosted, false if cut.
    // Returns true only if BOTH the frequency and the direction are correct.
    // After this call the round is considered finished (roundActive = false)
    // until the next startNewRound().
    bool checkAnswer (int freqIndex, bool guessedBoost);

    int   getCorrectFreqIndex() const { return correctFreqIndex; }
    float getCorrectFreqValue() const { return frequencyList[(size_t) correctFreqIndex]; }
    bool  wasBoost() const { return currentGainDb > 0.0f; }
    bool  isRoundActive() const { return roundActive; }

    int getScoreCorrect() const { return scoreCorrect; }
    int getScoreTotal()   const { return scoreTotal; }

    // If false, every round will be a boost (+6dB) only - no cuts.
    void setAllowCutRounds (bool shouldAllow) { allowCutRounds = shouldAllow; }
    bool getAllowCutRounds() const { return allowCutRounds; }

    // The magnitude (in dB) used for the NEXT round. Must be one of gainStepOptions.
    void  setGainStepDb (float newStepDb) { selectedGainStepDb = newStepDb; }
    float getGainStepDb() const { return selectedGainStepDb; }

private:
    //==============================================================================
    using Filter       = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    // ProcessorDuplicator creates one filter per channel automatically (stereo).
    juce::dsp::ProcessorDuplicator<Filter, Coefficients> eqFilter;

    double currentSampleRate = 44100.0;

    int   correctFreqIndex = 0;
    float currentGainDb    = 6.0f; // the actual +/- value applied this round
    float selectedGainStepDb = 6.0f; // the difficulty level chosen by the user (3/6/9/12)

    bool roundActive = false;
    bool allowCutRounds = true;

    int scoreCorrect = 0;
    int scoreTotal   = 0;

    std::mt19937 rng { std::random_device{}() };

    void updateFilterCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarTrainerAudioProcessor)
};
