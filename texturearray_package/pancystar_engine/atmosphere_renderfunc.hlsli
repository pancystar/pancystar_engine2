#include"atmosphere_function.hlsli"

static const float3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = float3(114974.916437, 71305.954816, 65310.548555);
static const float3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = float3(98242.786222, 69954.398112, 66475.012354);

#define RADIANCE_API_ENABLED

Texture2D transmittance_texture;
Texture3D scattering_texture;
Texture3D single_mie_scattering_texture;
Texture2D irradiance_texture;
#ifdef RADIANCE_API_ENABLED
static const DensityProfile data_check1 = {
	{ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 },
	{ 0.000000, 1.000000, -0.125000, 0.000000, 0.000000 }
};
static const DensityProfile data_check2 =
{
	{ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 },
	{ 0.000000, 1.000000, -0.833333, 0.000000, 0.000000 }
};
static const DensityProfile data_check3 =
{
	{ 25.000000, 0.000000, 0.000000, 0.066667, -0.666667 },
	{ 0.000000, 0.000000, 0.000000, -0.066667, 2.666667 }
};

RadianceSpectrum GetSolarRadiance()
{
	return ATMOSPHERE.solar_irradiance / (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius);
}
RadianceSpectrum GetSkyRadiance(
	Position camera, Direction view_ray, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance)
{
	return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
		scattering_texture, single_mie_scattering_texture,
		camera, view_ray, shadow_length, sun_direction, transmittance);
}
RadianceSpectrum GetSkyRadianceToPoint(
	Position camera, Position point_in, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance)
{
	return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
		scattering_texture, single_mie_scattering_texture,
		camera, point_in, shadow_length, sun_direction, transmittance);
}
IrradianceSpectrum GetSunAndSkyIrradiance(
	Position p, Direction normal, Direction sun_direction,
	out IrradianceSpectrum sky_irradiance)
{
	return GetSunAndSkyIrradiance(ATMOSPHERE, transmittance_texture,
		irradiance_texture, p, normal, sun_direction, sky_irradiance);
}
#endif
Luminance3 GetSolarLuminance()
{
	return ATMOSPHERE.solar_irradiance /
		(PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius) *
		SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminance(
	Position camera, Direction view_ray, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance)
{
	return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
		scattering_texture, single_mie_scattering_texture,
		camera, view_ray, shadow_length, sun_direction, transmittance) *
		SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminanceToPoint(
	Position camera, Position point_in, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance)
{
	return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
		scattering_texture, single_mie_scattering_texture,
		camera, point_in, shadow_length, sun_direction, transmittance) *
		SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Illuminance3 GetSunAndSkyIlluminance(
	Position p, Direction normal, Direction sun_direction,
	out IrradianceSpectrum sky_irradiance)
{
	IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(
		ATMOSPHERE, transmittance_texture, irradiance_texture, p, normal,
		sun_direction, sky_irradiance);
	sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
	return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}