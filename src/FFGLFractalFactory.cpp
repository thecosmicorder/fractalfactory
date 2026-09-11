#include "FFGLFractalFactory.h"
#include <algorithm>
#include <chrono>
#include <cmath>

using namespace ffglex;

enum ParamType : FFUInt32 {
    PT_TYPE, PT_ZOOM, PT_PANX, PT_PANY, PT_ROT, PT_ITER, PT_POWER, PT_JULIAX, PT_JULIAY,
    PT_CHAOS, PT_CHAOSRATE, PT_SYM, PT_WARP, PT_HUE, PT_SAT, PT_BRIGHT, PT_CONTRAST, PT_SEED, PT_PRESET
};

static CFFGLPluginInfo PluginInfo(
    PluginFactory<FFGLFractalFactory>,
    "FF01", "Fractal Factory",
    2, 1, 1, 0,
    FF_SOURCE,
    "GPU fractal and chaos visual synthesizer with detachable control UI",
    "Fractal Factory"
);

static const char vertexShaderCode[] = R"GLSL(#version 410 core
layout(location=0) in vec4 vPosition;
layout(location=1) in vec2 vUV;
out vec2 uv;
void main(){ gl_Position=vPosition; uv=vUV; }
)GLSL";

static const char fragmentShaderCode[] = R"GLSL(#version 410 core
uniform vec2 resolution;
uniform float time;
uniform int fractalType;
uniform float zoom;
uniform vec2 pan;
uniform float rotation;
uniform int iterations;
uniform float power;
uniform vec2 juliaC;
uniform float chaos;
uniform float chaosRate;
uniform float symmetry;
uniform float warp;
uniform float hue;
uniform float saturation;
uniform float brightness;
uniform float contrast;
uniform float seed;
in vec2 uv;
out vec4 fragColor;

float hash11(float x){ return fract(sin(x*127.1+seed*311.7)*43758.5453123); }
vec3 hsv2rgb(vec3 c){ vec3 q=abs(fract(c.xxx+vec3(0.0,2.0/3.0,1.0/3.0))*6.0-3.0); return c.z*mix(vec3(1.0),clamp(q-1.0,0.0,1.0),c.y); }
vec2 cpow2(vec2 z,float pw){ float r=length(z); float a=atan(z.y,z.x); float rp=pow(max(r,1e-8),pw); return rp*vec2(cos(a*pw),sin(a*pw)); }
float trap(vec2 z){ return min(abs(length(z)-0.5),min(abs(z.x),abs(z.y))); }

float escapeSet(vec2 c,float pw,int kind,out float tr){
    vec2 z=vec2(0.0), prev=vec2(0.0); float m=0.0; tr=10.0; int i=0;
    for(i=0;i<512;i++){
        if(i>=iterations) break;
        vec2 old=z;
        if(kind==1) z=vec2(z.x*z.x-z.y*z.y,2.0*abs(z.x*z.y));
        else if(kind==2) z=cpow2(vec2(z.x,-z.y),pw);
        else if(kind==3) z=cpow2(z,pw)+0.18*prev;
        else z=cpow2(z,pw);
        prev=old; z+=c; tr=min(tr,trap(z)); m=dot(z,z); if(m>256.0) break;
    }
    if(i>=iterations) return float(iterations);
    return float(i)+1.0-log2(max(log2(max(m,1.0001)),0.0001));
}

float juliaSet(vec2 z,vec2 c,float pw,out float tr){
    float m=0.0; tr=10.0; int i=0;
    for(i=0;i<512;i++){ if(i>=iterations) break; z=cpow2(z,pw)+c; tr=min(tr,trap(z)); m=dot(z,z); if(m>256.0) break; }
    if(i>=iterations) return float(iterations);
    return float(i)+1.0-log2(max(log2(max(m,1.0001)),0.0001));
}

float newton(vec2 z){
    float d=0.0;
    for(int i=0;i<64;i++){
        if(i>=iterations/4) break;
        vec2 z2=vec2(z.x*z.x-z.y*z.y,2.0*z.x*z.y);
        vec2 z3=vec2(z2.x*z.x-z2.y*z.y,z2.x*z.y+z2.y*z.x);
        vec2 f=z3-vec2(1.0,0.0);
        vec2 fp=3.0*z2;
        float den=max(dot(fp,fp),1e-8);
        vec2 q=vec2(f.x*fp.x+f.y*fp.y,f.y*fp.x-f.x*fp.y)/den;
        z-=q; d=float(i); if(length(f)<0.0001) break;
    }
    float a=atan(z.y,z.x); return d+12.0*fract((a+3.14159265)/6.2831853*3.0);
}

float sierpinski(vec2 p){
    p=(p+1.0)*0.5; float v=1.0;
    for(int i=0;i<11;i++){ if(p.x+p.y>1.0) p=1.0-p; p=fract(p*2.0); v*=step(p.x+p.y,1.0); }
    return v;
}

float carpet(vec2 p){
    p=(p+1.0)*1.5; float keep=1.0;
    for(int i=0;i<8;i++){ vec2 q=floor(mod(p,3.0)); if(q.x==1.0&&q.y==1.0) keep=0.0; p*=3.0; }
    return keep;
}

float dragon(vec2 p){
    vec2 z=p; float d=10.0;
    for(int i=0;i<20;i++){ z=abs(z); if(z.x<z.y) z=z.yx; z=mat2(0.707106,-0.707106,0.707106,0.707106)*vec2(z.x,-z.y)-vec2(0.45,0.0); d=min(d,length(z)); }
    return exp(-20.0*d);
}

float strange(vec2 p,int kind){
    vec2 z=vec2(hash11(seed)*0.2,hash11(seed+2.0)*0.2); float d=10.0;
    float a=1.4+1.2*hash11(seed+3.0), b=-2.2+1.5*hash11(seed+5.0), c=1.2+1.2*hash11(seed+7.0), e=-1.8+1.6*hash11(seed+9.0);
    for(int i=0;i<220;i++){
        if(kind==0) z=vec2(sin(a*z.y)+c*cos(a*z.x),sin(b*z.x)+e*cos(b*z.y));
        else z=vec2(sin(a*z.y)-cos(b*z.x),sin(c*z.x)-cos(e*z.y));
        d=min(d,length(p-z*0.36));
    }
    return exp(-60.0*d);
}

float henon(vec2 p){
    vec2 z=vec2(0.1,0.1); float d=10.0; float a=1.2+0.25*hash11(seed+1.0), b=0.22+0.12*hash11(seed+2.0);
    for(int i=0;i<260;i++){ z=vec2(1.0-a*z.x*z.x+z.y,b*z.x); d=min(d,length(p-z*0.55)); }
    return exp(-75.0*d);
}

float ikeda(vec2 p){
    vec2 z=vec2(0.1); float d=10.0; float u=0.82+0.12*hash11(seed+4.0);
    for(int i=0;i<260;i++){ float t=0.4-6.0/(1.0+dot(z,z)); float ct=cos(t),st=sin(t); z=vec2(1.0+u*(z.x*ct-z.y*st),u*(z.x*st+z.y*ct)); d=min(d,length(p-(z-vec2(1.0,0.0))*0.38)); }
    return exp(-70.0*d);
}

float logisticField(vec2 p){
    float r=3.4+0.6*(p.x*0.5+0.5); float x=0.15+0.7*hash11(seed+11.0); float d=10.0;
    for(int i=0;i<160;i++){ x=r*x*(1.0-x); d=min(d,abs(p.y-(x*2.0-1.0))); }
    return exp(-90.0*d);
}

void main(){
    vec2 p=uv*2.0-1.0; p.x*=resolution.x/max(resolution.y,1.0);
    float a=rotation*6.2831853; p=mat2(cos(a),-sin(a),sin(a),cos(a))*p;
    float zf=exp2((zoom*2.0-1.0)*12.0); p=p/max(zf,1e-6)+(pan-0.5)*4.0;

    // deterministic chaos modulation: coupled logistic oscillators plus multi-frequency phase motion
    float lx=0.123+0.77*hash11(seed+31.0); float rr=3.55+0.44*chaos;
    for(int i=0;i<14;i++) lx=rr*lx*(1.0-lx);
    float ch=chaos*((lx-0.5)*0.8+0.45*sin(time*chaosRate*2.17+seed*17.0)+0.25*sin(time*chaosRate*5.31+seed*41.0));
    p+=ch*0.10*vec2(sin(p.y*7.0+time*chaosRate),cos(p.x*6.0-time*chaosRate*0.7));

    if(symmetry>0.02){ float n=2.0+floor(symmetry*14.0); float ang=atan(p.y,p.x), rad=length(p); ang=abs(mod(ang,6.2831853/n)-3.14159265/n); p=rad*vec2(cos(ang),sin(ang)); }
    p+=warp*0.18*vec2(sin(p.y*5.0+time*0.3),sin(p.x*6.0-time*0.2));

    float v=0.0,tr=0.0; int ft=fractalType;
    if(ft==0) v=escapeSet(p,power,0,tr);
    else if(ft==1) v=juliaSet(p,juliaC,power,tr);
    else if(ft==2) v=escapeSet(p,2.0,1,tr);
    else if(ft==3) v=escapeSet(p,max(2.0,power),2,tr);
    else if(ft==4) v=escapeSet(p,max(2.2,power+0.8),0,tr);
    else if(ft==5) v=newton(p);
    else if(ft==6) v=escapeSet(p,2.0,3,tr);
    else if(ft==7) v=sierpinski(p)*float(iterations);
    else if(ft==8) v=carpet(p)*float(iterations);
    else if(ft==9) v=dragon(p)*float(iterations);
    else if(ft==10) v=strange(p,0)*float(iterations);
    else if(ft==11) v=strange(p,1)*float(iterations);
    else if(ft==12) v=henon(p)*float(iterations);
    else if(ft==13) v=ikeda(p)*float(iterations);
    else if(ft==14) v=logisticField(p)*float(iterations);
    else { v=escapeSet(p,power,0,tr)+max(0.0,1.0-tr*10.0)*22.0; }

    float norm=v/max(float(iterations),1.0);
    if(ft>=7&&ft<=14) norm=clamp(norm,0.0,1.0);
    float h=fract(hue+0.88*norm+0.10*sin(norm*13.0+time*0.18));
    float val=pow(clamp(norm,0.0,1.0),0.44);
    vec3 col=hsv2rgb(vec3(h,saturation,val*brightness));
    col=(col-0.5)*(0.25+contrast*2.1)+0.5;
    col+=(hash11(dot(gl_FragCoord.xy,vec2(12.9898,78.233))+floor(time*60.0))-0.5)/1023.0;
    fragColor=vec4(clamp(col,0.0,1.0),1.0);
}
)GLSL";

static float frand01(std::uint32_t& s){
    s = s * 1664525u + 1013904223u;
    return float((s >> 8) & 0x00FFFFFFu) / float(0x01000000u);
}

FFGLFractalFactory::FFGLFractalFactory(){
    SetMinInputs(0); SetMaxInputs(0);
    const char* names[FRACTAL_FACTORY_PARAM_COUNT] = {
        "Fractal Type","Zoom","Pan X","Pan Y","Rotation","Iterations","Power","Julia X","Julia Y",
        "Chaos","Chaos Rate","Symmetry","Warp","Hue","Saturation","Brightness","Contrast","Seed","Preset"
    };
    for(unsigned i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++) SetParamInfof(i,names[i],FF_TYPE_STANDARD);

    mapping = CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(FractalFactorySharedState),FRACTAL_FACTORY_MAPPING);
    if(mapping){
        shared = static_cast<FractalFactorySharedState*>(MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(FractalFactorySharedState)));
        if(shared && shared->magic != FRACTAL_FACTORY_MAGIC){
            shared->magic=FRACTAL_FACTORY_MAGIC; shared->version=1; shared->sequence=1;
            for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++) shared->values[i]=p[i];
        }
    }
}

FFGLFractalFactory::~FFGLFractalFactory(){
    if(shared) UnmapViewOfFile(shared);
    if(mapping) CloseHandle(mapping);
}

void FFGLFractalFactory::ApplyPreset(int preset){
    preset=std::clamp(preset,0,255);
    std::uint32_t s=0x9E3779B9u ^ (std::uint32_t(preset+1)*2654435761u);
    int family=preset/16;
    p[PT_TYPE]=float(family)/15.0f;
    p[PT_ZOOM]=0.25f+0.48f*frand01(s);
    p[PT_PANX]=0.35f+0.30f*frand01(s); p[PT_PANY]=0.35f+0.30f*frand01(s);
    p[PT_ROT]=frand01(s); p[PT_ITER]=0.24f+0.70f*frand01(s); p[PT_POWER]=0.10f+0.80f*frand01(s);
    p[PT_JULIAX]=frand01(s); p[PT_JULIAY]=frand01(s);
    p[PT_CHAOS]=(family>=10?0.25f:0.05f)+0.62f*frand01(s); p[PT_CHAOSRATE]=0.08f+0.72f*frand01(s);
    p[PT_SYM]=(family==7||family==8||family==9)?0.25f+0.55f*frand01(s):0.35f*frand01(s);
    p[PT_WARP]=0.08f+0.62f*frand01(s); p[PT_HUE]=frand01(s); p[PT_SAT]=0.55f+0.45f*frand01(s);
    p[PT_BRIGHT]=0.45f+0.45f*frand01(s); p[PT_CONTRAST]=0.35f+0.45f*frand01(s); p[PT_SEED]=frand01(s);
    p[PT_PRESET]=float(preset)/255.0f;
}

void FFGLFractalFactory::PollDetachedUI(){
    if(!shared || shared->magic!=FRACTAL_FACTORY_MAGIC) return;
    std::uint32_t seq=shared->sequence;
    if(seq==lastSequence) return;
    for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++) p[i]=std::clamp(shared->values[i],0.0f,1.0f);
    lastSequence=seq;
}

FFResult FFGLFractalFactory::InitGL(const FFGLViewportStruct* vp){
    if(!shader.Compile(vertexShaderCode,fragmentShaderCode)){ DeInitGL(); return FF_FAIL; }
    if(!quad.Initialise()){ DeInitGL(); return FF_FAIL; }
    ScopedShaderBinding b(shader.GetGLID());
    uResolution=shader.FindUniform("resolution"); uTime=shader.FindUniform("time"); uType=shader.FindUniform("fractalType");
    uZoom=shader.FindUniform("zoom"); uPan=shader.FindUniform("pan"); uRot=shader.FindUniform("rotation"); uIter=shader.FindUniform("iterations");
    uPower=shader.FindUniform("power"); uJulia=shader.FindUniform("juliaC"); uChaos=shader.FindUniform("chaos"); uChaosRate=shader.FindUniform("chaosRate");
    uSym=shader.FindUniform("symmetry"); uWarp=shader.FindUniform("warp"); uHue=shader.FindUniform("hue"); uSat=shader.FindUniform("saturation");
    uBright=shader.FindUniform("brightness"); uContrast=shader.FindUniform("contrast"); uSeed=shader.FindUniform("seed");
    return CFFGLPlugin::InitGL(vp);
}

FFResult FFGLFractalFactory::ProcessOpenGL(ProcessOpenGLStruct*){
    PollDetachedUI();
    static auto t0=std::chrono::steady_clock::now();
    float t=std::chrono::duration<float>(std::chrono::steady_clock::now()-t0).count();
    ScopedShaderBinding b(shader.GetGLID());
    glUniform2f(uResolution,float(currentViewport.width),float(currentViewport.height));
    glUniform1f(uTime,t); glUniform1i(uType,int(std::round(p[PT_TYPE]*15.0f))); glUniform1f(uZoom,p[PT_ZOOM]);
    glUniform2f(uPan,p[PT_PANX],p[PT_PANY]); glUniform1f(uRot,p[PT_ROT]); glUniform1i(uIter,24+int(p[PT_ITER]*488.0f));
    glUniform1f(uPower,1.5f+p[PT_POWER]*5.5f); glUniform2f(uJulia,p[PT_JULIAX]*2.0f-1.0f,p[PT_JULIAY]*2.0f-1.0f);
    glUniform1f(uChaos,p[PT_CHAOS]); glUniform1f(uChaosRate,0.04f+p[PT_CHAOSRATE]*4.5f); glUniform1f(uSym,p[PT_SYM]);
    glUniform1f(uWarp,p[PT_WARP]); glUniform1f(uHue,p[PT_HUE]); glUniform1f(uSat,p[PT_SAT]); glUniform1f(uBright,0.25f+p[PT_BRIGHT]*1.75f);
    glUniform1f(uContrast,p[PT_CONTRAST]); glUniform1f(uSeed,p[PT_SEED]);
    quad.Draw(); return FF_SUCCESS;
}

FFResult FFGLFractalFactory::DeInitGL(){
    shader.FreeGLResources(); quad.Release(); return FF_SUCCESS;
}

FFResult FFGLFractalFactory::SetFloatParameter(unsigned int i,float v){
    if(i>=FRACTAL_FACTORY_PARAM_COUNT) return FF_FAIL;
    if(i==PT_PRESET) ApplyPreset(int(std::round(std::clamp(v,0.0f,1.0f)*255.0f)));
    else p[i]=std::clamp(v,0.0f,1.0f);
    return FF_SUCCESS;
}

float FFGLFractalFactory::GetFloatParameter(unsigned int i){ return i<FRACTAL_FACTORY_PARAM_COUNT?p[i]:0.0f; }
