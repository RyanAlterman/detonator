// Copyright (C) 2020-2021 Sami Väisänen
// Copyright (C) 2020-2021 Ensisoft http://www.ensisoft.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "config.h"

#include "warnpush.h"
#  include <QWindow>
#  include <QOpenGLContext>
#  include <QKeyEvent>
#  include <QElapsedTimer>
#  include <QPalette>
#  include <QImage>
#  include <QTimer>
#include "warnpop.h"

#include <memory>
#include <functional>
#include <optional>

#include "base/assert.h"
#include "graphics/painter.h"
#include "graphics/device.h"
#include "graphics/color4f.h"

namespace gui
{
    // Integrate QOpenGLWidget and custom graphics device and painter
    // implementations from gfx into a reusable widget class.
    class GfxWindow : public QWindow
    {
        Q_OBJECT

    public:
        GfxWindow();
       ~GfxWindow() override;

        enum class MouseCursor {
            Native, Custom
        };

        enum class CursorShape {
            ArrowCursor, CrossHair
        };

        // Important to call dispose to cleanly dispose of all the graphics
        // resources while the Qt's OpenGL context is still valid,
        // I.e the window still exists and hasn't been closed or anything.
        void dispose();

        void reloadShaders()
        {
            // we simply just delete all program objects
            // which will trigger the rebuild of the needed
            // programs which will ultimately need to (re)load
            // and compile the shaders as well.
            mCustomGraphicsDevice->DeletePrograms();
            mCustomGraphicsDevice->DeleteShaders();
        }
        void reloadTextures()
        {
            mCustomGraphicsDevice->DeleteTextures();
        }

        void triggerPaint()
        {
            paintGL();
        }

        bool hasInputFocus() const;

        gfx::Color4f GetCurrentClearColor();

        const gfx::Color4f* GetClearColor() const
        {
            if (!mClearColor.has_value())
                return nullptr;
            return &mClearColor.value();
        }
        gfx::Device* GetDevice() const
        { return mCustomGraphicsDevice.get(); }
        gfx::Painter* GetPainter() const
        { return mCustomGraphicsPainter.get(); }
        gfx::Device::ResourceStats getDeviceResourceStats() const
        {
            gfx::Device::ResourceStats stats;
            if (mCustomGraphicsDevice)
                mCustomGraphicsDevice->GetResourceStats(&stats);
            return stats;
        }

        double GetTime() const noexcept
        { return mTimeAccum; }

        void SetClearColor(const gfx::Color4f& color)
        { mClearColor = color; }

        void ResetClearColor()
        { mClearColor.reset(); }

        bool haveVSYNC() const;

        float getCurrentFPS() const
        { return mCurrentFps; }

        void CreateRenderingSurface(bool vsync);

        void SetCursorShape(CursorShape shape);

        CursorShape GetCursorShape() const
        { return mCursorShape; }

        QImage TakeScreenshot() const;

        // callback to invoke when paint must be done.
        // secs is the seconds elapsed since last paint.
        std::function<void (gfx::Painter&, double secs)> onPaintScene;

        // callback to invoke to initialize game, i.e. the OpenGL widget
        // has been initialized.
        // width and height are the width and height of the widget's viewport.
        std::function<void (unsigned width, unsigned height)> onInitScene;

        // mouse callbacks
        std::function<void (QMouseEvent* mickey)> onMouseMove;
        std::function<void (QMouseEvent* mickey)> onMousePress;
        std::function<void (QMouseEvent* mickey)> onMouseRelease;
        std::function<void (QWheelEvent* wheel)>  onMouseWheel;
        std::function<void (QMouseEvent* mickey)> onMouseDoubleClick;
        // keyboard callbacks
        std::function<bool (QKeyEvent* key)>      onKeyPress;
        std::function<bool (QKeyEvent* key)>      onKeyRelease;

        static void SetDefaultFilter(gfx::Device::MinFilter filter)
        { DefaultMinFilter = filter; }
        static void SetDefaultFilter(gfx::Device::MagFilter filter)
        { DefaultMagFilter = filter; }
        static void SetDefaultClearColor(const gfx::Color4f& color)
        { ClearColor = color; }
        static gfx::Color4f& GetDefaultClearColor()
        { return ClearColor; }
        static void CleanGarbage();
        static void DeleteTexture(const std::string& gpuId);

        static void BeginFrame();
        static bool EndFrame();

        static void SetVSYNC(bool on_off);
        static bool GetVSYNC();

        static void SetMouseCursor(MouseCursor cursor);
        static MouseCursor GetMouseCursor();
    public slots:
        void clearColorChanged(QColor color);
    private slots:
        void doInit();

    private:
        void initializeGL();
        void paintGL();
        virtual void mouseMoveEvent(QMouseEvent* mickey) override;
        virtual void mousePressEvent(QMouseEvent* mickey) override;
        virtual void mouseReleaseEvent(QMouseEvent* mickey) override;
        virtual void mouseDoubleClickEvent(QMouseEvent* mickey) override;
        virtual void keyPressEvent(QKeyEvent* key) override;
        virtual void keyReleaseEvent(QKeyEvent* key) override;
        virtual void wheelEvent(QWheelEvent* wheel) override;
        virtual void focusInEvent(QFocusEvent* event) override;
        virtual void focusOutEvent(QFocusEvent* event) override;

    private:
        std::shared_ptr<gfx::Device> mCustomGraphicsDevice;
        std::unique_ptr<gfx::Painter> mCustomGraphicsPainter;
        std::optional<gfx::Color4f> mClearColor;
    private:
        QElapsedTimer mClock;
        bool mVsync       = false;
        bool mHasFocus    = false;
        bool mInitDone    = false;
    private:
        quint64 mNumFrames = 0;
        float mCurrentFps  = 0.0f;
        double mTimeStamp = 0.0;
        double mTimeAccum = 0.0;
        CursorShape mCursorShape = CursorShape::ArrowCursor;

    private:
        std::shared_ptr<QOpenGLContext> mContext;
    private:
        static gfx::Device::MinFilter DefaultMinFilter;
        static gfx::Device::MagFilter DefaultMagFilter;
        static gfx::Color4f ClearColor;
        static MouseCursor WindowMouseCursor;
    };

    // This is now a "widget shim" that internally creates a QOpenGLWindow
    // and places it in a window container. This is done because of
    // using QOpenGLWindow provides slightly better performance
    // than QOpenGLWidget.
    class GfxWidget : public QWidget
    {
        Q_OBJECT

    public:
        using CursorShape = GfxWindow::CursorShape;

        explicit GfxWidget(QWidget* parent);
       ~GfxWidget() override;

        bool hasInputFocus() const
        { return mWindow->hasInputFocus(); }
        bool haveVSYNC() const
        { return mWindow->haveVSYNC(); }
        float getCurrentFPS() const
        { return mWindow->getCurrentFPS(); }

        double GetTime() const noexcept
        { return mWindow->GetTime(); }

        gfx::Color4f GetCurrentClearColor() const
        { return mWindow->GetCurrentClearColor(); }

        const gfx::Color4f* GetClearColor() const
        { return mWindow->GetClearColor(); }
        gfx::Device* GetDevice() const
        { return mWindow ? mWindow->GetDevice() : nullptr; }
        gfx::Painter* GetPainter() const
        { return mWindow ? mWindow->GetPainter() : nullptr; }
        gfx::Device::ResourceStats getDeviceResourceStats() const
        { return mWindow->getDeviceResourceStats(); }

        void SetClearColor(const QColor& color);

        void SetClearColor(const gfx::Color4f& color)
        {
            mWindow->SetClearColor(color);
        }

        void ResetClearColor()
        {
            mWindow->ResetClearColor();
        }
        QImage TakeSreenshot() const
        { return mWindow->TakeScreenshot(); }
        // callback to invoke when paint must be done.
        // secs is the seconds elapsed since last paint.
        std::function<void (gfx::Painter&, double secs)> onPaintScene;

        // callback to invoke to initialize game, i.e. the OpenGL widget
        // has been initialized.
        // width and height are the width and height of the widget's viewport.
        std::function<void (unsigned width, unsigned height)> onInitScene;

        // mouse callbacks
        std::function<void (QMouseEvent* mickey)> onMouseMove;
        std::function<void (QMouseEvent* mickey)> onMousePress;
        std::function<void (QMouseEvent* mickey)> onMouseRelease;
        std::function<void (QWheelEvent* wheel)>  onMouseWheel;
        std::function<void (QMouseEvent* mickey)> onMouseDoubleClick;
        // keyboard callbacks. Returns true if the key press event
        // was consumed. This will stop further processing of the
        // keypress.
        std::function<bool (QKeyEvent* key)>      onKeyPress;
        std::function<bool (QKeyEvent* key)>      onKeyRelease;

        // zoom in/out callbacks
        std::function<void ()> onZoomIn;
        std::function<void ()> onZoomOut;

        void ShowColorDialog();

        void SetCursorShape(CursorShape shape);

        // start low frequency paint timer. used with dialogs.
        // Tries to run at 60 fps
        void StartPaintTimer();

    public slots:
        void dispose();
        void reloadShaders();
        void reloadTextures();
        void triggerPaint();
    private:
        void TranslateZoomInOut(QWheelEvent* event);
        void ToggleVSync();
        enum class WidgetFocus {
            FocusNextWidget,
            FocusPrevWidget
        };
        void FocusNextPrev(WidgetFocus which);

    private:
        virtual void resizeEvent(QResizeEvent* event) override;
        virtual void focusInEvent(QFocusEvent* focus) override;
        virtual void focusOutEvent(QFocusEvent* focus) override;
    private:
        GfxWindow* mWindow = nullptr;
        QWidget* mContainer = nullptr;
        QTimer mTimer;
    };

} // namespace

