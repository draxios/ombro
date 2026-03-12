#pragma once
// Ombro — Embedded HLSL shaders

// Wireframe vertex shader
static const char g_wireframeVS[] = R"hlsl(
cbuffer Constants : register(b0) {
    float4x4 viewProj;
};

struct VSInput {
    float3 pos   : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.pos = mul(float4(input.pos, 1.0), viewProj);
    output.color = input.color;
    return output;
}
)hlsl";

// Wireframe pixel shader
static const char g_wireframePS[] = R"hlsl(
struct PSInput {
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET {
    return input.color;
}
)hlsl";

// Fullscreen quad vertex shader (generates triangle from SV_VertexID)
static const char g_fullscreenVS[] = R"hlsl(
struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

VSOutput main(uint id : SV_VertexID) {
    VSOutput output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.pos = float4(output.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return output;
}
)hlsl";

// Fullscreen fade pixel shader (afterglow effect)
static const char g_fadePS[] = R"hlsl(
Texture2D prevFrame : register(t0);
SamplerState samp   : register(s0);

cbuffer FadeConstants : register(b0) {
    float fadeAlpha;
    float3 _pad;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET {
    return prevFrame.Sample(samp, input.uv) * fadeAlpha;
}
)hlsl";
