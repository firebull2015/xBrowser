// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "xBrowser/browser/root_window_manager.h"

#include <sstream>

#include "include/base/cef_callback.h"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"
#include "xBrowser/browser/client_handler_std.h"
#include "xBrowser/browser/main_context.h"
#include "xBrowser/browser/test_runner.h"
#include "shared/browser/extension_util.h"
#include "shared/browser/file_util.h"
#include "shared/browser/resource_util.h"
#include "shared/common/client_switches.h"

namespace client {

namespace {

class ClientRequestContextHandler : public CefRequestContextHandler,
                                    public CefExtensionHandler {
 public:
  ClientRequestContextHandler() {}

  // CefRequestContextHandler methods:
  void OnRequestContextInitialized(
      CefRefPtr<CefRequestContext> request_context) override {
    CEF_REQUIRE_UI_THREAD();

    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    if (command_line->HasSwitch(switches::kLoadExtension)) {
      if (MainContext::Get()
              ->GetRootWindowManager()
              ->request_context_per_browser()) {
        // The example extension loading implementation requires all browsers to
        // share the same request context.
        LOG(ERROR)
            << "Cannot mix --load-extension and --request-context-per-browser";
        return;
      }

      // Load one or more extension paths specified on the command-line and
      // delimited with semicolon.
      const std::string& extension_path =
          command_line->GetSwitchValue(switches::kLoadExtension);
      if (!extension_path.empty()) {
        std::string part;
        std::istringstream f(extension_path);
        while (getline(f, part, ';')) {
          if (!part.empty())
            extension_util::LoadExtension(request_context, part, this);
        }
      }
    }
  }

  // CefExtensionHandler methods:
  void OnExtensionLoaded(CefRefPtr<CefExtension> extension) override {
    CEF_REQUIRE_UI_THREAD();
    MainContext::Get()->GetRootWindowManager()->AddExtension(extension);
  }

  CefRefPtr<CefBrowser> GetActiveBrowser(CefRefPtr<CefExtension> extension,
                                         CefRefPtr<CefBrowser> browser,
                                         bool include_incognito) override {
    CEF_REQUIRE_UI_THREAD();

    // Return the browser for the active/foreground window.
    CefRefPtr<CefBrowser> active_browser =
        MainContext::Get()->GetRootWindowManager()->GetActiveBrowser();
    if (!active_browser) {
      LOG(WARNING)
          << "No active browser available for extension "
          << browser->GetHost()->GetExtension()->GetIdentifier().ToString();
    } else {
      // The active browser should not be hosting an extension.
      DCHECK(!active_browser->GetHost()->GetExtension());
    }
    return active_browser;
  }

 private:
  IMPLEMENT_REFCOUNTING(ClientRequestContextHandler);
  DISALLOW_COPY_AND_ASSIGN(ClientRequestContextHandler);
};

}  // namespace

RootWindowManager::RootWindowManager(bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed) {
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  DCHECK(command_line.get());
  request_context_per_browser_ =
      command_line->HasSwitch(switches::kRequestContextPerBrowser);
  request_context_shared_cache_ =
      command_line->HasSwitch(switches::kRequestContextSharedCache);
}

RootWindowManager::~RootWindowManager() {
  // All root windows should already have been destroyed.
  DCHECK(root_windows_.empty());
}

scoped_refptr<RootWindow> RootWindowManager::CreateRootWindow(
    std::unique_ptr<RootWindowConfig> config) {
  CefBrowserSettings settings;
  MainContext::Get()->PopulateBrowserSettings(&settings);

  scoped_refptr<RootWindow> root_window =
      RootWindow::Create(MainContext::Get()->UseViews());
  root_window->Init(this, std::move(config), settings);

  // Store a reference to the root window on the main thread.
  OnRootWindowCreated(root_window);

  return root_window;
}

scoped_refptr<RootWindow> RootWindowManager::CreateRootWindowAsPopup(
    bool with_controls,
    bool with_osr,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings) {
  CEF_REQUIRE_UI_THREAD();

  MainContext::Get()->PopulateBrowserSettings(&settings);

  if (MainContext::Get()->UseDefaultPopup()) {
    // Use default window creation for the popup. A new |client| instance is
    // still required by xBrowser architecture.
    client = new ClientHandlerStd(/*delegate=*/nullptr, with_controls,
                                  /*startup_url=*/CefString());
    return nullptr;
  }

  if (!temp_window_) {
    // TempWindow must be created on the UI thread.
    temp_window_.reset(new TempWindow());
  }

  scoped_refptr<RootWindow> root_window =
      RootWindow::Create(MainContext::Get()->UseViews());
  root_window->InitAsPopup(this, with_controls, with_osr, popupFeatures,
                           windowInfo, client, settings);

  // Store a reference to the root window on the main thread.
  OnRootWindowCreated(root_window);

  return root_window;
}

scoped_refptr<RootWindow> RootWindowManager::CreateRootWindowAsExtension(
    CefRefPtr<CefExtension> extension,
    const CefRect& source_bounds,
    CefRefPtr<CefWindow> parent_window,
    base::OnceClosure close_callback,
    bool with_controls,
    bool with_osr) {
  const std::string& extension_url = extension_util::GetExtensionURL(extension);
  if (extension_url.empty()) {
    NOTREACHED() << "Extension cannot be loaded directly.";
    return nullptr;
  }

  // Create an initially hidden browser window that loads the extension URL.
  // We'll show the window when the desired size becomes available via
  // ClientHandler::OnAutoResize.
  auto config = std::make_unique<RootWindowConfig>();
  config->with_controls = with_controls;
  config->with_osr = with_osr;
  config->with_extension = true;
  config->initially_hidden = true;
  config->source_bounds = source_bounds;
  config->parent_window = parent_window;
  config->close_callback = std::move(close_callback);
  config->url = extension_url;
  return CreateRootWindow(std::move(config));
}

bool RootWindowManager::HasRootWindowAsExtension(
    CefRefPtr<CefExtension> extension) {
  REQUIRE_MAIN_THREAD();

  for (auto root_window : root_windows_) {
    if (!root_window->WithExtension())
      continue;

    CefRefPtr<CefBrowser> browser = root_window->GetBrowser();
    if (!browser)
      continue;

    CefRefPtr<CefExtension> browser_extension =
        browser->GetHost()->GetExtension();
    DCHECK(browser_extension);
    if (browser_extension->GetIdentifier() == extension->GetIdentifier())
      return true;
  }

  return false;
}

scoped_refptr<RootWindow> RootWindowManager::GetWindowForBrowser(
    int browser_id) const {
  REQUIRE_MAIN_THREAD();

  for (auto root_window : root_windows_) {
    CefRefPtr<CefBrowser> browser = root_window->GetBrowser();
    if (browser.get() && browser->GetIdentifier() == browser_id)
      return root_window;
  }
  return nullptr;
}

scoped_refptr<RootWindow> RootWindowManager::GetActiveRootWindow() const {
  REQUIRE_MAIN_THREAD();
  return active_root_window_;
}

CefRefPtr<CefBrowser> RootWindowManager::GetActiveBrowser() const {
  base::AutoLock lock_scope(active_browser_lock_);
  return active_browser_;
}

void RootWindowManager::CloseAllWindows(bool force) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(base::BindOnce(&RootWindowManager::CloseAllWindows,
                                     base::Unretained(this), force));
    return;
  }

  if (root_windows_.empty())
    return;

  // Use a copy of |root_windows_| because the original set may be modified
  // in OnRootWindowDestroyed while iterating.
  RootWindowSet root_windows = root_windows_;

  for (auto root_window : root_windows) {
    root_window->Close(force);
  }
}

void RootWindowManager::AddExtension(CefRefPtr<CefExtension> extension) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(base::BindOnce(&RootWindowManager::AddExtension,
                                     base::Unretained(this), extension));
    return;
  }

  // Don't track extensions that can't be loaded directly.
  if (extension_util::GetExtensionURL(extension).empty())
    return;

  // Don't add the same extension multiple times.
  ExtensionSet::const_iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    if ((*it)->GetIdentifier() == extension->GetIdentifier())
      return;
  }

  extensions_.insert(extension);
  NotifyExtensionsChanged();
}

void RootWindowManager::OnRootWindowCreated(
    scoped_refptr<RootWindow> root_window) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(base::BindOnce(&RootWindowManager::OnRootWindowCreated,
                                     base::Unretained(this), root_window));
    return;
  }

  root_windows_.insert(root_window);
  if (!root_window->WithExtension()) {
    root_window->OnExtensionsChanged(extensions_);

    if (root_windows_.size() == 1U) {
      // The first non-extension root window should be considered the active
      // window.
      OnRootWindowActivated(root_window.get());
    }
  }
}

void RootWindowManager::NotifyExtensionsChanged() {
  REQUIRE_MAIN_THREAD();

  for (auto root_window : root_windows_) {
    if (!root_window->WithExtension())
      root_window->OnExtensionsChanged(extensions_);
  }
}

CefRefPtr<CefRequestContext> RootWindowManager::GetRequestContext(
    RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  if (request_context_per_browser_) {
    // Create a new request context for each browser.
    CefRequestContextSettings settings;

    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    if (command_line->HasSwitch(switches::kCachePath)) {
      if (request_context_shared_cache_) {
        // Give each browser the same cache path. The resulting context objects
        // will share the same storage internally.
        CefString(&settings.cache_path) =
            command_line->GetSwitchValue(switches::kCachePath);
      } else {
        // Give each browser a unique cache path. This will create completely
        // isolated context objects.
        std::stringstream ss;
        ss << command_line->GetSwitchValue(switches::kCachePath).ToString()
           << file_util::kPathSep << time(nullptr);
        CefString(&settings.cache_path) = ss.str();
      }
    }

    return CefRequestContext::CreateContext(settings,
                                            new ClientRequestContextHandler);
  }

  // All browsers will share the global request context.
  if (!shared_request_context_.get()) {
    shared_request_context_ = CefRequestContext::CreateContext(
        CefRequestContext::GetGlobalContext(), new ClientRequestContextHandler);
  }
  return shared_request_context_;
}

scoped_refptr<ImageCache> RootWindowManager::GetImageCache() {
  CEF_REQUIRE_UI_THREAD();

  if (!image_cache_) {
    image_cache_ = new ImageCache;
  }
  return image_cache_;
}

void RootWindowManager::OnTest(RootWindow* root_window, int test_id) {
  REQUIRE_MAIN_THREAD();

  test_runner::RunTest(root_window->GetBrowser(), test_id);
}

void RootWindowManager::OnExit(RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  CloseAllWindows(false);
}

void RootWindowManager::OnRootWindowDestroyed(RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  RootWindowSet::iterator it = root_windows_.find(root_window);
  DCHECK(it != root_windows_.end());
  if (it != root_windows_.end())
    root_windows_.erase(it);

  if (root_window == active_root_window_) {
    active_root_window_ = nullptr;

    base::AutoLock lock_scope(active_browser_lock_);
    active_browser_ = nullptr;
  }

  if (terminate_when_all_windows_closed_ && root_windows_.empty()) {
    // All windows have closed. Clean up on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&RootWindowManager::CleanupOnUIThread,
                                       base::Unretained(this)));
  }
}

void RootWindowManager::OnRootWindowActivated(RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  if (root_window->WithExtension()) {
    // We don't want extension apps to become the active RootWindow.
    return;
  }

  if (root_window == active_root_window_)
    return;

  active_root_window_ = root_window;

  {
    base::AutoLock lock_scope(active_browser_lock_);
    // May be nullptr at this point, in which case we'll make the association in
    // OnBrowserCreated.
    active_browser_ = active_root_window_->GetBrowser();
  }
}

void RootWindowManager::OnBrowserCreated(RootWindow* root_window,
                                         CefRefPtr<CefBrowser> browser) {
  REQUIRE_MAIN_THREAD();

  if (root_window == active_root_window_) {
    base::AutoLock lock_scope(active_browser_lock_);
    active_browser_ = browser;
  }
}

void RootWindowManager::CreateExtensionWindow(
    CefRefPtr<CefExtension> extension,
    const CefRect& source_bounds,
    CefRefPtr<CefWindow> parent_window,
    base::OnceClosure close_callback,
    bool with_osr) {
  REQUIRE_MAIN_THREAD();

  if (!HasRootWindowAsExtension(extension)) {
    CreateRootWindowAsExtension(extension, source_bounds, parent_window,
                                std::move(close_callback), false, with_osr);
  }
}

void RootWindowManager::CleanupOnUIThread() {
  CEF_REQUIRE_UI_THREAD();

  if (temp_window_) {
    // TempWindow must be destroyed on the UI thread.
    temp_window_.reset(nullptr);
  }

  if (image_cache_) {
    image_cache_ = nullptr;
  }

  // Quit the main message loop.
  MainMessageLoop::Get()->Quit();
}

}  // namespace client
