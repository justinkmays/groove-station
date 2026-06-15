#include "SoundBrowser.h"

SoundBrowser::SoundBrowser (SamplerEngine& eng)
    : engine (eng), bookmarkModel (*this)
{
    previewFormatManager.registerBasicFormats();

    // File filter for audio files
    fileFilter = std::make_unique<juce::WildcardFileFilter> (
        "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg", "*", "Audio Files");

    // File browser in list mode
    browser = std::make_unique<juce::FileBrowserComponent> (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        juce::File::getSpecialLocation (juce::File::userHomeDirectory),
        fileFilter.get(), nullptr);

    browser->addListener (this);
    addAndMakeVisible (browser.get());

    // Search box
    addAndMakeVisible (searchBox);
    searchBox.setTextToShowWhenEmpty ("Search samples...", Colours_::textSecondary);
    searchBox.setColour (juce::TextEditor::backgroundColourId, Colours_::surface);
    searchBox.setColour (juce::TextEditor::textColourId, Colours_::textPrimary);
    searchBox.setColour (juce::TextEditor::outlineColourId, Colours_::surfaceLight);
    searchBox.onTextChange = [this] { applySearchFilter(); };

    // Load button
    addAndMakeVisible (loadButton);
    loadButton.onClick = [this] { loadSelectedToTargetPad(); };

    // Preview button
    addAndMakeVisible (previewButton);
    previewButton.setClickingTogglesState (true);
    previewButton.onClick = [this] { togglePreview(); };

    // Add bookmark button
    addAndMakeVisible (addBookmarkButton);
    addBookmarkButton.onClick = [this]
    {
        auto root = browser->getRoot();
        if (root.isDirectory())
            addBookmark (root);
    };

    // Bookmark list
    addAndMakeVisible (bookmarkList);
    bookmarkList.setModel (&bookmarkModel);
    bookmarkList.setRowHeight (22);
    bookmarkList.setColour (juce::ListBox::backgroundColourId, Colours_::surface);

    // Target pad label
    addAndMakeVisible (targetPadLabel);
    targetPadLabel.setColour (juce::Label::textColourId, Colours_::textSecondary);
    targetPadLabel.setFont (juce::Font (10.0f));
    setTargetPad (0);

    // Current folder label
    addAndMakeVisible (currentFolderLabel);
    currentFolderLabel.setColour (juce::Label::textColourId, Colours_::textSecondary);
    currentFolderLabel.setFont (juce::Font (9.0f));

    // Default bookmarks
    addBookmark (juce::File::getSpecialLocation (juce::File::userHomeDirectory));
    addBookmark (juce::File::getSpecialLocation (juce::File::userDesktopDirectory));
    addBookmark (juce::File::getSpecialLocation (juce::File::userMusicDirectory));
}

SoundBrowser::~SoundBrowser()
{
    stopPreview();
    browser->removeListener (this);
}

void SoundBrowser::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::surface);

    // Section header
    g.setColour (Colours_::textSecondary);
    g.setFont (juce::Font (11.0f).boldened());
    g.drawText ("SOUND BROWSER", getLocalBounds().reduced (8, 4).removeFromTop (16),
                juce::Justification::centredLeft);

    // Dividers
    g.setColour (Colours_::surfaceLight);
    g.drawHorizontalLine (20, 4.0f, (float) getWidth() - 4.0f);
}

void SoundBrowser::resized()
{
    auto area = getLocalBounds().reduced (4);
    area.removeFromTop (22); // header

    // Target pad label + search row
    auto topRow = area.removeFromTop (24);
    targetPadLabel.setBounds (topRow.removeFromLeft (60).reduced (2));
    searchBox.setBounds (topRow.reduced (2));

    area.removeFromTop (2);

    // Bookmarks sidebar (left 25%)
    auto bookmarkArea = area.removeFromLeft (juce::jmin (120, area.getWidth() / 4));
    addBookmarkButton.setBounds (bookmarkArea.removeFromBottom (24).reduced (2));
    bookmarkList.setBounds (bookmarkArea.reduced (0, 2));

    area.removeFromLeft (2);

    // Bottom row: Load + Preview buttons
    auto bottomRow = area.removeFromBottom (28);
    loadButton.setBounds (bottomRow.removeFromLeft (bottomRow.getWidth() / 2).reduced (2));
    previewButton.setBounds (bottomRow.reduced (2));

    // Current folder label
    currentFolderLabel.setBounds (area.removeFromBottom (14));

    // File browser fills the rest
    browser->setBounds (area);
}

void SoundBrowser::selectionChanged()
{
    // Nothing specific needed on selection change
}

void SoundBrowser::fileClicked (const juce::File& file, const juce::MouseEvent&)
{
    if (file.existsAsFile())
    {
        currentFolderLabel.setText (file.getFileName(), juce::dontSendNotification);
    }
}

void SoundBrowser::fileDoubleClicked (const juce::File& file)
{
    if (file.existsAsFile())
    {
        // Double-click loads to target pad
        int absPad = targetPad;
        if (engine.loadSample (absPad, file))
        {
            if (onSampleLoaded)
                onSampleLoaded (absPad);
        }
    }
}

void SoundBrowser::browserRootChanged (const juce::File& newRoot)
{
    currentFolderLabel.setText (newRoot.getFullPathName(), juce::dontSendNotification);
}

void SoundBrowser::setTargetPad (int absolutePadIndex)
{
    targetPad = juce::jlimit (0, SamplerEngine::TOTAL_PADS - 1, absolutePadIndex);
    int bank = targetPad / SamplerEngine::PADS_PER_BANK;
    int localPad = targetPad % SamplerEngine::PADS_PER_BANK;
    targetPadLabel.setText ("-> " + engine.getBankName (bank) + juce::String (localPad + 1),
                            juce::dontSendNotification);
}

void SoundBrowser::addBookmark (const juce::File& folder)
{
    if (! folder.isDirectory())
        return;

    // Avoid duplicates
    auto path = folder.getFullPathName();
    if (bookmarkPaths.contains (path))
        return;

    bookmarkPaths.add (path);
    bookmarkNames.add (folder.getFileName());
    bookmarkList.updateContent();
}

void SoundBrowser::removeBookmark (int index)
{
    if (index >= 0 && index < bookmarkPaths.size())
    {
        bookmarkPaths.remove (index);
        bookmarkNames.remove (index);
        bookmarkList.updateContent();
    }
}

void SoundBrowser::loadSelectedToTargetPad()
{
    auto selectedFile = browser->getSelectedFile (0);
    if (selectedFile.existsAsFile())
    {
        int absPad = targetPad;
        if (engine.loadSample (absPad, selectedFile))
        {
            if (onSampleLoaded)
                onSampleLoaded (absPad);
        }
    }
}

void SoundBrowser::togglePreview()
{
    if (previewButton.getToggleState())
    {
        auto selectedFile = browser->getSelectedFile (0);
        if (selectedFile.existsAsFile())
        {
            std::unique_ptr<juce::AudioFormatReader> reader (
                previewFormatManager.createReaderFor (selectedFile));
            if (reader != nullptr)
            {
                previewSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
                previewTransport.setSource (previewSource.get());
                previewTransport.start();
                return;
            }
        }
        previewButton.setToggleState (false, juce::dontSendNotification);
    }
    else
    {
        stopPreview();
    }
}

void SoundBrowser::stopPreview()
{
    previewTransport.stop();
    previewTransport.setSource (nullptr);
    previewSource.reset();
    previewButton.setToggleState (false, juce::dontSendNotification);
}

void SoundBrowser::navigateToBookmark (int index)
{
    if (index >= 0 && index < bookmarkPaths.size())
    {
        juce::File folder (bookmarkPaths[index]);
        if (folder.isDirectory())
            browser->setRoot (folder);
    }
}

void SoundBrowser::applySearchFilter()
{
    auto searchText = searchBox.getText().trim();
    if (searchText.isEmpty())
    {
        fileFilter = std::make_unique<juce::WildcardFileFilter> (
            "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg", "*", "Audio Files");
    }
    else
    {
        fileFilter = std::make_unique<juce::WildcardFileFilter> (
            "*" + searchText + "*.wav;*" + searchText + "*.aiff;*" + searchText + "*.aif;"
            + "*" + searchText + "*.mp3;*" + searchText + "*.flac;*" + searchText + "*.ogg",
            "*" + searchText + "*", "Search: " + searchText);
    }

    auto currentRoot = browser->getRoot();
    browser->removeListener (this);
    browser.reset();

    browser = std::make_unique<juce::FileBrowserComponent> (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        currentRoot, fileFilter.get(), nullptr);

    browser->addListener (this);
    addAndMakeVisible (browser.get());
    resized();
}

// BookmarkListModel
void SoundBrowser::BookmarkListModel::paintListBoxItem (int row, juce::Graphics& g,
                                                         int width, int height, bool selected)
{
    if (row >= 0 && row < owner.bookmarkNames.size())
    {
        if (selected)
            g.fillAll (Colours_::accent.withAlpha (0.3f));

        g.setColour (selected ? Colours_::textPrimary : Colours_::textSecondary);
        g.setFont (10.0f);
        g.drawText (owner.bookmarkNames[row], 4, 0, width - 8, height,
                     juce::Justification::centredLeft);
    }
}

void SoundBrowser::BookmarkListModel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    owner.navigateToBookmark (row);
}

void SoundBrowser::BookmarkListModel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    owner.navigateToBookmark (row);
}
