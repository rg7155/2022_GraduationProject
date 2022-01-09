#include "Shaders.hlsl"
#include "Light.hlsl"



struct CB_TOOBJECTSPACE
{
    matrix mtxToTexture;
    float4 f4Position;
};

cbuffer cbToLightSpace : register(b3)
{
    CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
};

struct VS_LIGHTING_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
    VS_LIGHTING_OUTPUT output;

    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

    return (output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
    input.normalW = normalize(input.normalW);
    float4 uvs[MAX_LIGHTS];
    float4 cIllumination = Lighting(input.positionW, input.normalW, false, uvs);

//	return(cIllumination);
    return (float4(input.normalW * 0.5f + 0.5f, 1.0f));
}



struct PS_DEPTH_OUTPUT
{
    float fzPosition : SV_Target;
    float fDepth : SV_Depth;
};

//±íÀÌ¸¦ ÀúÀåÇÏ´Â PS
PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
    PS_DEPTH_OUTPUT output;

	//¿øÅõ ³ª´©±â ÇÑ ÁÂÇ¥-±íÀÌ
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

    float4 uvs[MAX_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
    VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT) 0;

    float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = positionW.xyz;
    output.position = mul(mul(positionW, gmtxView), gmtxProjection);
    output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
		//0Àº Á¶¸í²û, Á¶¸í ÁÂÇ¥°è·Î ¹Ù²Ù°í ÅØ½ºÃÄ ÁÂÇ¥°è·Î ¹Ù²Þ
        if (gcbToLightSpaces[i].f4Position.w != 0.0f)
            output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
    }

    return (output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	//±×¸²ÀÚ¸é ¾îµÓ°í ¾Æ´Ï¸é ¿ø·¡ Á¶¸í »ö
    float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);

    return (cIllumination);
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

float4 GetColorFromDepth(float fDepth)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (fDepth < 0.00625f)
        cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    else if (fDepth < 0.0125f)
        cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    else if (fDepth < 0.025f)
        cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
    else if (fDepth < 0.05f)
        cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
    else if (fDepth < 0.075f)
        cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
    else if (fDepth < 0.1f)
        cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
    else if (fDepth < 0.4f)
        cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
    else if (fDepth < 0.6f)
        cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
    else if (fDepth < 0.8f)
        cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
    else if (fDepth < 0.9f)
        cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
    else if (fDepth < 0.95f)
        cColor = float4(0.5f, 0.0f, 0.5f, 1.0f);
    else if (fDepth < 0.99f)
        cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    else if (fDepth < 0.999f)
        cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
    else if (fDepth == 1.0f)
        cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
    else if (fDepth > 1.0f)
        cColor = float4(0.0f, 0.0f, 0.5f, 1.0f);
    else
        cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    return (cColor);
}

//SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float fDepthFromLight0 = gtxtDepthTextures[0 /*gfShadowMapIndex*/].SampleLevel(gssWrap /*gssBorder*/, input.uv, 0).r;

    return ((float4) (fDepthFromLight0 * 0.8f));
}

