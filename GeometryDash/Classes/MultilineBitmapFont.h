// MultilineBitmapFont.h
#ifndef __MULTILINEBITMAPFONT_H__
#define __MULTILINEBITMAPFONT_H__

#include "cocos2d.h"
#include <string>
#include <vector>
#include <cstring>

USING_NS_CC;

class FontObject;
class BitmapFontCache;

// Forward declarations for section classes
class ColoredSection;
class InstantSection;
class DelaySection;

class MultilineBitmapFont : public cocos2d::CCNode
{
public:
    MultilineBitmapFont();
    virtual ~MultilineBitmapFont();

    bool initWithFont(const char* font, std::string text, float separation,
        float maxWidth, cocos2d::CCPoint anchorPoint,
        int lineSeparation, bool plainText);

    static MultilineBitmapFont* createWithFont(const char* font, std::string text,
        float separation, float maxWidth,
        cocos2d::CCPoint anchorPoint,
        int lineSeparation, bool plainText);

    void readColorInfo(std::string& text);
    std::string stringWithMaxWidth(std::string text, float maxWidth, float separation);
    void moveSpecialDescriptors(int pos, int length);

    // Getters
    float getContentWidth() const { return m_contentWidth; }
    float getContentHeight() const { return m_contentHeight; }
    cocos2d::CCPoint getContentOffset() const { return m_contentOffset; }
    float getFinalLineWidth() const { return m_finalLineWidth; }

    // Public methods for TextArea
    void setOpacity(GLubyte opacity);
    void setColor(const ccColor3B& color);
    void setPlainText(bool plainText) { m_plainText = plainText; }
    bool isPlainText() const { return m_plainText; }

    // Arrays for sections (offsets based on disassembly)
    cocos2d::CCArray* m_coloredSections;      // offset 0x694
    cocos2d::CCArray* m_instantSections;      // offset 0x698
    cocos2d::CCArray* m_delaySections;        // offset 0x69C
    cocos2d::CCArray* m_lineCharacters;       // offset 0x6A0

protected:
    float m_charWidths[300];                   // offset 0x1E4
    bool m_plainText;                          // offset 0x6C4

    float m_contentWidth;                      // offset 0x6B4
    float m_contentHeight;                     // offset 0x6B0
    float m_finalLineWidth;                    // offset 0x6C0
    cocos2d::CCPoint m_contentOffset;          // offset 0x6B8
};

// ============================================================================
// ColoredSection class
// ============================================================================
class ColoredSection : public cocos2d::CCObject
{
public:
    static ColoredSection* create(cocos2d::ccColor3B color, int start, int end);
    virtual ~ColoredSection();

    cocos2d::ccColor3B m_color;
    int m_startIndex;
    int m_endIndex;

private:
    ColoredSection() : m_startIndex(0), m_endIndex(0)
    {
        m_color.r = 255;
        m_color.g = 255;
        m_color.b = 255;
    }
};

// ============================================================================
// InstantSection class
// ============================================================================
class InstantSection : public cocos2d::CCObject
{
public:
    static InstantSection* create(int start, int end);
    virtual ~InstantSection();

    int m_startIndex;
    int m_endIndex;

private:
    InstantSection() : m_startIndex(0), m_endIndex(0) {}
};

// ============================================================================
// DelaySection class
// ============================================================================
class DelaySection : public cocos2d::CCObject
{
public:
    static DelaySection* create(int start, float delay);
    virtual ~DelaySection();

    int m_startIndex;
    float m_delay;

private:
    DelaySection() : m_startIndex(0), m_delay(0.0f) {}
};

// ============================================================================
// FontObject class
// ============================================================================
class FontObject : public cocos2d::CCObject
{
public:
    FontObject();
    virtual ~FontObject();

    bool initWithConfigFile(const char* filename, float scale);
    bool parseConfigFile(const char* filename, float scale);
    float getFontWidth(int charCode);

    static FontObject* createWithConfigFile(const char* filename, float scale);

    float m_charWidths[300];  // Array of character widths
};

// ============================================================================
// BitmapFontCache class (singleton)
// ============================================================================
class BitmapFontCache : public cocos2d::CCObject
{
public:
    BitmapFontCache();
    virtual ~BitmapFontCache();

    bool init();
    FontObject* fontWithConfigFile(const char* filename, float scale);
    static void purgeSharedFontCache();
    static BitmapFontCache* sharedFontCache();

    cocos2d::CCDictionary* m_fontCache;  // offset 0x20

private:
    static BitmapFontCache* s_sharedFontCache;
};

#endif // __MULTILINEBITMAPFONT_H__