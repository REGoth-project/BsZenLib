#include "$ENGINE$\GBufferOutput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\PerObjectData.bslinc"

shader Surface
{
	mixin GBufferOutput;
    mixin PerCameraData;
    mixin PerObjectData;
	
	code
	{
		[alias(gAlbedoTex)]
		SamplerState gAlbedoSamp;
		
		Texture2D gAlbedoTex = white;
				
		struct ZenVertexInput
		{
			float3 position : POSITION;
			float2 uv0 : TEXCOORD0;
			float3 normal : NORMAL;		
			float4 color : COLOR;		
		};
		
		struct ZenVStoFS
		{
			// Position in clip space (multiplied by world-view-projection matrix)
			float4 position : SV_Position;
			
			// Texture coordinates
			float2 uv0 : TEXCOORD0;
			
			// Position in world space (multiplied by the world matrix)
			float3 worldPosition : TEXCOORD1;
			
			// Normal vector in world space (multiplied by the world matrix)
			float3 worldNormal : NORMAL;
			
			// Per-vertex color
			float4 color : COLOR;
		};

		
		ZenVStoFS vsmain(ZenVertexInput input)
        {
            ZenVStoFS output;
			
            float4 worldPosition = mul(gMatWorld, float4(input.position, 1.0f));
            
			output.uv0 = input.uv0;
            output.worldPosition = worldPosition.xyz;
            output.position = mul(gMatViewProj, worldPosition);
			output.worldNormal = mul((float3x3)gMatWorld, input.normal);
			output.color = float4(input.color.rrr, 1.0);
                        
            return output;
        }
		
		void fsmain(
			in ZenVStoFS input, 
			out float3 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3,
			out float OutGBufferD : SV_Target4)
		{
			float2 uv = input.uv0;
		
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, uv);
			surfaceData.worldNormal.xyz = input.worldNormal;
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			
			clip(surfaceData.albedo.a - 0.5f);

			encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC, OutGBufferD);
			
			OutSceneColor = surfaceData.albedo.rgb * input.color.rgb * 0.1f;
		}	
	};
};

subshader DeferredDirectLighting
{
    // An example shader that implements the basic Lambert BRDF
    mixin StandardBRDF
    {
        code
        {   
            float3 evaluateStandardBRDF(float3 V, float3 L, float specLobeEnergy, SurfaceData surfaceData)
            {
                return surfaceData.albedo.rgb / 3.14f;
            }
        };
    };

};