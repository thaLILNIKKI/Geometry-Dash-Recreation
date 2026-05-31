// MultilineBitmapFont.cpp (полный файл с добавленными методами)
#include "MultilineBitmapFont.h"
#include <algorithm>

USING_NS_CC;

// ============================================================================
// MultilineBitmapFont implementation
// ============================================================================

MultilineBitmapFont::MultilineBitmapFont()
    : m_coloredSections(nullptr)
    , m_instantSections(nullptr)
    , m_delaySections(nullptr)
    , m_lineCharacters(nullptr)
    , m_plainText(false)
    , m_contentWidth(0.0f)
    , m_contentHeight(0.0f)
    , m_finalLineWidth(0.0f)
{
    memset(m_charWidths, 0, sizeof(m_charWidths));
    m_contentOffset = CCPointZero;
}

MultilineBitmapFont::~MultilineBitmapFont()
{
    if (m_coloredSections)
        m_coloredSections->release();
    if (m_instantSections)
        m_instantSections->release();
    if (m_delaySections)
        m_delaySections->release();
    if (m_lineCharacters)
        m_lineCharacters->release();
}

bool MultilineBitmapFont::initWithFont(const char* font, std::string text, float separation,
    float maxWidth, cocos2d::CCPoint anchorPoint,
    int lineSeparation, bool plainText)
{
    if (!CCNode::init())
        return false;

    m_plainText = plainText;

    // Create arrays for sections
    m_coloredSections = CCArray::create();
    m_coloredSections->retain();

    m_instantSections = CCArray::create();
    m_instantSections->retain();

    m_delaySections = CCArray::create();
    m_delaySections->retain();

    m_lineCharacters = CCArray::create();
    m_lineCharacters->retain();

    // Get font and load character widths
    BitmapFontCache* cache = BitmapFontCache::sharedFontCache();
    FontObject* fontObj = nullptr;
    if (cache)
        fontObj = cache->fontWithConfigFile(font, 1.0f);

    // Load widths for first 300 characters
    for (int i = 0; i < 300; i++)
    {
        if (fontObj)
            m_charWidths[i] = fontObj->getFontWidth(i);
        else
            m_charWidths[i] = 10.0f;
    }

    // Process color information if not plain text
    std::string processedText = text;
    if (!plainText)
    {
        readColorInfo(processedText);
    }

    // Split text into lines based on maxWidth
    std::vector<std::string> lines;
    std::string currentLine;
    float currentLineWidth = 0.0f;

    for (size_t i = 0; i < processedText.length(); i++)
    {
        char c = processedText[i];
        float charWidth = (c < 300) ? m_charWidths[(unsigned char)c] : 10.0f;

        if (c == '\n')
        {
            if (!currentLine.empty())
            {
                lines.push_back(currentLine);
                currentLine.clear();
                currentLineWidth = 0.0f;
            }
        }
        else if (maxWidth > 0 && currentLineWidth + charWidth > maxWidth)
        {
            // Find last space for word wrap
            size_t lastSpace = currentLine.find_last_of(' ');
            if (lastSpace != std::string::npos && lastSpace > 0)
            {
                std::string remaining = currentLine.substr(lastSpace + 1);
                currentLine = currentLine.substr(0, lastSpace);
                lines.push_back(currentLine);
                currentLine = remaining;

                // Recalculate width of remaining text
                currentLineWidth = 0.0f;
                for (char rc : currentLine)
                {
                    currentLineWidth += (rc < 300) ? m_charWidths[(unsigned char)rc] : 10.0f;
                }
                currentLineWidth += charWidth;
                currentLine += c;
            }
            else
            {
                lines.push_back(currentLine);
                currentLine.clear();
                currentLineWidth = charWidth;
                currentLine += c;
            }
        }
        else
        {
            currentLineWidth += charWidth;
            currentLine += c;
        }
    }

    if (!currentLine.empty())
    {
        lines.push_back(currentLine);
    }

    // Calculate total height
    float lineHeight = 30.0f;
    float totalHeight = lines.size() * lineHeight + (lines.size() - 1) * separation;
    float maxLineWidth = 0.0f;

    // Calculate max line width
    for (const auto& line : lines)
    {
        float lineWidth = 0.0f;
        for (char c : line)
        {
            lineWidth += (c < 300) ? m_charWidths[(unsigned char)c] : 10.0f;
        }
        maxLineWidth = /*std::*/max(maxLineWidth, lineWidth);
    }

    // Create labels for each line
    float currentY = 0.0f;

    for (size_t i = 0; i < lines.size(); i++)
    {
        CCLabelBMFont* label = CCLabelBMFont::create(lines[i].c_str(), font);
        if (label)
        {
            label->setAnchorPoint(anchorPoint);

            float xPos = 0.0f;
            if (anchorPoint.x == 0.5f)
                xPos = maxLineWidth / 2.0f;
            else if (anchorPoint.x == 1.0f)
                xPos = maxLineWidth;

            label->setPosition(CCPointMake(xPos, -currentY));

            this->addChild(label);

            currentY += lineHeight + separation;
        }
    }

    // Set content dimensions
    m_contentWidth = maxLineWidth;
    m_contentHeight = totalHeight;
    m_finalLineWidth = maxLineWidth;
    m_contentOffset = CCPointMake(0, 0);

    return true;
}

void MultilineBitmapFont::setOpacity(GLubyte opacity)
{
    CCArray* children = this->getChildren();
    if (children)
    {
        for (unsigned int i = 0; i < children->count(); i++)
        {
            CCNode* child = static_cast<CCNode*>(children->objectAtIndex(i));
            if (child)
            {
                CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(child);
                if (label)
                {
                    label->setOpacity(opacity);
                }
            }
        }
    }
}

void MultilineBitmapFont::setColor(const ccColor3B& color)
{
    CCArray* children = this->getChildren();
    if (children)
    {
        for (unsigned int i = 0; i < children->count(); i++)
        {
            CCNode* child = static_cast<CCNode*>(children->objectAtIndex(i));
            if (child)
            {
                CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(child);
                if (label)
                {
                    label->setColor(color);
                }
            }
        }
    }
}

void MultilineBitmapFont::readColorInfo(std::string& text)
{
    size_t pos = 0;
    while ((pos = text.find('<', pos)) != std::string::npos)
    {
        size_t endTag = text.find('>', pos);
        if (endTag == std::string::npos) break;

        std::string tag = text.substr(pos, endTag - pos + 1);

        // Handle color tags <cX>
        if (tag.substr(0, 2) == "<c" && tag.length() >= 3)
        {
            char colorCode = tag[2];
            ccColor3B color;

            switch (colorCode)
            {
            case 'b':  // Blue
                color.r = 0x4A; color.g = 0x52; color.b = 0xE1;
                break;
            case 'g':  // Green
                color.r = 0x40; color.g = 0xE3; color.b = 0x48;
                break;
            case 'l':  // Light blue
                color.r = 0x60; color.g = 0xAB; color.b = 0xEF;
                break;
            case 'j':  // Bright blue
                color.r = 0x32; color.g = 0xC8; color.b = 0xFF;
                break;
            case 'y':  // Yellow
                color.r = 0xFF; color.g = 0xFF; color.b = 0x00;
                break;
            case 'o':  // Orange
                color.r = 0xA5; color.g = 0x4B; color.b = 0x00;
                break;
            case 'r':  // Red
                color.r = 0x5A; color.g = 0x00; color.b = 0x00;
                break;
            case 'p':  // Purple
                color.r = 0x00; color.g = 0x00; color.b = 0x00;
                break;
            default:
                pos++;
                continue;
            }

            size_t closeTag = text.find("</c>", endTag);
            if (closeTag != std::string::npos)
            {
                int start = endTag + 1;
                int end = closeTag;

                if (m_coloredSections)
                {
                    ColoredSection* section = ColoredSection::create(color, start, end);
                    if (section)
                    {
                        m_coloredSections->addObject(section);
                    }
                }

                text.erase(closeTag, 4);
                text.erase(pos, endTag - pos + 1);
            }
            else
            {
                pos++;
            }
        }
        // Handle italic tags <i>
        else if (tag == "<i>")
        {
            size_t closeTag = text.find("</i>", pos);
            if (closeTag != std::string::npos)
            {
                int start = pos + 3;
                int end = closeTag;

                if (m_instantSections)
                {
                    InstantSection* section = InstantSection::create(start, end);
                    if (section)
                    {
                        m_instantSections->addObject(section);
                    }
                }

                text.erase(closeTag, 4);
                text.erase(pos, 3);
            }
            else
            {
                pos++;
            }
        }
        // Handle delay tags <dXX>
        else if (tag.substr(0, 2) == "<d" && tag[tag.length() - 1] == '>')
        {
            std::string delayStr = tag.substr(2, tag.length() - 3);
            int delayValue = atoi(delayStr.c_str());
            float delay = delayValue / 100.0f;

            if (m_delaySections)
            {
                DelaySection* section = DelaySection::create(endTag + 1, delay);
                if (section)
                {
                    m_delaySections->addObject(section);
                }
            }

            text.erase(pos, endTag - pos + 1);
        }
        else
        {
            pos++;
        }
    }
}

std::string MultilineBitmapFont::stringWithMaxWidth(std::string text, float maxWidth, float separation)
{
    std::string result;
    float currentWidth = 0.0f;
    std::string currentLine;

    for (size_t i = 0; i < text.length(); i++)
    {
        char c = text[i];
        float charWidth = (c < 300) ? m_charWidths[(unsigned char)c] : 10.0f;

        if (c == '\n' || (maxWidth > 0 && currentWidth + charWidth > maxWidth))
        {
            if (!currentLine.empty())
            {
                if (!result.empty()) result += '\n';
                result += currentLine;
                currentLine.clear();
                currentWidth = 0.0f;
            }

            if (c != '\n')
            {
                currentWidth += charWidth;
                currentLine += c;
            }
        }
        else
        {
            currentWidth += charWidth;
            currentLine += c;
        }
    }

    if (!currentLine.empty())
    {
        if (!result.empty()) result += '\n';
        result += currentLine;
    }

    return result;
}

void MultilineBitmapFont::moveSpecialDescriptors(int pos, int length)
{
    if (m_coloredSections)
    {
        for (int i = 0; i < m_coloredSections->count(); i++)
        {
            ColoredSection* section = static_cast<ColoredSection*>(m_coloredSections->objectAtIndex(i));
            if (section)
            {
                if (section->m_startIndex >= pos)
                    section->m_startIndex += length;
                if (section->m_endIndex >= pos)
                    section->m_endIndex += length;
            }
        }
    }

    if (m_instantSections)
    {
        for (int i = 0; i < m_instantSections->count(); i++)
        {
            InstantSection* section = static_cast<InstantSection*>(m_instantSections->objectAtIndex(i));
            if (section)
            {
                if (section->m_startIndex >= pos)
                    section->m_startIndex += length;
                if (section->m_endIndex >= pos)
                    section->m_endIndex += length;
            }
        }
    }

    if (m_delaySections)
    {
        for (int i = 0; i < m_delaySections->count(); i++)
        {
            DelaySection* section = static_cast<DelaySection*>(m_delaySections->objectAtIndex(i));
            if (section && section->m_startIndex >= pos)
                section->m_startIndex += length;
        }
    }
}

MultilineBitmapFont* MultilineBitmapFont::createWithFont(const char* font, std::string text,
    float separation, float maxWidth,
    cocos2d::CCPoint anchorPoint,
    int lineSeparation, bool plainText)
{
    MultilineBitmapFont* pRet = new MultilineBitmapFont();
    if (pRet && pRet->initWithFont(font, text, separation, maxWidth, anchorPoint, lineSeparation, plainText))
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

// ============================================================================
// ColoredSection implementation
// ============================================================================

ColoredSection* ColoredSection::create(ccColor3B color, int start, int end)
{
    ColoredSection* pRet = new ColoredSection();
    if (pRet)
    {
        pRet->m_color = color;
        pRet->m_startIndex = start;
        pRet->m_endIndex = end;
        pRet->autorelease();
    }
    return pRet;
}

ColoredSection::~ColoredSection()
{
}

// ============================================================================
// InstantSection implementation
// ============================================================================

InstantSection* InstantSection::create(int start, int end)
{
    InstantSection* pRet = new InstantSection();
    if (pRet)
    {
        pRet->m_startIndex = start;
        pRet->m_endIndex = end;
        pRet->autorelease();
    }
    return pRet;
}

InstantSection::~InstantSection()
{
}

// ============================================================================
// DelaySection implementation
// ============================================================================

DelaySection* DelaySection::create(int start, float delay)
{
    DelaySection* pRet = new DelaySection();
    if (pRet)
    {
        pRet->m_startIndex = start;
        pRet->m_delay = delay;
        pRet->autorelease();
    }
    return pRet;
}

DelaySection::~DelaySection()
{
}

// ============================================================================
// FontObject implementation
// ============================================================================

FontObject::FontObject()
{
    memset(m_charWidths, 0, sizeof(m_charWidths));
}

FontObject::~FontObject()
{
}

bool FontObject::initWithConfigFile(const char* filename, float scale)
{
    return parseConfigFile(filename, scale);
}

bool FontObject::parseConfigFile(const char* filename, float scale)
{
    if (!filename) return false;

    // Load file content
    CCString* content = CCString::createWithContentsOfFile(filename);
    if (!content) return false;

    std::string fileContent = content->getCString();
    if (fileContent.empty()) return false;

    // Find "chars" section
    size_t charsPos = fileContent.find("chars");
    if (charsPos == std::string::npos) return false;

    // Find the line with character definitions
    size_t lineStart = fileContent.find('\n', charsPos);
    if (lineStart == std::string::npos) return false;

    // Parse each character line
    size_t pos = lineStart + 1;
    while (pos < fileContent.length())
    {
        size_t lineEnd = fileContent.find('\n', pos);
        if (lineEnd == std::string::npos) lineEnd = fileContent.length();

        std::string line = fileContent.substr(pos, lineEnd - pos);

        // Look for "char id="
        size_t idPos = line.find("id=");
        if (idPos != std::string::npos)
        {
            // Extract character ID
            idPos += 3; // skip "id="
            size_t idEnd = line.find(' ', idPos);
            if (idEnd == std::string::npos) idEnd = line.length();
            std::string idStr = line.substr(idPos, idEnd - idPos);
            int charId = atoi(idStr.c_str());

            // Look for "x=", "y=", "width=", "height=", "xoffset=", "yoffset=", "xadvance="
            size_t widthPos = line.find("width=");
            if (widthPos != std::string::npos)
            {
                widthPos += 6; // skip "width="
                size_t widthEnd = line.find(' ', widthPos);
                if (widthEnd == std::string::npos) widthEnd = line.length();
                std::string widthStr = line.substr(widthPos, widthEnd - widthPos);
                int width = atoi(widthStr.c_str());

                // Store width (index + 8 as in disassembly)
                if (charId >= 0 && charId < 300)
                {
                    m_charWidths[charId + 8] = static_cast<float>(width) * scale;
                }
            }
        }

        pos = lineEnd + 1;
        if (lineEnd >= fileContent.length()) break;

        // Stop if we hit a new section
        if (fileContent.find("kerning", pos) != std::string::npos)
            break;
    }

    return true;
}

float FontObject::getFontWidth(int charCode)
{
    int index = charCode + 8;
    if (index >= 0 && index < 300)
        return m_charWidths[index];
    return 10.0f;
}

FontObject* FontObject::createWithConfigFile(const char* filename, float scale)
{
    FontObject* pRet = new FontObject();
    if (pRet && pRet->initWithConfigFile(filename, scale))
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

// ============================================================================
// BitmapFontCache implementation
// ============================================================================

BitmapFontCache* BitmapFontCache::s_sharedFontCache = nullptr;

BitmapFontCache::BitmapFontCache()
    : m_fontCache(nullptr)
{
}

BitmapFontCache::~BitmapFontCache()
{
    if (m_fontCache)
        m_fontCache->release();
}

bool BitmapFontCache::init()
{
    m_fontCache = CCDictionary::create();
    if (m_fontCache)
    {
        m_fontCache->retain();
        return true;
    }
    return false;
}

FontObject* BitmapFontCache::fontWithConfigFile(const char* filename, float scale)
{
    std::string key = std::string(filename) + std::to_string(scale);

    if (m_fontCache)
    {
        FontObject* cached = static_cast<FontObject*>(m_fontCache->objectForKey(key));
        if (cached)
            return cached;
    }

    FontObject* font = FontObject::createWithConfigFile(filename, scale);
    if (font && m_fontCache)
    {
        m_fontCache->setObject(font, key);
    }

    return font;
}

void BitmapFontCache::purgeSharedFontCache()
{
    if (s_sharedFontCache)
    {
        s_sharedFontCache->release();
        s_sharedFontCache = nullptr;
    }
}

BitmapFontCache* BitmapFontCache::sharedFontCache()
{
    if (!s_sharedFontCache)
    {
        s_sharedFontCache = new BitmapFontCache();
        if (s_sharedFontCache && s_sharedFontCache->init())
        {
            s_sharedFontCache->autorelease();
        }
        else
        {
            delete s_sharedFontCache;
            s_sharedFontCache = nullptr;
        }
    }
    return s_sharedFontCache;
}