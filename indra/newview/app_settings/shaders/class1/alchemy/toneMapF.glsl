/** 
 * @file toneMapF.glsl
 *
 * $LicenseInfo:firstyear=2021&license=viewerlgpl$
 * Alchemy Viewer Source Code
 * Copyright (C) 2021, Rye Mutt<rye@alchemyviewer.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * $/LicenseInfo$
 */

/*[EXTRA_CODE_HERE]*/

#ifdef DEFINE_GL_FRAGCOLOR
out vec4 frag_color;
#else
#define frag_color gl_FragColor
#endif

VARYING vec2 vary_fragcoord;

uniform sampler2DRect diffuseRect;

uniform vec2 screen_res;
uniform float exposure;

vec3 linear_to_srgb(vec3 cl);

vec3 reinhard(vec3 x)
{
	return x/(1+x);
}

vec3 reinhard2(vec3 x) {
    const float L_white = 4.0;
    return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

vec3 filmic(vec3 color)
{
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

vec3 unreal(vec3 x) {
    // Unreal 3, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return x / (x + 0.155) * 1.019;
}

vec3 aces(vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

vec3 uchimura(vec3 x) {
  const float P = 1.0;  // max display brightness
  const float a = 1.0;  // contrast
  const float m = 0.22; // linear section start
  const float l = 0.4;  // linear section length
  const float c = 1.33; // black
  const float b = 0.0;  // pedestal

  return uchimura(x, P, a, m, l, c, b);
}

// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
vec3 lottes(vec3 x) {
  const vec3 a = vec3(1.6);
  const vec3 d = vec3(0.977);
  const vec3 hdrMax = vec3(8.0);
  const vec3 midIn = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b =
      (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
      (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

void main()
{
    vec4 diff = texture2DRect(diffuseRect, vary_fragcoord);

#if TONEMAP_METHOD != 0
    // Exposure adjustment
    diff.rgb *= exposure; 
#endif

#if TONEMAP_METHOD == 0 // None, Gamma Correct Only
    #define NEEDS_GAMMA_CORRECT 1
#elif TONEMAP_METHOD == 1 // Linear 
    #define NEEDS_GAMMA_CORRECT 1
#elif TONEMAP_METHOD == 2 // Reinhard method
    #define NEEDS_GAMMA_CORRECT 1
    diff.rgb = reinhard(diff.rgb);
#elif TONEMAP_METHOD == 3 // Reinhard2 method
    #define NEEDS_GAMMA_CORRECT 1
    diff.rgb = reinhard2(diff.rgb);
#elif TONEMAP_METHOD == 4 // Filmic method
    diff.rgb = filmic(diff.rgb);
#elif TONEMAP_METHOD == 5 // Unreal method
    diff.rgb = unreal(diff.rgb);
#elif TONEMAP_METHOD == 6 // Aces method
    #define NEEDS_GAMMA_CORRECT 1
    diff.rgb = aces(diff.rgb);
#elif TONEMAP_METHOD == 7 // Uchimura's Gran Turismo method
    #define NEEDS_GAMMA_CORRECT 1
    diff.rgb = uchimura(diff.rgb);
#elif TONEMAP_METHOD == 8 // Lottes 2016
    #define NEEDS_GAMMA_CORRECT 1
    diff.rgb = lottes(diff.rgb);
#else
    #define NEEDS_GAMMA_CORRECT 1
#endif

#if NEEDS_GAMMA_CORRECT
    diff.rgb = linear_to_srgb(diff.rgb);
#endif
    frag_color = diff;
}
