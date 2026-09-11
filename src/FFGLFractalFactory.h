#pragma once
#include <FFGLSDK.h>
#include <windows.h>
#include "FractalFactoryShared.h"

class FFGLFractalFactory : public CFFGLPlugin {
public:
    FFGLFractalFactory();
    ~FFGLFractalFactory() override;
    FFResult InitGL(const FFGLViewportStruct* vp) override;
    FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
    FFResult DeInitGL() override;
    FFResult SetFloatParameter(unsigned int index, float value) override;
    float GetFloatParameter(unsigned int index) override;

private:
    void ApplyPreset(int preset);
    void PollDetachedUI();

    ffglex::FFGLShader shader;
    ffglex::FFGLScreenQuad quad;
    GLint uResolution=-1,uTime=-1,uType=-1,uZoom=-1,uPan=-1,uRot=-1,uIter=-1,uPower=-1,uJulia=-1,uChaos=-1,uChaosRate=-1,uSym=-1,uWarp=-1,uHue=-1,uSat=-1,uBright=-1,uContrast=-1,uSeed=-1;
    float p[FRACTAL_FACTORY_PARAM_COUNT] = {
        0.0f, 0.35f, 0.5f, 0.5f, 0.5f, 0.55f, 0.5f, 0.5f, 0.25f,
        0.35f, 0.5f, 0.2f, 0.6f, 0.6f, 0.8f, 0.5f, 0.5f, 0.1234f, 0.0f
    };
    HANDLE mapping = nullptr;
    FractalFactorySharedState* shared = nullptr;
    std::uint32_t lastSequence = 0;
};
