// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_WINDOW_TEST_RUNNER_VIEWS_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_WINDOW_TEST_RUNNER_VIEWS_H_
#pragma once

#include "xBrowser/browser/window_test_runner.h"

namespace client {
namespace window_test {

// Views platform implementation.
class WindowTestRunnerViews : public WindowTestRunner {
 public:
  WindowTestRunnerViews();

  void SetPos(CefRefPtr<CefBrowser> browser,
              int x,
              int y,
              int width,
              int height) override;
  void Minimize(CefRefPtr<CefBrowser> browser) override;
  void Maximize(CefRefPtr<CefBrowser> browser) override;
  void Restore(CefRefPtr<CefBrowser> browser) override;
};

}  // namespace window_test
}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_WINDOW_TEST_RUNNER_VIEWS_H_
