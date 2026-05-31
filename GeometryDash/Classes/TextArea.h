// TextArea.h
#ifndef __TEXTAREA_H__
#define __TEXTAREA_H__

#include "cocos2d.h"
#include "MultilineBitmapFont.h"
#include <string>

class TextArea : public cocos2d::CCSprite
{
public:
    TextArea();
    virtual ~TextArea();

    bool init(std::string s, const char* font, float separation, float maxWidth,
        cocos2d::CCPoint anchorPoint, float lineSeparation, bool plainText);

    void draw() override;
    void setString(std::string s);
    void setOpacity(GLubyte opacity) override;
    void setIgnoreColorCode(bool ignore);

    void showAll();
    void hideAll();
    void finishFade();
    void fadeIn(float duration, bool wait);
    void fadeOut(float duration);
    void fadeOutAndRemove();
    void fadeInCharacters(float duration, float delayBetween);
    void colorAllCharactersTo(cocos2d::ccColor3B color);
    void stopAllCharacterActions();

    static TextArea* create(std::string s, const char* font, float separation, float maxWidth,
        cocos2d::CCPoint anchorPoint, float lineSeparation, bool plainText);

    // Offsets based on disassembly
    bool m_ignoreColorCode;                    // offset 0x1E4
    MultilineBitmapFont* m_pMultilineFont;     // offset 0x1E8
    float m_separation;                        // offset 0x1EC
    std::string* m_fontName;                   // offset 0x1F4 (String storage)
    float m_lineSeparation;                    // offset 0x1F8
    bool m_finishedFade;                       // offset 0x1FC (byte)
    cocos2d::CCPoint m_anchorPoint;            // offset 0x200
    float m_maxWidth;                          // offset 0x20C
    bool m_showAllFlag;                        // offset 0x208
    float m_textHeight;                        // offset 0x210
    float m_textWidth;                         // offset 0x214
    float m_finalLineWidth;                    // offset 0x218
    cocos2d::CCPoint m_textOffset;             // offset 0x21C
    cocos2d::CCAction* m_currentAction;        // offset 0x224
};

#endif // __TEXTAREA_H__