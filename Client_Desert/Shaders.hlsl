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
    matrix                  gmtxOrthoProjection : packoffset(c8);
    float3                  gvCameraPosition : packoffset(c12);
	//float3				gvCameraPosition : packoffset(c8);
    //matrix				gmtxOrthoProjection : packoffset(c9);
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

Texture2D gtxtTexture2/*gtxtMetallicTexture*/ : register(t9); //어차피 안쓸꺼 그냥 이펙트 텍츠처 1
//Texture2D gtxtEmissionTexture : register(t10);
Buffer<float4> gRandomBuffer : register(t10);

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
float4 Dissolve(float2 uv)
{
	float4 f4Dissolve = gtxtTexture.Sample(gssWrap, uv);
	float fClip = f4Dissolve.r * 2.f - gfDissolve;
	clip(fClip);

	//return;
    float4 fBurn = { 1.f, 1.f, 1.f, 1.f };
	
    if (fClip < 0.2 && gfDissolve > 0.1)
        fBurn = gtxtTexture2.Sample(gssWrap, float2(f4Dissolve.r, 0));
	
    return fBurn;
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
    //if (gnEffectsMask & EFFECT_FOG)			cColorByFog = Fog(cColorByLight, input.positionW);
    float4 cColorByLim = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnEffectsMask & EFFECT_LIMLIGHT)	cColorByLim = Limlight(input.normalW);
    if (gnEffectsMask & EFFECT_DISSOLVE)    cColorByFog *= Dissolve(input.uv);

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

VS_TEXTURED_OUTPUT VSOrthoTextured(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;
    //output.position = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxOrthoProjection);
    output.position = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxOrthoProjection);
    output.uv = input.uv;
    return (output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    //cColor.a = cColor.r;
    return (cColor);
}

float4 PSAlphaTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    cColor.a = gfDissolve;
    
    return (cColor);
}


float4 PSTexturedTrail(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cDistortion = gtxtTexture2.Sample(gssWrap, input.uv);
    
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv + (cDistortion.r * 2.f));
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


float4 PSSpriteAnimationShockwave(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    cColor.a = cColor.r;
    
    float4 cMulColor = { 1.f / 255.f, 165.f / 255.f, 172.f / 255.f, 0.f };
    cColor += cMulColor;
    return (cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





#define PARTICLE_TYPE_EMITTER	0 //정점을 만드는
#define PARTICLE_TYPE_FLARE		0x0ff //다른파티클을 생성하지 못하는?


void GetBillboardCorners(float3 position, float2 size, out float4 pf4Positions[4])
{
    float3 f3Up = float3(0.0f, 1.0f, 0.0f);
    float3 f3Look = normalize(gvCameraPosition - position);
    float3 f3Right = normalize(cross(f3Up, f3Look));

    pf4Positions[0] = float4(position + size.x * f3Right - size.y * f3Up, 1.0f);
    pf4Positions[1] = float4(position + size.x * f3Right + size.y * f3Up, 1.0f);
    pf4Positions[2] = float4(position - size.x * f3Right - size.y * f3Up, 1.0f);
    pf4Positions[3] = float4(position - size.x * f3Right + size.y * f3Up, 1.0f);
}

void GetPositions(float3 position, float2 f2Size, out float3 pf3Positions[8])
{
    float3 f3Right = float3(1.0f, 0.0f, 0.0f);
    float3 f3Up = float3(0.0f, 1.0f, 0.0f);
    float3 f3Look = float3(0.0f, 0.0f, 1.0f);

    float3 f3Extent = normalize(float3(1.0f, 1.0f, 1.0f));

    pf3Positions[0] = position + float3(-f2Size.x, 0.0f, -f2Size.y);
    pf3Positions[1] = position + float3(-f2Size.x, 0.0f, +f2Size.y);
    pf3Positions[2] = position + float3(+f2Size.x, 0.0f, -f2Size.y);
    pf3Positions[3] = position + float3(+f2Size.x, 0.0f, +f2Size.y);
    pf3Positions[4] = position + float3(-f2Size.x, 0.0f, 0.0f);
    pf3Positions[5] = position + float3(+f2Size.x, 0.0f, 0.0f);
    pf3Positions[6] = position + float3(0.0f, 0.0f, +f2Size.y);
    pf3Positions[7] = position + float3(0.0f, 0.0f, -f2Size.y);
}


struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float3 color : COLOR;
    float3 velocity : VELOCITY;
    float3 acceleration : ACCELERATION; //가속도
    float2 size : SIZE;
    float2 age : AGELIFETIME; //(Age, Lifetime)
    uint type : PARTICLETYPE;
    float alpha : ALPHA;
};

//0번
VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input)
{
    return (input);
}


[maxvertexcount(9)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
     //VS_PARTICLE_INPUT particle = input[0];

    //particle.age.x += gfElapsedTime;
    

    /////////////////////////////////////////////////////1
    VS_PARTICLE_INPUT particle = input[0];

    particle.age.x += gfElapsedTime;
    if (particle.type == PARTICLE_TYPE_EMITTER)
    {
        if (particle.age.x >= 0.01f)
        {
            particle.age.x = 0.0f;
            output.Append(particle);

            //float4 f4Random = gRandomBuffer.Load(int(fmod(gfCurrentTime - floor(gfCurrentTime) * 1000.0f, 1000.0f)));
            //float3 pf3Positions[8];
            
            
            for (int j = 0; j < 8; j++)
            {
                float4 f4Random2 = gRandomBuffer.Load( int(fmod( (gfCurrentTime - floor(gfCurrentTime)) * 1000.0f, 1000.0f) ) );
                
                particle.type = PARTICLE_TYPE_FLARE;
                
                particle.color = float3(1.f, 1.f, 1.f);
                particle.position = f4Random2.xyz * 3.f;
                particle.velocity = float3(0.0f, 3.0f, 0.0f);
                particle.acceleration = float3(10.0f * f4Random2.y, 1000.f * abs(f4Random2.x), 10.0f * f4Random2.z);
                //particle.acceleration = float3(10.0f, 250.f, 10.0f);
                
                particle.age.y = 1.f; //수명
                particle.alpha = 1.f; //알파값
                output.Append(particle);
            }
        }
    }
    else
    {
         //수명까지
        if (particle.age.x <= particle.age.y)
        {
            particle.position += (0.5f * particle.acceleration * gfElapsedTime * gfElapsedTime) + (particle.velocity * gfElapsedTime);
            
            float x = particle.age.x;
            float y = particle.age.y;
            //particle.alpha = saturate(1.f - ((y - x) / y)); // 1~0값
            
            //너무 많은 append로 인해 알파값이 0이 되기전에 사라짐, 따라서 수명을 짧게하여 금방 사라지도록 함
            particle.alpha = saturate((y - x) / y); // 1~0값
            
            //particle.alpha = 0.1f;
            output.Append(particle);
        }
    }
    
    /////////////////////////////////////////////////////2
    //VS_PARTICLE_INPUT particle = input[0];
    //particle.age.x += gfElapsedTime;
    //if (particle.type == PARTICLE_TYPE_EMITTER)
    //{
    //    if (particle.age.x > 0.2f) //2초마다 생성
    //    {
    //        VS_PARTICLE_INPUT newParticle = input[0];
    //        for (int i = 0; i < 1 /*9*/; ++i)
    //        {
    //            float4 f4Random = gRandomBuffer.Load(int(fmod((gfCurrentTime - floor(gfCurrentTime)) * 1000.0f, 1000.0f)));
    //            //float4 f4Random = gRandomBuffer.Load(int((gfCurrentTime * 1000.f) % 1000.f));
    //            //float4 f4Random = gRandomBuffer.Load(int( (gfCurrentTime - floor(gfCurrentTime))  * 1000.0f));
                
				
    //            //f4Random = normalize(f4Random);
    //            //f4Random.x = (f4Random.x * 500.f) % 500.f;
    //            //f4Random.z = (f4Random.z * 500.f) % 500.f;
    //            f4Random *= 500.f;
    //            f4Random.y = 20.f;

    //            newParticle.position = float3(f4Random.x, f4Random.y, f4Random.z);
    //            //newParticle.position = float3(0.f + i * 10.f, 40.f, 0.f);
    //            //float fColor = normalize(f4Random.x);
    //            //newParticle.color = float3(fColor, fColor, fColor);
    //            newParticle.velocity = float3(0.f, 1.f, 0.f);
    //            newParticle.age = float2(0.f, 10.f);
    //            newParticle.type = PARTICLE_TYPE_FLARE; // 0?
    //            output.Append(newParticle);
    //        }
			
    //        particle.age.x = 0.f;
    //    }
    //    output.Append(particle);
    //}
    //else
    //{
    //    if (particle.age.x <= particle.age.y) //lifetime
    //    {
    //        output.Append(particle);
    //    }
    //}
		
	
}


//1번
VS_PARTICLE_INPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
    return (input);
	
    //VS_PARTICLE_INPUT output = input;
    //float t = input.age.x;
    ////output.position = (input.velocity * t * 100.f) + input.position;
    //float3 f3Acceleration = float3(0.f, 40.f, 1.f);
    //output.position = (0.5f * f3Acceleration * t * t) + (input.velocity * t * 10.f) + input.position;
    
    //return output;
}

struct GS_PARTICLE_OUTPUT
{
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
    float2 age : AGELIFETIME; //(Age, Lifetime)
    float alpha : ALPHA;
    uint type : PARTICLETYPE;
};

static float2 gf2QuadUVs[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

//빌보드 사각형으로 파티클 그림
[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_INPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> outputStream)
{
    float4 pVertices[4];
//	GetBillboardCorners(input[0].position, input[0].size * 0.5f, pVertices);
    GetBillboardCorners(mul(float4(input[0].position, 1.0f), gmtxGameObject).xyz, input[0].size * 0.5f, pVertices);

    GS_PARTICLE_OUTPUT output = (GS_PARTICLE_OUTPUT) 0;
    output.color = input[0].color;
    output.age = input[0].age;
    output.type = input[0].type;
    output.alpha = input[0].alpha;
    
    for (int i = 0; i < 4; i++)
    {
        output.position = mul(mul(pVertices[i], gmtxView), gmtxProjection);
        output.uv = gf2QuadUVs[i];

        outputStream.Append(output);
    }
}

float4 PSParticleDraw(GS_PARTICLE_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
    
    //cColor.a = cColor.r;
    
    cColor.a = input.alpha;
    cColor.a *= cColor.r;
    
    return (cColor);
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

