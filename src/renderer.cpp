// Ombro — Direct3D 11 wireframe renderer
// AGPL-3.0 License

#include "renderer.h"
#include "shaders.h"
#include <d3dcompiler.h>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace ombro {

static const int INITIAL_MAX_VERTICES = 16384;
static const int INITIAL_MAX_INDICES  = 65536;

template<typename T>
static void SafeRelease(T*& ptr) {
    if (ptr) { ptr->Release(); ptr = nullptr; }
}

static ID3DBlob* CompileShader(const char* source, const char* entry, const char* target) {
    ID3DBlob* blob = nullptr;
    ID3DBlob* errors = nullptr;
    UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr,
                            entry, target, flags, 0, &blob, &errors);
    if (errors) errors->Release();
    if (FAILED(hr)) { if (blob) blob->Release(); return nullptr; }
    return blob;
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(HWND hwnd, int width, int height) {
    hwnd_ = hwnd;
    width_ = width;
    height_ = height;

    if (!CreateDeviceAndSwapChain(hwnd, width, height)) return false;
    if (!CreateRenderTargets()) return false;
    if (!CreateShaders()) return false;
    if (!CreateBuffers()) return false;

    // Wireframe rasterizer
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    rd.AntialiasedLineEnable = TRUE;
    device_->CreateRasterizerState(&rd, &wireRaster_);

    // Solid rasterizer (for fullscreen quad)
    rd.FillMode = D3D11_FILL_SOLID;
    device_->CreateRasterizerState(&rd, &solidRaster_);

    // Alpha blending (for fade pass)
    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&bd, &blendState_);

    // Additive blending (for wireframe — phosphor glow at intersections)
    D3D11_BLEND_DESC abd = {};
    abd.RenderTarget[0].BlendEnable = TRUE;
    abd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    abd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    abd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    abd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    abd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    abd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    abd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&abd, &additiveBlendState_);

    // Sampler
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device_->CreateSamplerState(&sd, &sampler_);

    // No depth testing
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    device_->CreateDepthStencilState(&dsd, &noDepthState_);

    return true;
}

void Renderer::Shutdown() {
    if (context_) context_->ClearState();

    ReleaseRenderTargets();
    SafeRelease(backBufferRTV_);
    SafeRelease(wireVS_);
    SafeRelease(wirePS_);
    SafeRelease(wireLayout_);
    SafeRelease(fadeVS_);
    SafeRelease(fadePS_);
    SafeRelease(wireRaster_);
    SafeRelease(solidRaster_);
    SafeRelease(blendState_);
    SafeRelease(additiveBlendState_);
    SafeRelease(sampler_);
    SafeRelease(noDepthState_);
    SafeRelease(vertexBuffer_);
    SafeRelease(indexBuffer_);
    SafeRelease(constantBuffer_);
    SafeRelease(fadeConstBuffer_);
    SafeRelease(swapChain_);
    SafeRelease(context_);
    SafeRelease(device_);
}

void Renderer::Resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (width == width_ && height == height_) return;

    width_ = width;
    height_ = height;

    context_->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseRenderTargets();
    SafeRelease(backBufferRTV_);

    swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTargets();
}

void Renderer::BeginFrame(float fadeAmount) {
    int prev = currentFrame_;
    int curr = 1 - currentFrame_;

    // Render faded previous frame onto current target
    context_->OMSetRenderTargets(1, &frameRTV_[curr], nullptr);
    float clearColor[4] = {0, 0, 0, 1};
    context_->ClearRenderTargetView(frameRTV_[curr], clearColor);

    D3D11_VIEWPORT vp = {0, 0, (float)width_, (float)height_, 0, 1};
    context_->RSSetViewports(1, &vp);

    if (fadeAmount < 1.0f) {
        // Draw faded copy of previous frame
        struct { float alpha; float pad[3]; } fc = {1.0f - fadeAmount, {}};
        context_->UpdateSubresource(fadeConstBuffer_, 0, nullptr, &fc, 0, 0);

        context_->RSSetState(solidRaster_);
        float blendFactor[4] = {0, 0, 0, 0};
        context_->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
        context_->OMSetDepthStencilState(noDepthState_, 0);

        context_->VSSetShader(fadeVS_, nullptr, 0);
        context_->PSSetShader(fadePS_, nullptr, 0);
        context_->PSSetConstantBuffers(0, 1, &fadeConstBuffer_);
        context_->PSSetShaderResources(0, 1, &frameSRV_[prev]);
        context_->PSSetSamplers(0, 1, &sampler_);

        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->IASetInputLayout(nullptr);
        context_->Draw(3, 0);

        // Unbind SRV so we can use the texture as render target later
        ID3D11ShaderResourceView* nullSRV = nullptr;
        context_->PSSetShaderResources(0, 1, &nullSRV);
    }

    // Set up for wireframe drawing with additive blending (phosphor glow)
    context_->RSSetState(wireRaster_);
    float blendFactor[4] = {0, 0, 0, 0};
    context_->OMSetBlendState(additiveBlendState_, blendFactor, 0xFFFFFFFF);
    context_->OMSetDepthStencilState(noDepthState_, 0);
}

void Renderer::DrawLines(const Vertex* vertices, int vertexCount,
                         const uint32_t* indices, int indexCount,
                         const float* viewProj) {
    if (!vertices || vertexCount <= 0 || !indices || indexCount <= 0) return;

    // Grow vertex buffer if needed
    if (vertexCount > maxVertices_) {
        SafeRelease(vertexBuffer_);
        maxVertices_ = vertexCount + 1024;
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = maxVertices_ * sizeof(Vertex);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&bd, nullptr, &vertexBuffer_);
    }

    // Grow index buffer if needed
    if (indexCount > maxIndices_) {
        SafeRelease(indexBuffer_);
        maxIndices_ = indexCount + 2048;
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = maxIndices_ * sizeof(uint32_t);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&bd, nullptr, &indexBuffer_);
    }

    // Upload vertex data
    D3D11_MAPPED_SUBRESOURCE mapped;
    context_->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices, vertexCount * sizeof(Vertex));
    context_->Unmap(vertexBuffer_, 0);

    // Upload index data
    context_->Map(indexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, indices, indexCount * sizeof(uint32_t));
    context_->Unmap(indexBuffer_, 0);

    // Upload constant buffer (view-projection matrix)
    context_->UpdateSubresource(constantBuffer_, 0, nullptr, viewProj, 0, 0);

    // Draw
    context_->VSSetShader(wireVS_, nullptr, 0);
    context_->PSSetShader(wirePS_, nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, &constantBuffer_);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
    context_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
    context_->IASetInputLayout(wireLayout_);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context_->DrawIndexed(indexCount, 0, 0);
}

void Renderer::EndFrame() {
    int curr = 1 - currentFrame_;

    // Copy current frame texture to back buffer
    context_->OMSetRenderTargets(1, &backBufferRTV_, nullptr);
    float clearColor[4] = {0, 0, 0, 1};
    context_->ClearRenderTargetView(backBufferRTV_, clearColor);

    D3D11_VIEWPORT vp = {0, 0, (float)width_, (float)height_, 0, 1};
    context_->RSSetViewports(1, &vp);

    // Draw fullscreen quad with the current frame texture
    struct { float alpha; float pad[3]; } fc = {1.0f, {}};
    context_->UpdateSubresource(fadeConstBuffer_, 0, nullptr, &fc, 0, 0);

    context_->RSSetState(solidRaster_);
    float blendFactor[4] = {0, 0, 0, 0};
    context_->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
    context_->OMSetDepthStencilState(noDepthState_, 0);

    context_->VSSetShader(fadeVS_, nullptr, 0);
    context_->PSSetShader(fadePS_, nullptr, 0);
    context_->PSSetConstantBuffers(0, 1, &fadeConstBuffer_);
    context_->PSSetShaderResources(0, 1, &frameSRV_[curr]);
    context_->PSSetSamplers(0, 1, &sampler_);

    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->IASetInputLayout(nullptr);
    context_->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context_->PSSetShaderResources(0, 1, &nullSRV);

    swapChain_->Present(1, 0);
    currentFrame_ = curr;
}

// --- Private implementation ---

bool Renderer::CreateDeviceAndSwapChain(HWND hwnd, int w, int h) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = w;
    scd.BufferDesc.Height = h;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, 2, D3D11_SDK_VERSION, &scd,
        &swapChain_, &device_, &featureLevel, &context_);

    return SUCCEEDED(hr);
}

bool Renderer::CreateRenderTargets() {
    // Back buffer RTV
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (!backBuffer) return false;
    device_->CreateRenderTargetView(backBuffer, nullptr, &backBufferRTV_);
    backBuffer->Release();

    // Ping-pong frame textures for afterglow
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width_;
    td.Height = height_;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    for (int i = 0; i < 2; i++) {
        device_->CreateTexture2D(&td, nullptr, &frameTex_[i]);
        device_->CreateRenderTargetView(frameTex_[i], nullptr, &frameRTV_[i]);
        device_->CreateShaderResourceView(frameTex_[i], nullptr, &frameSRV_[i]);

        // Clear to black
        float black[4] = {0, 0, 0, 1};
        context_->ClearRenderTargetView(frameRTV_[i], black);
    }

    return true;
}

bool Renderer::CreateShaders() {
    // Wireframe shaders
    ID3DBlob* vsBlob = CompileShader(g_wireframeVS, "main", "vs_4_0");
    if (!vsBlob) return false;
    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &wireVS_);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    device_->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &wireLayout_);
    vsBlob->Release();

    ID3DBlob* psBlob = CompileShader(g_wireframePS, "main", "ps_4_0");
    if (!psBlob) return false;
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &wirePS_);
    psBlob->Release();

    // Fullscreen fade shaders
    vsBlob = CompileShader(g_fullscreenVS, "main", "vs_4_0");
    if (!vsBlob) return false;
    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &fadeVS_);
    vsBlob->Release();

    psBlob = CompileShader(g_fadePS, "main", "ps_4_0");
    if (!psBlob) return false;
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &fadePS_);
    psBlob->Release();

    return true;
}

bool Renderer::CreateBuffers() {
    maxVertices_ = INITIAL_MAX_VERTICES;
    maxIndices_ = INITIAL_MAX_INDICES;

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = maxVertices_ * sizeof(Vertex);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&bd, nullptr, &vertexBuffer_);

    bd.ByteWidth = maxIndices_ * sizeof(uint32_t);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    device_->CreateBuffer(&bd, nullptr, &indexBuffer_);

    // Constant buffer for wireframe view-projection (64 bytes = 4x4 float matrix)
    bd.ByteWidth = 64;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    device_->CreateBuffer(&bd, nullptr, &constantBuffer_);

    // Constant buffer for fade alpha (16 bytes)
    bd.ByteWidth = 16;
    device_->CreateBuffer(&bd, nullptr, &fadeConstBuffer_);

    return vertexBuffer_ && indexBuffer_ && constantBuffer_ && fadeConstBuffer_;
}

void Renderer::ReleaseRenderTargets() {
    for (int i = 0; i < 2; i++) {
        SafeRelease(frameSRV_[i]);
        SafeRelease(frameRTV_[i]);
        SafeRelease(frameTex_[i]);
    }
}

} // namespace ombro
