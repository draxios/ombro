#pragma once
// Ombro — Direct3D 11 wireframe renderer

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>

namespace ombro {

struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool Init(HWND hwnd, int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Begin a frame: fade previous content by fadeAmount (0..1, 1 = full clear)
    void BeginFrame(float fadeAmount);

    // Draw wireframe lines. viewProj is a row-major 4x4 matrix.
    void DrawLines(const Vertex* vertices, int vertexCount,
                   const uint32_t* indices, int indexCount,
                   const float* viewProj);

    // Present the frame
    void EndFrame();

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    bool IsDeviceLost() const { return deviceLost_; }
    bool HandleDeviceLost();

private:
    bool CreateDeviceAndSwapChain(HWND hwnd, int w, int h);
    bool CreateRenderTargets();
    bool CreateShaders();
    bool CreateBuffers();
    void ReleaseRenderTargets();

    HWND hwnd_ = nullptr;
    int width_ = 0;
    int height_ = 0;

    // Core D3D11
    ID3D11Device*           device_   = nullptr;
    ID3D11DeviceContext*    context_  = nullptr;
    IDXGISwapChain*         swapChain_ = nullptr;
    ID3D11RenderTargetView* backBufferRTV_ = nullptr;

    // Ping-pong render targets for afterglow
    ID3D11Texture2D*          frameTex_[2]  = {};
    ID3D11RenderTargetView*   frameRTV_[2]  = {};
    ID3D11ShaderResourceView* frameSRV_[2]  = {};
    int currentFrame_ = 0;

    // Wireframe pipeline
    ID3D11VertexShader*  wireVS_      = nullptr;
    ID3D11PixelShader*   wirePS_      = nullptr;
    ID3D11InputLayout*   wireLayout_  = nullptr;

    // Fullscreen fade pipeline
    ID3D11VertexShader*  fadeVS_ = nullptr;
    ID3D11PixelShader*   fadePS_ = nullptr;

    // States
    ID3D11RasterizerState*   wireRaster_  = nullptr;
    ID3D11RasterizerState*   solidRaster_ = nullptr;
    ID3D11BlendState*        blendState_  = nullptr;
    ID3D11BlendState*        additiveBlendState_ = nullptr;
    ID3D11SamplerState*      sampler_     = nullptr;
    ID3D11DepthStencilState* noDepthState_ = nullptr;

    // Buffers
    ID3D11Buffer* vertexBuffer_   = nullptr;
    ID3D11Buffer* indexBuffer_    = nullptr;
    ID3D11Buffer* constantBuffer_ = nullptr;
    ID3D11Buffer* fadeConstBuffer_ = nullptr;
    int maxVertices_ = 0;
    int maxIndices_  = 0;
    bool deviceLost_ = false;
};

} // namespace ombro
