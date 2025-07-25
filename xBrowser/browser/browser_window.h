// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
#pragma once

#include <memory>

#include "include/cef_browser.h"
#include "xBrowser/browser/client_handler.h"
#include "xBrowser/browser/client_types.h"

namespace client {

// Represents a native child window hosting a single browser instance. The
// methods of this class must be called on the main thread unless otherwise
// indicated.
class BrowserWindow : public ClientHandler::Delegate {
 public:
  // This interface is implemented by the owner of the BrowserWindow. The
  // methods of this class will be called on the main thread.
  class Delegate {
   public:
    // Called when the browser has been created.
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) = 0;

    // Called when the BrowserWindow is closing.
    virtual void OnBrowserWindowClosing() {}

    // Called when the BrowserWindow has been destroyed.
    virtual void OnBrowserWindowDestroyed() = 0;

    // Set the window URL address.
    virtual void OnSetAddress(const std::string& url) = 0;

    // Set the window title.
    virtual void OnSetTitle(const std::string& title) = 0;

    // Set fullscreen mode.
    virtual void OnSetFullscreen(bool fullscreen) = 0;

    // Auto-resize contents.
    virtual void OnAutoResize(const CefSize& new_size) = 0;

    // Set the loading state.
    virtual void OnSetLoadingState(bool isLoading,
                                   bool canGoBack,
                                   bool canGoForward) = 0;

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(
        const std::vector<CefDraggableRegion>& regions) = 0;

    // Called when OnResourceResponse
    virtual void OnUrlReady(CefString type, CefString url) = 0;

   protected:
    virtual ~Delegate() {}
  };

  // Create a new browser and native window.
  virtual void CreateBrowser(ClientWindowHandle parent_handle,
                             const CefRect& rect,
                             const CefBrowserSettings& settings,
                             CefRefPtr<CefDictionaryValue> extra_info,
                             CefRefPtr<CefRequestContext> request_context) = 0;

  // Retrieve the configuration that will be used when creating a popup window.
  // The popup browser will initially be parented to |temp_handle| which should
  // be a pre-existing hidden window. The native window will be created later
  // after the browser has been created. This method will be called on the
  // browser process UI thread.
  virtual void GetPopupConfig(CefWindowHandle temp_handle,
                              CefWindowInfo& windowInfo,
                              CefRefPtr<CefClient>& client,
                              CefBrowserSettings& settings) = 0;

  // Show the popup window with correct parent and bounds in parent coordinates.
  virtual void ShowPopup(ClientWindowHandle parent_handle,
                         int x,
                         int y,
                         size_t width,
                         size_t height) = 0;

  // Show the window.
  virtual void Show() = 0;

  // Hide the window.
  virtual void Hide() = 0;

  // Set the window bounds in parent coordinates.
  virtual void SetBounds(int x, int y, size_t width, size_t height) = 0;

  // Set focus to the window.
  virtual void SetFocus(bool focus) = 0;

  // Set the device scale factor. Only used in combination with off-screen
  // rendering.
  virtual void SetDeviceScaleFactor(float device_scale_factor);

  // Returns the device scale factor. Only used in combination with off-screen
  // rendering.
  virtual float GetDeviceScaleFactor() const;

  // Returns the window handle.
  virtual ClientWindowHandle GetWindowHandle() const = 0;

  // Returns the browser owned by the window.
  CefRefPtr<CefBrowser> GetBrowser() const;

  // Returns true if the browser is closing.
  bool IsClosing() const;

 protected:
  // Allow deletion via std::unique_ptr only.
  friend std::default_delete<BrowserWindow>;

  // Constructor may be called on any thread.
  // |delegate| must outlive this object.
  explicit BrowserWindow(Delegate* delegate);

  // ClientHandler::Delegate methods.
  void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;
  void OnBrowserClosing(CefRefPtr<CefBrowser> browser) override;
  void OnBrowserClosed(CefRefPtr<CefBrowser> browser) override;
  void OnSetAddress(const std::string& url) override;
  void OnSetTitle(const std::string& title) override;
  void OnSetFullscreen(bool fullscreen) override;
  void OnAutoResize(const CefSize& new_size) override;
  void OnSetLoadingState(bool isLoading,
                         bool canGoBack,
                         bool canGoForward) override;
  void OnSetDraggableRegions(
      const std::vector<CefDraggableRegion>& regions) override;

  void OnUrlReady(CefString type, CefString url) override;

  Delegate* delegate_;
  CefRefPtr<CefBrowser> browser_;
  CefRefPtr<ClientHandler> client_handler_;
  bool is_closing_;

 private:
  DISALLOW_COPY_AND_ASSIGN(BrowserWindow);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
