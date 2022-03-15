struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8.x);
	uint					gnEffectsMask : packoffset(c8.y);
	float					gfDissolve : packoffset(c8.z);
};

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

cbuffer cbFrameworkInfo : register(b5)
{
    float gfCurrentTime : packoffset(c0.x);
    float gfElapsedTime : packoffset(c0.y);
    float gfShadowMapIndex : packoffset(c0.z);
};

cbuffer cbTextureAnim : register(b6)
{
	matrix	gmtxTextureAnim : packoffset(c0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

//gnTexturesMask
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

//gnEffectsMask
#define EFFECT_FOG					0x01
#define EFFECT_LIMLIGHT				0x02
#define EFFECT_DISSOLVE				0x04

Texture2D gtxtTexture : register(t0);

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	
    float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		//0은 조명끔, 조명 좌표계로 바꾸고 텍스쳐 좌표계로 바꿈
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}
	
	return(output);
}

//안개
float4 Fog(float4 cColor, float3 vPos)
{
    float3 vPosToCamera = gvCameraPosition - vPos;
    float fDisToCamera = length(vPosToCamera);
    float fFogFactor = 0.0f;
    
    float fStart = 0.f, fEnd = 100.f;
    fFogFactor = saturate((fEnd - fDisToCamera) / (fEnd - fStart));
	
    //float4 fFogColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
    float4 fFogColor = float4(135 / 255.f, 116 / 255.f, 75 / 255.f, 1.0f);
	
    //return (lerp(cColor, fFogColor, fFogFactor));
    //return (lerp(fFogColor, cColor, fFogFactor));
    return (fFogFactor * cColor + (1.f - fFogFactor) * fFogColor);
}

//림라이트
float4 Limlight(float3 normalW)
{
    float rim = 0;
	
    float3 cameraDirection = (gmtxView._13_23_33_11).xyz; //Look?
    //float3 cameraDirection = (gmtxView._31_32_33_11).xyz; //Look?
	
    rim = 1 - saturate(dot(normalW, -cameraDirection));

    rim = pow(rim, 3.0f); // 강도 조정.

    float3 rimColor = float3(0.8f, 0.8f, 0.8f);
    rimColor = rim * rimColor;

    return float4(rimColor, 1);
}

//디졸브
void Dissolve(float2 uv)
{
	float4 f4Dissolve = gtxtTexture.Sample(gssWrap, uv);
	float fClip = f4Dissolve.r - gfDissolve;
	clip(fClip);

	return;
	//if (fClip < 0.2 && gfDissolve > 0.1)
	//{
	//	float4 fBurn = gtxtAlbedoTexture.Sample(gssWrap, float2(vDissovle.r * 4, 0));
	//	Color *= fBurn;
	//}
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) 
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	else
        cAlbedoColor = gMaterial.m_cDiffuse;

	float4 cColor = cAlbedoColor;
	
	float3 normalW;
	normalW = normalize(input.normalW);
	
	float4 cIllumination = Lighting(input.positionW, normalW, true, input.uvs);
    float4 cColorByLight = lerp(cColor, cIllumination, 0.5f);
	
    float4 cColorByFog = cColorByLight;
    if (gnEffectsMask & EFFECT_FOG)			cColorByFog = Fog(cColorByLight, input.positionW);
    float4 cColorByLim = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnEffectsMask & EFFECT_LIMLIGHT)	cColorByLim = Limlight(input.normalW);
	if (gnEffectsMask & EFFECT_DISSOLVE)	Dissolve(input.uv);

    return cColorByFog + cColorByLim;
    //return cColorByFog + Limlight(input.normalW); //림라이트
	
    //return (lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	float4 positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
	output.positionW = positionW.xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	for (int j = 0; j < MAX_LIGHTS; j++)
	{
		//0은 조명끔, 조명 좌표계로 바꾸고 텍스쳐 좌표계로 바꿈
		if (gcbToLightSpaces[j].f4Position.w != 0.0f)
			output.uvs[j] = mul(positionW, gcbToLightSpaces[j].mtxToTexture);
	}
	
	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct VS_TEXTURED_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    //float4 cColor = { 1.f, 1.f, 1.f, 1.f };
    return (cColor);
}

float4 PSTexturedTrail(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    cColor.a = input.uv.y;
	
    float4 cMulColor = { 1.f / 255.f, 165.f / 255.f, 172.f / 255.f, 0.f };
    cColor += cMulColor;
    return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = mul(float3(input.uv, 1.0f), (float3x3) (gmtxTextureAnim)).xy;

    return (output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
	
    float3 positionW : POSITION1;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

    float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = positionW.xyz;
	
	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	//return(cColor);
	
    return Fog(cColor, input.positionW);
	
}

//////////////////////////

