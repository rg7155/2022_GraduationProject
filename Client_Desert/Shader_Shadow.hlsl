#pragma once

#include "Shaders.hlsl"
//#include "Light.hlsl" //이미 Shaders에 있으므로 오류남





struct PS_DEPTH_OUTPUT
{
    float fzPosition : SV_Target;
    float fDepth : SV_Depth;
};

//깊이를 저장하는 PS
PS_DEPTH_OUTPUT PSDepthWriteShader(VS_STANDARD_OUTPUT input)
{
    PS_DEPTH_OUTPUT output;

	//원투 나누기 한 좌표-깊이
    output.fzPosition = input.position.z;
    output.fDepth = input.position.z;

    return (output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SHADOW_MAP_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;

    float2 uv : TEXCOORD;
    float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
    float4 uvs[MAX_LIGHTS] : TEXCOORD1;
    
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_STANDARD_INPUT input)
{
    VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT) 0;

    float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = positionW.xyz;
    output.position = mul(mul(positionW, gmtxView), gmtxProjection);
    output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;
    output.uv = input.uv;
    output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
		//0은 조명끔, 조명 좌표계로 바꾸고 텍스쳐 좌표계로 바꿈
        if (gcbToLightSpaces[i].f4Position.w != 0.0f)
            output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
    }

    return (output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP) 
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, /*float2(0.5f, 0.5f) */input.uv);
    else
        cAlbedoColor = gMaterial.m_cDiffuse;
    
	//그림자면 어둡고 아니면 원래 조명 색
    float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);

    //return (cIllumination);
	float4 cColor = cAlbedoColor;
	return(lerp(cColor, cIllumination, 0.4f));
    
}

///////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT) 0;

    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 1)
    {
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 0.0f);
    }
    if (nVertexID == 2)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    if (nVertexID == 3)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 4)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    if (nVertexID == 5)
    {
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 1.0f);
    }

    return (output);
}

//SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float fDepthFromLight0 = gtxtDepthTextures[gfShadowMapIndex].SampleLevel(gssWrap /*gssBorder*/, input.uv, 0).r;

    return ((float4) (fDepthFromLight0 * 0.8f));
}

///////////////////////////////////////////////////////////////////////////////


