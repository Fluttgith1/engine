/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <cutils/log.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <hb.h>
#include <hb-ot.h>

#include <utils/JenkinsHash.h>

#include "FontLanguage.h"
#include "FontLanguageListCache.h"
#include "HbFaceCache.h"
#include "MinikinInternal.h"
#include <minikin/MinikinFont.h>
#include <minikin/AnalyzeStyle.h>
#include <minikin/CmapCoverage.h>
#include <minikin/FontFamily.h>
#include <UniquePtr.h>

using std::vector;

namespace android {

FontStyle::FontStyle(int variant, int weight, bool italic)
        : FontStyle(FontLanguageListCache::kEmptyListId, variant, weight, italic) {
}

FontStyle::FontStyle(uint32_t languageListId, int variant, int weight, bool italic)
        : bits(pack(variant, weight, italic)), mLanguageListId(languageListId) {
}

hash_t FontStyle::hash() const {
    uint32_t hash = JenkinsHashMix(0, bits);
    hash = JenkinsHashMix(hash, mLanguageListId);
    return JenkinsHashWhiten(hash);
}

// static
uint32_t FontStyle::registerLanguageList(const std::string& languages) {
    AutoMutex _l(gMinikinLock);
    return FontLanguageListCache::getId(languages);
}

// static
uint32_t FontStyle::pack(int variant, int weight, bool italic) {
    return (weight & kWeightMask) | (italic ? kItalicMask : 0) | (variant << kVariantShift);
}

FontFamily::FontFamily(int variant) : FontFamily(FontLanguageListCache::kEmptyListId, variant) {
}

FontFamily::~FontFamily() {
    for (size_t i = 0; i < mFonts.size(); i++) {
        mFonts[i].typeface->UnrefLocked();
    }
}

bool FontFamily::addFont(MinikinFont* typeface) {
    AutoMutex _l(gMinikinLock);
    const uint32_t os2Tag = MinikinFont::MakeTag('O', 'S', '/', '2');
    size_t os2Size = 0;
    bool ok = typeface->GetTable(os2Tag, NULL, &os2Size);
    if (!ok) return false;
    UniquePtr<uint8_t[]> os2Data(new uint8_t[os2Size]);
    ok = typeface->GetTable(os2Tag, os2Data.get(), &os2Size);
    if (!ok) return false;
    int weight;
    bool italic;
    if (analyzeStyle(os2Data.get(), os2Size, &weight, &italic)) {
        //ALOGD("analyzed weight = %d, italic = %s", weight, italic ? "true" : "false");
        FontStyle style(weight, italic);
        addFontLocked(typeface, style);
        return true;
    } else {
        ALOGD("failed to analyze style");
    }
    return false;
}

void FontFamily::addFont(MinikinFont* typeface, FontStyle style) {
    AutoMutex _l(gMinikinLock);
    addFontLocked(typeface, style);
}

void FontFamily::addFontLocked(MinikinFont* typeface, FontStyle style) {
    typeface->RefLocked();
    mFonts.push_back(Font(typeface, style));
    mCoverageValid = false;
}

// Compute a matching metric between two styles - 0 is an exact match
static int computeMatch(FontStyle style1, FontStyle style2) {
    if (style1 == style2) return 0;
    int score = abs(style1.getWeight() - style2.getWeight());
    if (style1.getItalic() != style2.getItalic()) {
        score += 2;
    }
    return score;
}

static FontFakery computeFakery(FontStyle wanted, FontStyle actual) {
    // If desired weight is semibold or darker, and 2 or more grades
    // higher than actual (for example, medium 500 -> bold 700), then
    // select fake bold.
    int wantedWeight = wanted.getWeight();
    bool isFakeBold = wantedWeight >= 6 && (wantedWeight - actual.getWeight()) >= 2;
    bool isFakeItalic = wanted.getItalic() && !actual.getItalic();
    return FontFakery(isFakeBold, isFakeItalic);
}

FakedFont FontFamily::getClosestMatch(FontStyle style) const {
    const Font* bestFont = NULL;
    int bestMatch = 0;
    for (size_t i = 0; i < mFonts.size(); i++) {
        const Font& font = mFonts[i];
        int match = computeMatch(font.style, style);
        if (i == 0 || match < bestMatch) {
            bestFont = &font;
            bestMatch = match;
        }
    }
    FakedFont result;
    if (bestFont == NULL) {
        result.font = NULL;
    } else {
        result.font = bestFont->typeface;
        result.fakery = computeFakery(style, bestFont->style);
    }
    return result;
}

size_t FontFamily::getNumFonts() const {
    return mFonts.size();
}

MinikinFont* FontFamily::getFont(size_t index) const {
    return mFonts[index].typeface;
}

FontStyle FontFamily::getStyle(size_t index) const {
    return mFonts[index].style;
}

const SparseBitSet* FontFamily::getCoverage() {
    if (!mCoverageValid) {
        const FontStyle defaultStyle;
        MinikinFont* typeface = getClosestMatch(defaultStyle).font;
        const uint32_t cmapTag = MinikinFont::MakeTag('c', 'm', 'a', 'p');
        size_t cmapSize = 0;
        if (!typeface->GetTable(cmapTag, NULL, &cmapSize)) {
            ALOGE("Could not get cmap table size!\n");
            // Note: This means we will retry on the next call to getCoverage, as we can't store
            //       the failure. This is fine, as we assume this doesn't really happen in practice.
            return nullptr;
        }
        UniquePtr<uint8_t[]> cmapData(new uint8_t[cmapSize]);
        if (!typeface->GetTable(cmapTag, cmapData.get(), &cmapSize)) {
            ALOGE("Unexpected failure to read cmap table!\n");
            return nullptr;
        }
        CmapCoverage::getCoverage(mCoverage, cmapData.get(), cmapSize);  // TODO: Error check?
#ifdef VERBOSE_DEBUG
        ALOGD("font coverage length=%d, first ch=%x\n", mCoverage.length(),
                mCoverage.nextSetBit(0));
#endif
        mCoverageValid = true;
    }
    return &mCoverage;
}

bool FontFamily::hasVariationSelector(uint32_t codepoint, uint32_t variationSelector) {
    assertMinikinLocked();
    if (!mHbFont) {
        const FontStyle defaultStyle;
        MinikinFont* minikinFont = getClosestMatch(defaultStyle).font;
        hb_face_t* face = getHbFaceLocked(minikinFont);
        mHbFont = hb_font_create(face);
        hb_ot_font_set_funcs(mHbFont);
    }
    uint32_t unusedGlyph;
    return hb_font_get_glyph(mHbFont, codepoint, variationSelector, &unusedGlyph);
}

void FontFamily::purgeHbFontCache() {
    assertMinikinLocked();
    if (mHbFont) {
        hb_font_destroy(mHbFont);
        mHbFont = nullptr;
    }
}

}  // namespace android
