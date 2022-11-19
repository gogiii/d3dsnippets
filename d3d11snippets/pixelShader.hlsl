#include "common.hlsl"

float4 main(PixelIn pin) : SV_TARGET {
	return pin.col;
}