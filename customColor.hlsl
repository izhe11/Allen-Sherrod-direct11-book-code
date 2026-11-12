cbuffer cbChangesPerFrame : register(b0)
{
    float4 col;
};


float4 VS_Main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}
float4 PS_Main(float4 pos : SV_POSITION) : SV_TARGET
{
    return col;
}