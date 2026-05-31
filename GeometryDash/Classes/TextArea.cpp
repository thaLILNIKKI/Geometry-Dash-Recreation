// TextArea.cpp (исправленный)
#include "TextArea.h"
#include <cmath>
#include <cctype>

USING_NS_CC;

TextArea::TextArea()
    : m_ignoreColorCode(false)
    , m_pMultilineFont(nullptr)
    , m_separation(0.0f)
    , m_fontName(nullptr)
    , m_lineSeparation(0.0f)
    , m_finishedFade(false)
    , m_maxWidth(0.0f)
    , m_showAllFlag(false)
    , m_textHeight(0.0f)
    , m_textWidth(0.0f)
    , m_finalLineWidth(0.0f)
    , m_currentAction(nullptr)
{
    m_anchorPoint = CCPointZero;
    m_textOffset = CCPointZero;
    m_fontName = new std::string();
}

TextArea::~TextArea()
{
    if (m_pMultilineFont)
    {
        m_pMultilineFont->release();
        m_pMultilineFont = nullptr;
    }
    if (m_fontName)
    {
        delete m_fontName;
        m_fontName = nullptr;
    }
}

bool TextArea::init(std::string s, const char* font, float separation, float maxWidth,
    cocos2d::CCPoint anchorPoint, float lineSeparation, bool plainText)
{
    if (!CCSprite::init())
        return false;

    m_separation = separation;
    m_anchorPoint = anchorPoint;
    m_maxWidth = maxWidth;
    m_lineSeparation = lineSeparation;
    m_ignoreColorCode = plainText;

    // Store font name
    if (m_fontName)
        *m_fontName = font;

    // Set initial string
    setString(s);

    return true;
}

void TextArea::draw()
{
    // Intentionally empty - drawing is handled by children
}

void TextArea::setString(std::string s)
{
    // Check if string is empty or only whitespace
    bool isEmptyOrWhitespace = true;
    for (char c : s)
    {
        if (c != ' ' && c != '\t' && c != '\n')
        {
            isEmptyOrWhitespace = false;
            break;
        }
    }

    if (isEmptyOrWhitespace)
    {
        s = " ";
    }

    // Remove existing multiline font
    if (m_pMultilineFont)
    {
        hideAll();
        m_pMultilineFont->removeFromParentAndCleanup(true);
        m_pMultilineFont->release();
        m_pMultilineFont = nullptr;
    }

    // Create new multiline bitmap font
    m_pMultilineFont = MultilineBitmapFont::createWithFont(
        m_fontName->c_str(),
        s,
        m_separation,
        m_maxWidth,
        m_anchorPoint,
        static_cast<int>(m_lineSeparation),
        m_ignoreColorCode
    );

    if (m_pMultilineFont)
    {
        m_pMultilineFont->retain();

        // Get dimensions from the created font
        m_textWidth = m_pMultilineFont->getContentWidth();
        m_textHeight = m_pMultilineFont->getContentHeight();
        m_finalLineWidth = m_pMultilineFont->getFinalLineWidth();
        m_textOffset = m_pMultilineFont->getContentOffset();

        // Set texture rect for the sprite
        CCRect rect = CCRectMake(0, 0, m_textWidth, m_textHeight);
        this->setTextureRect(rect);

        // Add as child
        this->addChild(m_pMultilineFont, 1);

        // Calculate position based on anchor point
        float posX = std::roundf(m_textWidth * m_anchorPoint.x);
        float posY = std::roundf(m_textHeight * m_anchorPoint.y);
        CCPoint textPos = CCPointMake(posX, posY);

        m_pMultilineFont->setPosition(textPos);
        m_textOffset = textPos;
    }
}

void TextArea::setOpacity(GLubyte opacity)
{
    CCSprite::setOpacity(opacity);
    if (m_pMultilineFont)
    {
        m_pMultilineFont->setOpacity(opacity);
    }
}

void TextArea::setIgnoreColorCode(bool ignore)
{
    m_ignoreColorCode = ignore;
    if (m_pMultilineFont)
    {
        m_pMultilineFont->setPlainText(ignore);
    }
}

void TextArea::showAll()
{
    stopAllCharacterActions();
    if (m_pMultilineFont)
    {
        m_pMultilineFont->setVisible(true);
        this->setOpacity(255);
        m_pMultilineFont->setOpacity(255);
    }
    m_showAllFlag = true;
}

void TextArea::hideAll()
{
    stopAllCharacterActions();
    if (m_pMultilineFont)
    {
        this->setOpacity(0);
        m_pMultilineFont->setOpacity(0);
    }
}

void TextArea::finishFade()
{
    m_showAllFlag = true;
    if (m_currentAction)
    {
        m_currentAction = nullptr;
    }
}

void TextArea::fadeIn(float duration, bool wait)
{
    this->stopAllActions();
    stopAllCharacterActions();

    if (m_pMultilineFont)
    {
        m_pMultilineFont->setOpacity(0);
    }
    this->setOpacity(0);
    m_showAllFlag = false;

    CCFadeIn* fadeIn = CCFadeIn::create(duration);

    if (wait)
    {
        CCDelayTime* delay = CCDelayTime::create(duration);
        CCCallFunc* finishCall = CCCallFunc::create(this, callfunc_selector(TextArea::finishFade));
        CCSequence* sequence = CCSequence::create(fadeIn, delay, finishCall, nullptr);
        this->runAction(sequence);
    }
    else
    {
        this->runAction(fadeIn);
        finishFade();
    }
}

void TextArea::fadeOut(float duration)
{
    this->stopAllActions();
    stopAllCharacterActions();

    // Store current opacity for tween
    GLubyte currentOpacity = this->getOpacity();

    CCActionTween* tween = CCActionTween::create(duration, "opacity",
        static_cast<float>(currentOpacity), 0.0f);
    this->runAction(tween);
}

void TextArea::fadeOutAndRemove()
{
    float duration = 0.3f;
    fadeOut(duration);

    CCDelayTime* delay = CCDelayTime::create(duration);
    CCCallFunc* removeCall = CCCallFunc::create(this, callfunc_selector(TextArea::removeFromParentAndCleanup));
    CCSequence* sequence = CCSequence::create(delay, removeCall, nullptr);
    this->runAction(sequence);
}

void TextArea::fadeInCharacters(float duration, float delayBetween)
{
    if (!m_pMultilineFont) return;

    // Get all character nodes from the multiline font
    CCArray* lines = m_pMultilineFont->getChildren();
    if (!lines) return;

    float currentDelay = 0.0f;
    int totalChars = 0;

    for (unsigned int i = 0; i < lines->count(); i++)
    {
        CCNode* line = static_cast<CCNode*>(lines->objectAtIndex(i));
        if (!line) continue;

        CCArray* characters = line->getChildren();
        if (!characters) continue;

        for (unsigned int j = 0; j < characters->count(); j++)
        {
            CCNode* characterNode = static_cast<CCNode*>(characters->objectAtIndex(j));
            if (!characterNode) continue;

            // Try to cast to CCSprite or CCLabelBMFont which have setOpacity
            CCSprite* character = dynamic_cast<CCSprite*>(characterNode);
            if (!character)
            {
                CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(characterNode);
                if (label)
                {
                    label->stopAllActions();
                    label->setOpacity(0);

                    float charDelay = currentDelay + (totalChars * delayBetween);
                    CCDelayTime* delay = CCDelayTime::create(charDelay);
                    CCFadeIn* fadeIn = CCFadeIn::create(duration);
                    CCSequence* sequence = CCSequence::create(delay, fadeIn, nullptr);
                    label->runAction(sequence);
                }
                totalChars++;
                continue;
            }

            character->stopAllActions();
            character->setOpacity(0);

            // Calculate delay for this character
            float charDelay = currentDelay + (totalChars * delayBetween);

            CCDelayTime* delay = CCDelayTime::create(charDelay);
            CCFadeIn* fadeIn = CCFadeIn::create(duration);
            CCSequence* sequence = CCSequence::create(delay, fadeIn, nullptr);
            character->runAction(sequence);

            totalChars++;
        }

        currentDelay += duration;
    }

    // Schedule finish callback after all characters are done
    float totalDuration = currentDelay + duration;
    CCDelayTime* finalDelay = CCDelayTime::create(totalDuration);
    CCCallFunc* finishCall = CCCallFunc::create(this, callfunc_selector(TextArea::finishFade));
    CCSequence* finalSequence = CCSequence::create(finalDelay, finishCall, nullptr);
    this->runAction(finalSequence);
}

void TextArea::colorAllCharactersTo(ccColor3B color)
{
    if (!m_pMultilineFont) return;

    CCArray* lines = m_pMultilineFont->getChildren();
    if (!lines) return;

    for (unsigned int i = 0; i < lines->count(); i++)
    {
        CCNode* line = static_cast<CCNode*>(lines->objectAtIndex(i));
        if (!line) continue;

        CCArray* characters = line->getChildren();
        if (!characters) continue;

        for (unsigned int j = 0; j < characters->count(); j++)
        {
            CCNode* characterNode = static_cast<CCNode*>(characters->objectAtIndex(j));
            if (!characterNode) continue;

            // Try to cast to CCSprite or CCLabelBMFont which have setColor
            CCSprite* character = dynamic_cast<CCSprite*>(characterNode);
            if (character)
            {
                character->setColor(color);
            }
            else
            {
                CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(characterNode);
                if (label)
                {
                    label->setColor(color);
                }
            }
        }
    }
}

void TextArea::stopAllCharacterActions()
{
    if (!m_pMultilineFont) return;

    CCArray* lines = m_pMultilineFont->getChildren();
    if (!lines) return;

    for (unsigned int i = 0; i < lines->count(); i++)
    {
        CCNode* line = static_cast<CCNode*>(lines->objectAtIndex(i));
        if (!line) continue;

        line->stopAllActions();

        CCArray* characters = line->getChildren();
        if (!characters) continue;

        for (unsigned int j = 0; j < characters->count(); j++)
        {
            CCNode* characterNode = static_cast<CCNode*>(characters->objectAtIndex(j));
            if (!characterNode) continue;

            characterNode->stopAllActions();
        }
    }
}

TextArea* TextArea::create(std::string s, const char* font, float separation, float maxWidth,
    cocos2d::CCPoint anchorPoint, float lineSeparation, bool plainText)
{
    TextArea* pRet = new TextArea();
    if (pRet && pRet->init(s, font, separation, maxWidth, anchorPoint, lineSeparation, plainText))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        return nullptr;
    }
}