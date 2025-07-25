// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "xBrowser/browser/window_test.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "include/base/cef_callback.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "xBrowser/browser/main_context.h"
#include "xBrowser/browser/test_runner.h"
#include "xBrowser/browser/window_test_runner.h"

#if defined(OS_WIN) || defined(OS_LINUX)
#include "xBrowser/browser/window_test_runner_views.h"
#endif

#if defined(OS_WIN)
#include "xBrowser/browser/window_test_runner_win.h"
#elif defined(OS_LINUX)
#include "xBrowser/browser/window_test_runner_gtk.h"
#elif defined(OS_MAC)
#include "xBrowser/browser/window_test_runner_mac.h"
#endif

namespace client {
namespace window_test {

namespace {

const char kTestUrlPath[] = "/window";
const char kMessagePositionName[] = "WindowTest.Position";
const char kMessageMinimizeName[] = "WindowTest.Minimize";
const char kMessageMaximizeName[] = "WindowTest.Maximize";
const char kMessageRestoreName[] = "WindowTest.Restore";

// Create the appropriate platform test runner object.
std::unique_ptr<WindowTestRunner> CreateWindowTestRunner() {
#if defined(OS_WIN) || defined(OS_LINUX)
  if (MainContext::Get()->UseViews())
    return std::make_unique<WindowTestRunnerViews>();
#endif

#if defined(OS_WIN)
  return std::make_unique<WindowTestRunnerWin>();
#elif defined(OS_LINUX)
  return std::make_unique<WindowTestRunnerGtk>();
#elif defined(OS_MAC)
  return std::make_unique<WindowTestRunnerMac>();
#else
#error "No implementation available for your platform."
#endif
}

// Handle messages in the browser process.
class Handler : public CefMessageRouterBrowserSide::Handler {
 public:
  Handler() : runner_(CreateWindowTestRunner()) {}

  // Called due to cefBroadcast execution in window.html.
  virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       int64 query_id,
                       const CefString& request,
                       bool persistent,
                       CefRefPtr<Callback> callback) override {
    // Only handle messages from the test URL.
    const std::string& url = frame->GetURL();
    if (!test_runner::IsTestURL(url, kTestUrlPath))
      return false;

    const std::string& message_name = request;
    if (message_name.find(kMessagePositionName) == 0) {
      // Parse the comma-delimited list of integer values.
      std::vector<int> vec;
      const std::string& vals =
          message_name.substr(sizeof(kMessagePositionName));
      std::stringstream ss(vals);
      int i;
      while (ss >> i) {
        vec.push_back(i);
        if (ss.peek() == ',')
          ss.ignore();
      }

      if (vec.size() == 4) {
        // Execute SetPos() on the main thread.
        runner_->SetPos(browser, vec[0], vec[1], vec[2], vec[3]);
      }
    } else if (message_name == kMessageMinimizeName) {
      // Execute Minimize() on the main thread.
      runner_->Minimize(browser);
    } else if (message_name == kMessageMaximizeName) {
      // Execute Maximize() on the main thread.
      runner_->Maximize(browser);
    } else if (message_name == kMessageRestoreName) {
      // Execute Restore() on the main thread.
      runner_->Restore(browser);
    } else {
      NOTREACHED();
    }

    callback->Success("");
    return true;
  }

 private:
  std::unique_ptr<WindowTestRunner> runner_;
};

}  // namespace

void CreateMessageHandlers(test_runner::MessageHandlerSet& handlers) {
  handlers.insert(new Handler());
}

}  // namespace window_test
}  // namespace client
