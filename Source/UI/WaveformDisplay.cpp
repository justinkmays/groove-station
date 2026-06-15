#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay (SamplerEngine& eng) : engine (eng)
{
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (Colours_::waveformBg);
    g.fillRoundedRectangle (bounds, 4.0f);

    auto* buffer = engine.getSampleBuffer (currentPad);
    if (buffer == nullptr || buffer->getNumSamples() == 0)
    {
        g.setColour (Colours_::textSecondary);
        g.setFont (14.0f);
        g.drawText ("No sample loaded — drag & drop audio file onto a pad",
                     bounds, juce::Justification::centred);
        return;
    }

    auto& state = engine.getPadState (currentPad);
    int numSamples = buffer->getNumSamples();
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float midY = bounds.getCentreY();

    // Draw start/end region overlay
    float startX = bounds.getX() + state.startPos * width;
    float endX = bounds.getX() + state.endPos * width;

    g.setColour (Colours_::waveform.withAlpha (0.08f));
    g.fillRect (startX, bounds.getY(), endX - startX, height);

    // Draw waveform
    auto* samples = buffer->getReadPointer (0);
    float samplesPerPixel = (float) numSamples / width;

    g.setColour (Colours_::waveform.withAlpha (0.8f));

    juce::Path waveformPath;
    bool pathStarted = false;

    for (int px = 0; px < (int) width; ++px)
    {
        int sampleStart = (int)(px * samplesPerPixel);
        int sampleEnd = juce::jmin ((int)((px + 1) * samplesPerPixel), numSamples);

        float minVal = 0.0f, maxVal = 0.0f;
        for (int s = sampleStart; s < sampleEnd; ++s)
        {
            float val = samples[s];
            minVal = juce::jmin (minVal, val);
            maxVal = juce::jmax (maxVal, val);
        }

        float topY = midY - maxVal * (height * 0.45f);
        float bottomY = midY - minVal * (height * 0.45f);

        if (! pathStarted)
        {
            waveformPath.startNewSubPath (bounds.getX() + px, topY);
            pathStarted = true;
        }
        else
        {
            waveformPath.lineTo (bounds.getX() + px, topY);
        }
    }

    // Go back along the bottom
    for (int px = (int) width - 1; px >= 0; --px)
    {
        int sampleStart = (int)(px * samplesPerPixel);
        int sampleEnd = juce::jmin ((int)((px + 1) * samplesPerPixel), numSamples);

        float minVal = 0.0f;
        for (int s = sampleStart; s < sampleEnd; ++s)
            minVal = juce::jmin (minVal, samples[s]);

        float bottomY = midY - minVal * (height * 0.45f);
        waveformPath.lineTo (bounds.getX() + px, bottomY);
    }

    waveformPath.closeSubPath();
    g.fillPath (waveformPath);

    // Center line
    g.setColour (Colours_::textSecondary.withAlpha (0.3f));
    g.drawHorizontalLine ((int) midY, bounds.getX(), bounds.getRight());

    // Start/End markers
    g.setColour (Colours_::meterGreen);
    g.fillRect (startX - 1.0f, bounds.getY(), 2.0f, height);
    g.setColour (Colours_::meterRed);
    g.fillRect (endX - 1.0f, bounds.getY(), 2.0f, height);

    // Labels
    g.setColour (Colours_::textSecondary);
    g.setFont (10.0f);
    g.drawText ("S", (int)(startX - 8), (int) bounds.getY() + 2, 16, 12, juce::Justification::centred);
    g.drawText ("E", (int)(endX - 8), (int) bounds.getY() + 2, 16, 12, juce::Justification::centred);

    // Sample name
    g.setColour (Colours_::textPrimary);
    g.setFont (12.0f);
    g.drawText (state.sampleName, bounds.reduced (8.0f, 4.0f),
                 juce::Justification::bottomLeft);
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::mouseDown (const juce::MouseEvent& e)
{
    auto& state = engine.getPadState (currentPad);
    float normalizedX = positionToNormalized (e.x);
    float startDist = std::abs (normalizedX - state.startPos);
    float endDist = std::abs (normalizedX - state.endPos);

    if (startDist < 0.02f)
        dragTarget = DragTarget::Start;
    else if (endDist < 0.02f)
        dragTarget = DragTarget::End;
    else if (normalizedX < (state.startPos + state.endPos) * 0.5f)
        dragTarget = DragTarget::Start;
    else
        dragTarget = DragTarget::End;
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& e)
{
    auto& state = engine.getPadState (currentPad);
    float normalizedX = positionToNormalized (e.x);

    if (dragTarget == DragTarget::Start)
    {
        state.startPos = juce::jlimit (0.0f, state.endPos - 0.01f, normalizedX);
    }
    else if (dragTarget == DragTarget::End)
    {
        state.endPos = juce::jlimit (state.startPos + 0.01f, 1.0f, normalizedX);
    }

    engine.updatePadParameters (currentPad);

    if (onRegionChanged)
        onRegionChanged (state.startPos, state.endPos);

    repaint();
}

void WaveformDisplay::setPadIndex (int index)
{
    currentPad = juce::jlimit (0, SamplerEngine::TOTAL_PADS - 1, index);
    repaint();
}

float WaveformDisplay::positionToNormalized (int x) const
{
    return juce::jlimit (0.0f, 1.0f, (float) x / (float) getWidth());
}

int WaveformDisplay::normalizedToPosition (float n) const
{
    return (int)(n * (float) getWidth());
}
