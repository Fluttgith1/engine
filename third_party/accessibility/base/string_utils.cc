// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "string_utils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <codecvt>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>

#include "third_party/dart/runtime/third_party/double-conversion/src/double-conversion.h"

#if defined(_WIN32)
#include "base/win/string_conversion.h"
#endif

#include "no_destructor.h"

namespace base {

namespace {
char const kExponentChar = 'e';
const char* const kInfinitySymbol = "Infinity";
const char* const kNaNSymbol = "NaN";

// Strip any trailing zeros, including the decimal marker if there are nothing
// but zeros after the decimal.
std::string StripTrailingZeros(const std::string& str) {
  int strip = 0;
  for (auto it = str.rbegin(); it != str.rend() - 1; ++it) {
    if (*it == '0') {
      strip++;
    } else if (*it == '.' || *it == ',') {
      strip++;
      // Don't keep stripping once we hit a decimal marker.
      break;
    } else {
      break;
    }
  }
  if (strip != 0) {
    return str.substr(0, str.length() - strip);
  }
  return str;
}
}  // namespace

using double_conversion::DoubleToStringConverter;
using double_conversion::StringBuilder;

std::u16string ASCIIToUTF16(std::string src) {
  return std::u16string(src.begin(), src.end());
}

std::u16string UTF8ToUTF16(std::string src) {
#if defined(_WIN32)
  return WideToUTF16(win::Utf16FromUtf8(src));
#else
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.from_bytes(src);
#endif
}

std::string UTF16ToUTF8(std::u16string src) {
#if defined(_WIN32)
  return win::Utf8FromUtf16(UTF16ToWide(src));
#else
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.to_bytes(src);
#endif
}

std::u16string WideToUTF16(const std::wstring& src) {
  return std::u16string(src.begin(), src.end());
}

std::wstring UTF16ToWide(const std::u16string& src) {
  return std::wstring(src.begin(), src.end());
}

std::u16string NumberToString16(float number, int precision) {
  return ASCIIToUTF16(NumberToString(number, precision));
}

std::u16string NumberToString16(int32_t number) {
  return ASCIIToUTF16(NumberToString(number));
}

std::u16string NumberToString16(unsigned int number) {
  return ASCIIToUTF16(NumberToString(number));
}

std::u16string NumberToString16(double number, int precision) {
  return ASCIIToUTF16(NumberToString(number, precision));
}

std::string NumberToString(int32_t number) {
  return std::to_string(number);
}

std::string NumberToString(unsigned int number) {
  return std::to_string(number);
}

std::string NumberToString(float number, int precision) {
  return NumberToString(static_cast<double>(number), precision);
}

std::string NumberToString(double number, int precision) {
  if (number == 0.0) {
    return "0";
  }
  static const int kMinPrecisionDigits = 0;
  static const int kMaxPrecisionDigits = 21;
  static const double kMaxFixed = 10e13;
  static const int kConversionFlags = DoubleToStringConverter::NO_FLAGS;
  const int kBufferSize = 128;

  ASSERT(kMinPrecisionDigits <= precision && precision <= kMaxPrecisionDigits);

  const DoubleToStringConverter converter(
      kConversionFlags, kInfinitySymbol, kNaNSymbol, kExponentChar, 0, 0, 0, 0);
  std::vector<char> char_buffer(kBufferSize);
  char_buffer[kBufferSize - 1] = '\0';
  StringBuilder builder(char_buffer.data(), kBufferSize);
  std::string result;
  double magnitude = fabs(number);
  if (magnitude < pow(10, -precision) || magnitude > kMaxFixed) {
    bool status = converter.ToExponential(number, precision, &builder);
    ASSERT(status);
    result = std::string(builder.Finalize());
    size_t exponent_index = result.rfind(kExponentChar);
    // Strip trailing zeros from just the mantissa.
    result = StripTrailingZeros(result.substr(0, exponent_index)) +
             result.substr(exponent_index);
  } else {
    bool status = converter.ToFixed(number, precision, &builder);
    ASSERT(status);

    result = StripTrailingZeros(std::string(builder.Finalize()));
  }
  return result;
}

std::string JoinString(std::vector<std::string> tokens, std::string delimiter) {
  std::ostringstream imploded;
  for (size_t i = 0; i < tokens.size(); i++) {
    if (i == tokens.size() - 1) {
      imploded << tokens[i];
    } else {
      imploded << tokens[i] << delimiter;
    }
  }
  return imploded.str();
}

std::u16string JoinString(std::vector<std::u16string> tokens,
                          std::u16string delimiter) {
  std::u16string result;
  for (size_t i = 0; i < tokens.size(); i++) {
    if (i == tokens.size() - 1) {
      result.append(tokens[i]);
    } else {
      result.append(tokens[i]);
      result.append(delimiter);
    }
  }
  return result;
}

void ReplaceChars(std::string in,
                  std::string from,
                  std::string to,
                  std::string* out) {
  size_t pos = in.find(from);
  while (pos != std::string::npos) {
    in.replace(pos, from.size(), to);
    pos = in.find(from, pos + to.size());
  }
  *out = in;
}

void ReplaceChars(std::u16string in,
                  std::u16string from,
                  std::u16string to,
                  std::u16string* out) {
  size_t pos = in.find(from);
  while (pos != std::u16string::npos) {
    in.replace(pos, from.size(), to);
    pos = in.find(from, pos + to.size());
  }
  *out = in;
}

const std::string& EmptyString() {
  static const base::NoDestructor<std::string> s;
  return *s;
}

std::string ToUpperASCII(std::string str) {
  std::string ret;
  ret.reserve(str.size());
  for (size_t i = 0; i < str.size(); i++)
    ret.push_back(std::toupper(str[i]));
  return ret;
}

std::string ToLowerASCII(std::string str) {
  std::string ret;
  ret.reserve(str.size());
  for (size_t i = 0; i < str.size(); i++)
    ret.push_back(std::tolower(str[i]));
  return ret;
}

bool LowerCaseEqualsASCII(std::string a, std::string b) {
  std::string lower_a = ToLowerASCII(a);
  return lower_a.compare(ToLowerASCII(b)) == 0;
}

bool ContainsOnlyChars(std::u16string str, char16_t ch) {
  return std::find_if(str.begin(), str.end(),
                      [ch](char16_t c) { return c != ch; }) == str.end();
}

}  // namespace base
