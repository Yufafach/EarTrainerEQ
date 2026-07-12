\xEF\xBB\xBF#include "PluginProcessor.h"
#include "PluginEditor.h"

// Fixed frequency list - must not be changed.
const std::array<float, 28> EarTrainerAudioProcessor::frequencyList = {
    31.5f, 40.0f, 50.0f, 63.0f, 80.0f, 100.0f, 125.0f, 160.0f, 200.0f, 250.0f,
    315.0f, 400.0f, 500.0f, 630.0f, 800.0f, 1000.0f, 1250.0f, 1600.0f, 2000.0f,
    2500.0f, 3150.0f, 4000.0f, 5000.0f, 6300.0f, 8000.0f, 10000.0f, 12500.0f, 16000.0f
};

EarTrainerAudioProcessor::EarTrainerAudioProcessor()
    : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // rng уже инициализирован через std::random_device в заголовке.
}

EarTrainerAudioProcessor::~EarTrainerAudioProcessor() = default;

//==============================================================================
void EarTrainerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) getTotalNumOutputChannels();

    eqFilter.prepare (spec);
    eqFilter.reset();

    // Если раунд ещё не начинался (первый запуск) - запускаем сразу первый раунд,
    // иначе просто пересчитываем коэффициенты под новый sample rate.
    if (! roundActive && scoreTotal == 0)
        startNewRound();
    else
        updateFilterCoefficients();
}

void EarTrainerAudioProcessor::releaseResources()
{
}

bool EarTrainerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Разрешаем только моно или стерео, вход = выход.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void EarTrainerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Глушим неиспользуемые входные каналы (стандартная практика JUCE-шаблонов).
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    eqFilter.process (context);
}

//==============================================================================
void EarTrainerAudioProcessor::updateFilterCoefficients()
{
    const float freq = frequencyList[(size_t) correctFreqIndex];
    const float gainLinear = juce::Decibels::decibelsToGain (currentGainDb);

    auto coeffs = Coefficients::makePeakFilter (currentSampleRate, freq, qFactor, gainLinear);
    *eqFilter.state = *coeffs;
}

void EarTrainerAudioProcessor::startNewRound()
{
    std::uniform_int_distribution<int> freqDist (0, (int) frequencyList.size() - 1);
    std::uniform_int_distribution<int> directionDist (0, 1); // 0 = cut, 1 = boost

    correctFreqIndex = freqDist (rng);

    const bool boost = allowCutRounds ? (directionDist (rng) == 1) : true;
    currentGainDb = boost ? gainStepDb : -gainStepDb;

    updateFilterCoefficients();

    roundActive = true;
}

bool EarTrainerAudioProcessor::checkAnswer (int freqIndex, bool guessedBoost)
{
    if (! roundActive)
        return false;

    const bool correct = (freqIndex == correctFreqIndex) && (guessedBoost == wasBoost());

    scoreTotal++;
    if (correct)
        scoreCorrect++;

    roundActive = false;
    return correct;
}

//==============================================================================
juce::AudioProcessorEditor* EarTrainerAudioProcessor::createEditor()
{
    return new EarTrainerAudioProcessorEditor (*this);
}

//==============================================================================
// Обязательная фабричная функция JUCE - должна существовать в проекте ровно одна.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EarTrainerAudioProcessor();
}
