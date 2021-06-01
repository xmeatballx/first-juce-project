#include "MainComponent.h"
juce::Slider mFreqSlider;
juce::Slider mGainSlider;
juce::Slider mReduxSlider;
class Viz : public juce::AudioVisualiserComponent
{
public:
    Viz() : AudioVisualiserComponent(1)
    {
        setBufferSize(512);
        setSamplesPerBlock(128);
        setRepaintRate(30);
        setColours(juce::Colours::black, juce::Colours::indianred);
    }
};
Viz viz;
//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(mFreqSlider);
    addAndMakeVisible(mGainSlider);
    addAndMakeVisible(mReduxSlider);
    addAndMakeVisible(viz);
    
    mFreqSlider.setRange(20.0f, 20000.0f);
    mFreqSlider.setSkewFactorFromMidPoint(440.0f);
    mGainSlider.setRange(0.0f, 1.0f);
    mReduxSlider.setRange(1.0f, 10.0f);
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

double currentSampleRate = 0.0f, currentAngle = 0.0f, angleDelta = 0.0f, gain = 0.0f, redux = 1.0;

void updateAngleDelta()
{
    angleDelta = (mFreqSlider.getValue() / currentSampleRate) * 2.0 * juce::MathConstants<double>::pi;
}
//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    currentSampleRate = sampleRate;
    updateAngleDelta();
    mFreqSlider.onValueChange = [this]
    {
        updateAngleDelta();
    };
    
    mGainSlider.onValueChange = [this]
    {
        gain = mGainSlider.getValue();
    };
    
    mReduxSlider.onValueChange = [this]
    {
        redux = mReduxSlider.getValue();
    };

    viz.clear();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!
        auto* outBufferLeft = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* outBufferRight = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
            currentAngle += angleDelta;
            auto currentSample = (float) std::sin (currentAngle);
            currentSample *= redux;
            currentSample = std::round(currentSample);
            currentSample /= redux;
            
            outBufferLeft[sample] = currentSample * gain;
            outBufferRight[sample] = currentSample * gain;
        }
        viz.pushBuffer(bufferToFill);
    
    

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
//    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    // You can add your drawing code here!
}

void MainComponent::resized()
{
    viz.setBounds(100,250,400,200);
    mReduxSlider.setBounds(100,200, 200, 50);
    mGainSlider.setBounds(100,150,200,50);
    mFreqSlider.setBounds(100,100,200,50);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
