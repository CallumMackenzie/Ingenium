#include "ModWin.h"
#include "WindowClass.h"
#include "Window.h"
#include "Log.h"
#include "Direct2D.h"
#include "Direct2DWindow.h"

// Callum Mackenzie

Direct2DWindow::Direct2DWindow(RootWindow *window_)
{
    window = window_;
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory);

    RECT rc;
    GetClientRect(window->getHWND(), &rc);

    hr = pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            window->getHWND(),
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top),
            D2D1_PRESENT_OPTIONS_NONE),
        &pRT);

    if (SUCCEEDED(hr))
    {
        pRT->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &pBlackBrush);
    }

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(m_pDWriteFactory),
        reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
    if (SUCCEEDED(hr))
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Verdana",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            20,
            L"",
            &m_pTextFormat);
    }

    if (SUCCEEDED(hr))
    {
        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    // https://docs.microsoft.com/en-us/windows/win32/direct2d/getting-started-with-direct2d#step-1-include-direct2d-header
}
void Direct2DWindow::releaseResources()
{
    pRT->Release();
    pRT = nullptr;
    pBlackBrush->Release();
    pBlackBrush = nullptr;
}
void Direct2DWindow::closing()
{
    pD2DFactory->Release();
    pD2DFactory = nullptr;
}
ID2D1HwndRenderTarget *Direct2DWindow::getRenderPane()
{
    return pRT;
}
RootWindow *Direct2DWindow::getWindow()
{
    return window;
}
void Direct2DWindow::beginRender()
{
    pRT->BeginDraw();
}
void Direct2DWindow::endRender()
{
    pRT->EndDraw();
}
void Direct2DWindow::drawQueue(bool preservePrev)
{
    if (pRT != nullptr)
    {
        pRT->SetTransform(D2D1::Matrix3x2F::Rotation(0, D2D1::Point2F(0, 0)));
        if (!preservePrev)
        {
            pRT->Clear(clearColour);
        }
        // TODO: Render render queue
    }
}
void Direct2DWindow::drawRect(float x, float y, float width, float height, ID2D1SolidColorBrush *br)
{
    pRT->DrawRectangle(
        D2D1::RectF(
            x,
            y,
            x + width,
            x + height),
        br, 10.0f);
}
void Direct2DWindow::drawBitmap(ID2D1Bitmap *bt, int width, int height, float top, float left, float rotation, float transparency, D2D1_POINT_2F rotationCenter, RECT sourceRect, D2D1_BITMAP_INTERPOLATION_MODE interpMode)
{
    D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(top, left);
    pRT->SetTransform(D2D1::Matrix3x2F::Rotation(rotation, rotationCenter));
    pRT->DrawBitmap(
        bt,
        D2D1::RectF(
            upperLeftCorner.x,
            upperLeftCorner.y,
            upperLeftCorner.x + width,
            upperLeftCorner.y + height),
        transparency,
        interpMode,
        D2D1::RectF(
            (float)sourceRect.left,
            (float)sourceRect.top,
            (float)sourceRect.right,
            (float)sourceRect.bottom));
}

int Direct2DWindow::checkRenderQueueLength()
{
    // TODO : Get render queue length
    return 0;
}