/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Minikin"

#include "FontLanguage.h"

#include <hb.h>
#include <unicode/uloc.h>

namespace android {

#define SCRIPT_TAG(c1, c2, c3, c4) \
        ((uint32_t)(c1)) << 24 | ((uint32_t)(c2)) << 16 | ((uint32_t)(c3)) <<  8 | ((uint32_t)(c4))

// Parse BCP 47 language identifier into internal structure
FontLanguage::FontLanguage(const char* buf, size_t length) : FontLanguage() {
    size_t i;
    for (i = 0; i < length; i++) {
        char c = buf[i];
        if (c == '-' || c == '_') break;
    }
    if (i == 2 || i == 3) {  // only accept two or three letter language code.
        mLanguage = buf[0] | (buf[1] << 8) | ((i == 3) ? (buf[2] << 16) : 0);
    } else {
        // We don't understand anything other than two-letter or three-letter
        // language codes, so we skip parsing the rest of the string.
        mLanguage = 0ul;
        return;
    }

    size_t next;
    for (i++; i < length; i = next + 1) {
        for (next = i; next < length; next++) {
            char c = buf[next];
            if (c == '-' || c == '_') break;
        }
        if (next - i == 4 && 'A' <= buf[i] && buf[i] <= 'Z') {
            mScript = SCRIPT_TAG(buf[i], buf[i + 1], buf[i + 2], buf[i + 3]);
        }
    }

    mSubScriptBits = scriptToSubScriptBits(mScript);
}

//static
uint8_t FontLanguage::scriptToSubScriptBits(uint32_t script) {
    uint8_t subScriptBits = 0u;
    switch (script) {
        case SCRIPT_TAG('H', 'a', 'n', 'g'):
            subScriptBits = kHangulFlag;
            break;
        case SCRIPT_TAG('H', 'a', 'n', 'i'):
            subScriptBits = kHanFlag;
            break;
        case SCRIPT_TAG('H', 'a', 'n', 's'):
            subScriptBits = kHanFlag | kSimplifiedChineseFlag;
            break;
        case SCRIPT_TAG('H', 'a', 'n', 't'):
            subScriptBits = kHanFlag | kTraditionalChineseFlag;
            break;
        case SCRIPT_TAG('H', 'i', 'r', 'a'):
            subScriptBits = kHiraganaFlag;
            break;
        case SCRIPT_TAG('H', 'r', 'k', 't'):
            subScriptBits = kKatakanaFlag | kHiraganaFlag;
            break;
        case SCRIPT_TAG('J', 'p', 'a', 'n'):
            subScriptBits = kHanFlag | kKatakanaFlag | kHiraganaFlag;
            break;
        case SCRIPT_TAG('K', 'a', 'n', 'a'):
            subScriptBits = kKatakanaFlag;
            break;
        case SCRIPT_TAG('K', 'o', 'r', 'e'):
            subScriptBits = kHanFlag | kHangulFlag;
            break;
        case SCRIPT_TAG('Q', 'a', 'a', 'e'):
            subScriptBits = kEmojiFlag;
            break;
    }
    return subScriptBits;
}

std::string FontLanguage::getString() const {
    if (mLanguage == 0ul) {
        return "und";
    }
    char buf[16];
    size_t i = 0;
    buf[i++] = mLanguage & 0xFF ;
    buf[i++] = (mLanguage >> 8) & 0xFF;
    char third_letter = (mLanguage >> 16) & 0xFF;
    if (third_letter != 0) buf[i++] = third_letter;
    if (mScript != 0) {
      buf[i++] = '-';
      buf[i++] = (mScript >> 24) & 0xFFu;
      buf[i++] = (mScript >> 16) & 0xFFu;
      buf[i++] = (mScript >> 8) & 0xFFu;
      buf[i++] = mScript & 0xFFu;
    }
    return std::string(buf, i);
}

bool FontLanguage::isEqualScript(const FontLanguage other) const {
    return other.mScript == mScript;
}

bool FontLanguage::supportsHbScript(hb_script_t script) const {
    static_assert(SCRIPT_TAG('J', 'p', 'a', 'n') == HB_TAG('J', 'p', 'a', 'n'),
                  "The Minikin script and HarfBuzz hb_script_t have different encodings.");
    if (script == mScript) return true;
    uint8_t requestedBits = scriptToSubScriptBits(script);
    return requestedBits != 0 && (mSubScriptBits & requestedBits) == requestedBits;
}

int FontLanguage::match(const FontLanguage other) const {
    // TODO: Use script for matching.
    return *this == other;
}

#undef SCRIPT_TAG
}  // namespace android
