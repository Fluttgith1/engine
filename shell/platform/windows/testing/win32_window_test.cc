#include "flutter/shell/platform/windows/testing/win32_window_test.h"

#include <iostream>

namespace flutter {
namespace testing {

Win32WindowTest::Win32WindowTest() : Win32Window(){};

Win32WindowTest::~Win32WindowTest() = default;

void Win32WindowTest::OnDpiScale(unsigned int dpi){};

// When DesktopWindow notifies that a WM_Size message has come in
// lets FlutterEngine know about the new size.
void Win32WindowTest::OnResize(unsigned int width, unsigned int height) {}

void Win32WindowTest::OnPointerMove(double x, double y) {}

void Win32WindowTest::OnPointerDown(double x, double y, UINT button) {}

void Win32WindowTest::OnPointerUp(double x, double y, UINT button) {}

void Win32WindowTest::OnPointerLeave() {}

void Win32WindowTest::OnChar(char32_t code_point) {}

void Win32WindowTest::OnKey(int key, int scancode, int action, int mods) {}

void Win32WindowTest::OnScroll(double delta_x, double delta_y) {}

void Win32WindowTest::OnClose() {}

void Win32WindowTest::OnFontChange() {
  std::cerr << "Getting called ======\n";
  on_font_change_called_ = true;
}

UINT Win32WindowTest::GetDpi() {
  return GetCurrentDPI();
}

bool Win32WindowTest::OnFontChangeWasCalled() {
  std::cerr << "on font changed called====s\n";
  return on_font_change_called_;
}

}  // namespace testing
}  // namespace flutter
