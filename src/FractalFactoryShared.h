#pragma once
#include <cstdint>

static constexpr const wchar_t* FRACTAL_FACTORY_MAPPING = L"Local\\FractalFactoryControlState_v1";
static constexpr std::uint32_t FRACTAL_FACTORY_MAGIC = 0x46464331u; // FFC1
static constexpr int FRACTAL_FACTORY_PARAM_COUNT = 19;

struct FractalFactorySharedState {
    std::uint32_t magic = FRACTAL_FACTORY_MAGIC;
    std::uint32_t version = 1;
    volatile std::uint32_t sequence = 0;
    float values[FRACTAL_FACTORY_PARAM_COUNT] = {};
};
