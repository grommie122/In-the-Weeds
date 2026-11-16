#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>

namespace
{
struct Control
{
    const char* id;
    const char* name;
    float minimum, maximum, initial;
};

constexpr std::array<Control, 4> controls {{
    {"smoke", "Smoke (dB)", 0, 42, 22},
    {"up", "Octave Up (%)", 0, 100, 55},
    {"down", "Octave Down (%)", 0, 100, 0},
    {"level", "Level (dB)", -30, 6, -8}
}};

using State = juce::AudioProcessorValueTreeState;

State::ParameterLayout makeParameters()
{
    State::ParameterLayout layout;
    for (const auto& control : controls)
    {
        const juce::ParameterID id {control.id, 1};
        layout.add(std::make_unique<juce::AudioParameterFloat>(id, control.name,
            juce::NormalisableRange<float>{control.minimum, control.maximum, 1.0f},
            control.initial));
    }
    return layout;
}

class WeedsEditor final : public juce::AudioProcessorEditor
{
public:
    WeedsEditor(juce::AudioProcessor& processor, State& state) : AudioProcessorEditor(processor)
    {
        for (size_t i = 0; i < knobs.size(); ++i)
        {
            const auto& control = controls[knobIndices[i]];
            auto& knob = knobs[i];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 23);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffd496));
            knob.setName(control.name);
            addAndMakeVisible(knob);
            knobLinks[i] = std::make_unique<State::SliderAttachment>(state, control.id, knob);
        }
        setSize(580, 264);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff102c22));
        g.setColour(juce::Colours::white);
        g.setFont(26.0f);
        g.drawText("In the Weeds", 20, 12, 580, 38, juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff21503b));
        g.fillRoundedRectangle(20, 60, 540, 184, 12);
        g.setColour(juce::Colours::white);
        g.setFont(17.0f);
        g.drawText("Octave fuzz", 34, 67, 120, 25, juce::Justification::centredLeft);
        // Draw seven serrated leaflets from the same point.
        juce::Path leaf;
        const auto addLeaflet = [&leaf](float angle, float length, float width)
        {
            juce::Path blade;
            blade.startNewSubPath(0.0f, 0.0f);
            blade.lineTo(-0.52f * width, -0.22f * length);
            blade.lineTo(-0.37f * width, -0.30f * length);
            blade.lineTo(-width, -0.47f * length);
            blade.lineTo(-0.56f * width, -0.48f * length);
            blade.lineTo(-0.85f * width, -0.67f * length);
            blade.lineTo(-0.35f * width, -0.65f * length);
            blade.lineTo(0.0f, -length);
            blade.lineTo(0.35f * width, -0.65f * length);
            blade.lineTo(0.85f * width, -0.67f * length);
            blade.lineTo(0.56f * width, -0.48f * length);
            blade.lineTo(width, -0.47f * length);
            blade.lineTo(0.37f * width, -0.30f * length);
            blade.lineTo(0.52f * width, -0.22f * length);
            blade.closeSubPath();
            leaf.addPath(blade, juce::AffineTransform::rotation(angle));
        };
        addLeaflet(0.0f, 23.0f, 3.7f);
        addLeaflet(-0.52f, 21.0f, 3.4f);
        addLeaflet(0.52f, 21.0f, 3.4f);
        addLeaflet(-1.02f, 17.0f, 2.9f);
        addLeaflet(1.02f, 17.0f, 2.9f);
        addLeaflet(-1.58f, 12.0f, 2.1f);
        addLeaflet(1.58f, 12.0f, 2.1f);
        g.setColour(juce::Colour(0xff8acb70));
        g.fillPath(leaf, juce::AffineTransform::translation(526.0f, 39.0f));
        g.drawLine(526.0f, 38.0f, 527.0f, 46.0f, 1.6f);
        g.setColour(juce::Colours::white);
        g.setFont(13.0f);
        for (size_t i = 0; i < knobs.size(); ++i)
            g.drawText(controls[knobIndices[i]].name, knobX[i], 208, 106, 23, juce::Justification::centred);
    }

    void resized() override
    {
        for (size_t i = 0; i < knobs.size(); ++i)
            knobs[i].setBounds(knobX[i], 102, 106, 103);
    }

private:
    static constexpr std::array<size_t, 4> knobIndices {0, 1, 2, 3};
    static constexpr std::array<int, 4> knobX {36, 169, 302, 435};
    std::array<juce::Slider, 4> knobs;
    std::array<std::unique_ptr<State::SliderAttachment>, 4> knobLinks;
};

class WeedsFuzz final : public juce::AudioProcessor
{
public:
    WeedsFuzz() : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
        state(*this, nullptr, "WeedsFuzz", makeParameters())
    {
        for (size_t i = 0; i < controls.size(); ++i)
            values[i] = state.getRawParameterValue(controls[i].id);
    }

    const juce::String getName() const override { return "In the Weeds"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.08; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return new WeedsEditor(*this, state); }
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layout) const override
    {
        const auto output = layout.getMainOutputChannelSet();
        return layout.getMainInputChannelSet() == output
            && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
    }

    void prepareToPlay(double sampleRate, int) override
    {
        grain = juce::jmax(32, static_cast<int>(0.055 * sampleRate));
        history.setSize(2, grain + 8);
        reset();
        for (size_t i = 0; i < smooth.size(); ++i)
        {
            smooth[i].reset(sampleRate, 0.02);
            smooth[i].setCurrentAndTargetValue(values[i]->load());
        }
        const auto pole = [sampleRate](float hz)
        {
            return 1.0f - std::exp(-juce::MathConstants<float>::twoPi * hz / static_cast<float>(sampleRate));
        };
        inputPole = pole(105);
        dcPole = pole(22);
        tonePole = pole(4400);
    }

    void reset() override
    {
        history.clear();
        inputLow.fill(0);
        rectifiedDC.fill(0);
        outputLow.fill(0);
        phase = 0;
        write = 0;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        const auto length = history.getNumSamples();
        for (size_t i = 0; i < smooth.size(); ++i)
            smooth[i].setTargetValue(values[i]->load());

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto drive = juce::Decibels::decibelsToGain(smooth[0].getNextValue());
            const auto up = smooth[1].getNextValue() * 0.01f;
            const auto down = smooth[2].getNextValue() * 0.01f;
            const auto level = juce::Decibels::decibelsToGain(smooth[3].getNextValue());
            const auto second = phase < 0.5f ? phase + 0.5f : phase - 0.5f;
            const auto weight = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phase);

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const auto input = buffer.getSample(channel, sample);
                inputLow[channel] += inputPole * (input - inputLow[channel]);
                const auto ac = input - inputLow[channel];
                history.setSample(channel, write, input - 0.45f * inputLow[channel]);
                const auto lower = weight * read(channel, phase) + (1 - weight) * read(channel, second);
                const auto rectified = std::abs(std::tanh(0.65f * drive * ac));
                rectifiedDC[channel] += dcPole * (rectified - rectifiedDC[channel]);
                const auto fuzz = std::tanh(1.75f * drive * ac);
                const auto upper = 2.0f * (rectified - rectifiedDC[channel]);
                const auto blend = (fuzz * (1.0f - 0.83f * up) + up * upper
                                    + down * std::tanh(0.6f * drive * lower)) / (1.0f + 0.6f * down);
                outputLow[channel] += tonePole * (blend - outputLow[channel]);
                buffer.setSample(channel, sample, level * outputLow[channel]);
            }
            phase += 0.5f / static_cast<float>(grain);
            phase -= std::floor(phase);
            write = (write + 1) % length;
        }
    }
    void getStateInformation(juce::MemoryBlock& destination) override
    {
        if (auto xml = state.copyState().createXml())
            copyXmlToBinary(*xml, destination);
    }

    void setStateInformation(const void* data, int bytes) override
    {
        if (auto xml = getXmlFromBinary(data, bytes))
            if (xml->hasTagName(state.state.getType()))
                state.replaceState(juce::ValueTree::fromXml(*xml));
    }

private:
    State state;
    std::array<std::atomic<float>*, 4> values {};
    float read(int channel, float position) const
    {
        const auto length = history.getNumSamples();
        auto index = static_cast<float>(write) - 2.0f - position * static_cast<float>(grain);
        if (index < 0.0f)
            index += static_cast<float>(length);
        const auto left = static_cast<int>(index);
        const auto fraction = index - static_cast<float>(left);
        const auto a = history.getSample(channel, left);
        const auto b = history.getSample(channel, (left + 1) % length);
        return a + fraction * (b - a);
    }

    juce::AudioBuffer<float> history;
    std::array<juce::SmoothedValue<float>, 4> smooth;
    std::array<float, 2> inputLow {}, rectifiedDC {}, outputLow {};
    float phase = 0, inputPole = 0, dcPole = 0, tonePole = 0;
    int grain = 0;
    int write = 0;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WeedsFuzz();
}





