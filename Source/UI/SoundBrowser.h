#pragma once
#include <JuceHeader.h>
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class SoundBrowser : public juce::Component,
                     public juce::FileBrowserListener
{
public:
    SoundBrowser (SamplerEngine& engine);
    ~SoundBrowser() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // FileBrowserListener
    void selectionChanged() override;
    void fileClicked (const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked (const juce::File& file) override;
    void browserRootChanged (const juce::File& newRoot) override;

    // Load the selected file to a target pad (absolute index)
    void setTargetPad (int absolutePadIndex);
    int getTargetPad() const { return targetPad; }

    // Bookmarks
    void addBookmark (const juce::File& folder);
    void removeBookmark (int index);

    std::function<void (int absolutePadIndex)> onSampleLoaded;

private:
    SamplerEngine& engine;
    int targetPad = 0;

    // File browser
    std::unique_ptr<juce::WildcardFileFilter> fileFilter;
    std::unique_ptr<juce::FileBrowserComponent> browser;

    // Search bar
    juce::TextEditor searchBox;
    juce::TextButton loadButton { "LOAD" };
    juce::TextButton previewButton { "PREVIEW" };
    juce::TextButton addBookmarkButton { "+" };

    // Bookmark list
    juce::ListBox bookmarkList;
    juce::StringArray bookmarkPaths;
    juce::StringArray bookmarkNames;

    // Preview playback
    juce::AudioFormatManager previewFormatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> previewSource;
    juce::AudioTransportSource previewTransport;

    juce::Label targetPadLabel;
    juce::Label currentFolderLabel;

    void loadSelectedToTargetPad();
    void togglePreview();
    void stopPreview();
    void navigateToBookmark (int index);
    void applySearchFilter();

    class BookmarkListModel : public juce::ListBoxModel
    {
    public:
        BookmarkListModel (SoundBrowser& owner) : owner (owner) {}
        int getNumRows() override { return owner.bookmarkNames.size(); }
        void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
    private:
        SoundBrowser& owner;
    };

    BookmarkListModel bookmarkModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBrowser)
};
