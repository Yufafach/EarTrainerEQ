#pragma once

#include <JuceHeader.h>
#include <array>
#include <random>

/**
    Основной аудио-процессор.

    Логика игры:
    - startNewRound() выбирает случайную частоту из фиксированного списка
      и случайное направление (+6 дБ или -6 дБ), настраивает peaking-фильтр
      с Q = 4.3 на этой частоте.
    - Звук проходит через фильтр в processBlock().
    - checkAnswer(index) сравнивает выбор пользователя с правильным индексом.
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
    // Фиксированный список частот (Гц). НЕ МЕНЯТЬ порядок/состав по условию ТЗ.
    static const std::array<float, 28> frequencyList;

    static constexpr float qFactor   = 4.3f;
    static constexpr float gainStepDb = 6.0f; // всегда ровно +6 или -6 дБ

    // Запустить новый раунд: выбрать случайную частоту + случайное направление
    // и пересчитать коэффициенты фильтра.
    void startNewRound();

    // Проверить ответ пользователя. index - индекс в frequencyList.
    // Возвращает true, если ответ верный. После вызова раунд считается завершённым
    // (roundActive = false) до следующего startNewRound().
    bool checkAnswer (int index);

    int  getCorrectFreqIndex() const { return correctFreqIndex; }
    float getCorrectFreqValue() const { return frequencyList[(size_t) correctFreqIndex]; }
    bool  wasBoost() const { return currentGainDb > 0.0f; }
    bool  isRoundActive() const { return roundActive; }

    int getScoreCorrect() const { return scoreCorrect; }
    int getScoreTotal()   const { return scoreTotal; }

private:
    //==============================================================================
    using Filter       = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    // ProcessorDuplicator сам создаёт по фильтру на каждый канал (стерео).
    juce::dsp::ProcessorDuplicator<Filter, Coefficients> eqFilter;

    double currentSampleRate = 44100.0;

    int   correctFreqIndex = 0;
    float currentGainDb    = gainStepDb; // +6 или -6

    bool roundActive = false;

    int scoreCorrect = 0;
    int scoreTotal   = 0;

    std::mt19937 rng { std::random_device{}() };

    void updateFilterCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarTrainerAudioProcessor)
};
