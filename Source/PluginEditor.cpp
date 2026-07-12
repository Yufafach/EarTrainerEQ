#include "PluginProcessor.h"
#include "PluginEditor.h"

EarTrainerAudioProcessorEditor::EarTrainerAudioProcessorEditor (EarTrainerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    titleLabel.setText ("EarTrainerEQ - тренажёр технического слуха", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    // Создаём по кнопке на каждую частоту из фиксированного списка.
    for (size_t i = 0; i < EarTrainerAudioProcessor::frequencyList.size(); ++i)
    {
        auto freq = EarTrainerAudioProcessor::frequencyList[i];

        juce::String label;
        if (freq >= 1000.0f)
        {
            float khz = freq / 1000.0f;
            bool isWholeNumber = (khz == (float) (int) khz);
            label = juce::String (khz, isWholeNumber ? 0 : 1) + " кГц";
        }
        else
        {
            label = juce::String ((int) freq) + " Гц";
        }

        auto* btn = new juce::TextButton (label);
        btn->setComponentID (juce::String ((int) i)); // храним индекс частоты в ID кнопки
        btn->addListener (this);
        addAndMakeVisible (btn);
        freqButtons.add (btn);
    }

    newRoundButton.addListener (this);
    addAndMakeVisible (newRoundButton);

    resultLabel.setJustificationType (juce::Justification::centred);
    resultLabel.setFont (juce::Font (16.0f));
    addAndMakeVisible (resultLabel);

    scoreLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (scoreLabel);

    refreshLabels();

    setSize (760, 480);
}

EarTrainerAudioProcessorEditor::~EarTrainerAudioProcessorEditor() = default;

void EarTrainerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2b2b2b));
}

void EarTrainerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    titleLabel.setBounds (area.removeFromTop (30));
    area.removeFromTop (8);

    resultLabel.setBounds (area.removeFromTop (26));
    scoreLabel.setBounds (area.removeFromTop (20));
    area.removeFromTop (8);

    newRoundButton.setBounds (area.removeFromBottom (36).reduced (200, 0));
    area.removeFromBottom (8);

    // Сетка 7 колонок x 4 строки = 28 кнопок.
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
    if (button == &newRoundButton)
    {
        processor.startNewRound();
        resultLabel.setText ("Слушайте... Какая область была изменена?", juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::white);
        refreshLabels();
        return;
    }

    // Если раунд уже завершён (ответ дан), игнорируем повторные нажатия на частоты -
    // пользователь должен нажать "Новый раунд".
    if (! processor.isRoundActive())
        return;

    int clickedIndex = button->getComponentID().getIntValue();
    bool correct = processor.checkAnswer (clickedIndex);

    juce::String correctFreqText = juce::String (processor.getCorrectFreqValue(), 0) + " Гц";
    juce::String direction = processor.wasBoost() ? "поднята (+6 дБ)" : "опущена (-6 дБ)";

    if (correct)
    {
        resultLabel.setText ("Верно! Область " + correctFreqText + " была " + direction,
                              juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    }
    else
    {
        resultLabel.setText ("Неверно. Правильный ответ: " + correctFreqText + " (" + direction + ")",
                              juce::dontSendNotification);
        resultLabel.setColour (juce::Label::textColourId, juce::Colours::orangered);
    }

    refreshLabels();
}

void EarTrainerAudioProcessorEditor::refreshLabels()
{
    scoreLabel.setText ("Счёт: " + juce::String (processor.getScoreCorrect()) + " / "
                             + juce::String (processor.getScoreTotal()),
                         juce::dontSendNotification);
}
