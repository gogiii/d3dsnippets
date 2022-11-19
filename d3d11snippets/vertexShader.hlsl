#include "common.hlsl"

cbuffer Matrices {
	matrix modelViewProjection;
};

struct VertexIn {
	float3 pos : POSITION;
	float4 col : COLOR;
};

PixelIn main(VertexIn vin) {
	PixelIn vout;
	vout.pos = mul(modelViewProjection, float4(vin.pos, 1));
	vout.col = vin.col;
	return vout;
}