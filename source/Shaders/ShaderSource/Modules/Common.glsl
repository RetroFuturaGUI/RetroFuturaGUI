// Shared by the plane and frame shaders: the feature bits, which match ShaderFeatures in C++, the quad's size in pixels, and
// the quad position every shader reads
#define ROUNDED_CORNERS 1
#define GLASS_EFFECT 2
#define GlassEffectWithImage 6
#define RASTER 8
#define FOG_EFFECT 16
#define WAVE 32
#define BACKGROUND_IMAGE 64

uniform vec2 uScale;
uniform int uDIP;

in vec2 vLocalPos;
in vec2 vUV;
