/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "ArchiverImpl.hpp"
#include "Archiver_Inc.hpp"

#include "../../GraphicsEngineOpenGL/include/pch.h"
#include "RenderDeviceGLImpl.hpp"
#include "PipelineResourceSignatureGLImpl.hpp"
#include "PipelineStateGLImpl.hpp"
#include "ShaderGLImpl.hpp"
#include "DeviceObjectArchiveGLImpl.hpp"

#if !DILIGENT_NO_GLSLANG
#    include "GLSLUtils.hpp"
#    include "GLSLangUtils.hpp"
#endif

namespace Diligent
{

template <>
struct SerializableResourceSignatureImpl::SignatureTraits<PipelineResourceSignatureGLImpl>
{
    static constexpr DeviceType Type = DeviceType::OpenGL;

    template <SerializerMode Mode>
    using PSOSerializerType = PSOSerializerGL<Mode>;
};

namespace
{

struct ShaderStageInfoGL
{
    ShaderStageInfoGL() {}

    ShaderStageInfoGL(const SerializableShaderImpl* _pShader) :
        Type{_pShader->GetDesc().ShaderType},
        pShader{_pShader}
    {}

    // Needed only for ray tracing
    void Append(const SerializableShaderImpl*) {}

    Uint32 Count() const { return 1; }

    SHADER_TYPE                   Type    = SHADER_TYPE_UNKNOWN;
    const SerializableShaderImpl* pShader = nullptr;
};

inline SHADER_TYPE GetShaderStageType(const ShaderStageInfoGL& Stage) { return Stage.Type; }

} // namespace

template <typename CreateInfoType>
bool ArchiverImpl::PatchShadersGL(CreateInfoType& CreateInfo, TPSOData<CreateInfoType>& Data, DefaultPRSInfo& DefPRS)
{
    TShaderIndices ShaderIndices;

    std::vector<ShaderStageInfoGL> ShaderStages;
    SHADER_TYPE                    ActiveShaderStages = SHADER_TYPE_UNKNOWN;
    PipelineStateGLImpl::ExtractShaders<SerializableShaderImpl>(CreateInfo, ShaderStages, ActiveShaderStages);

    (void)GetShaderStageType(ShaderStageInfoGL{});

    // AZ TODO: default PRS

    for (size_t i = 0; i < ShaderStages.size(); ++i)
    {
        SerializeShaderSource(ShaderIndices, DeviceType::OpenGL, ShaderStages[i].pShader->GetCreateInfo());
    }
    Data.PerDeviceData[static_cast<size_t>(DeviceType::OpenGL)] = SerializeShadersForPSO(ShaderIndices);
    return true;
}

template bool ArchiverImpl::PatchShadersGL<GraphicsPipelineStateCreateInfo>(GraphicsPipelineStateCreateInfo& CreateInfo, TPSOData<GraphicsPipelineStateCreateInfo>& Data, DefaultPRSInfo& DefPRS);
template bool ArchiverImpl::PatchShadersGL<ComputePipelineStateCreateInfo>(ComputePipelineStateCreateInfo& CreateInfo, TPSOData<ComputePipelineStateCreateInfo>& Data, DefaultPRSInfo& DefPRS);
template bool ArchiverImpl::PatchShadersGL<TilePipelineStateCreateInfo>(TilePipelineStateCreateInfo& CreateInfo, TPSOData<TilePipelineStateCreateInfo>& Data, DefaultPRSInfo& DefPRS);
template bool ArchiverImpl::PatchShadersGL<RayTracingPipelineStateCreateInfo>(RayTracingPipelineStateCreateInfo& CreateInfo, TPSOData<RayTracingPipelineStateCreateInfo>& Data, DefaultPRSInfo& DefPRS);


template PipelineResourceSignatureGLImpl* SerializableResourceSignatureImpl::GetSignature<PipelineResourceSignatureGLImpl>() const;

void SerializableResourceSignatureImpl::CreatePRSGL(IReferenceCounters*                  pRefCounters,
                                                    const PipelineResourceSignatureDesc& Desc,
                                                    SHADER_TYPE                          ShaderStages)
{
    CreateSignature<PipelineResourceSignatureGLImpl>(pRefCounters, Desc, ShaderStages);
}


void SerializationDeviceImpl::GetPipelineResourceBindingsGL(const PipelineResourceBindingAttribs& Info,
                                                            std::vector<PipelineResourceBinding>& ResourceBindings)
{
    const auto            ShaderStages        = (Info.ShaderStages == SHADER_TYPE_UNKNOWN ? static_cast<SHADER_TYPE>(~0u) : Info.ShaderStages);
    constexpr SHADER_TYPE SupportedStagesMask = (SHADER_TYPE_ALL_GRAPHICS | SHADER_TYPE_COMPUTE);

    SignatureArray<PipelineResourceSignatureGLImpl> Signatures      = {};
    Uint32                                          SignaturesCount = 0;
    SortResourceSignatures(Info, Signatures, SignaturesCount);

    PipelineResourceSignatureGLImpl::TBindings BaseBindings = {};
    for (Uint32 s = 0; s < SignaturesCount; ++s)
    {
        const auto& pSignature = Signatures[s];
        if (pSignature == nullptr)
            continue;

        for (Uint32 r = 0; r < pSignature->GetTotalResourceCount(); ++r)
        {
            const auto& ResDesc = pSignature->GetResourceDesc(r);
            const auto& ResAttr = pSignature->GetResourceAttribs(r);
            const auto  Range   = PipelineResourceToBindingRange(ResDesc);

            for (auto Stages = ShaderStages & SupportedStagesMask; Stages != 0;)
            {
                const auto ShaderStage = ExtractLSB(Stages);
                if ((ResDesc.ShaderStages & ShaderStage) == 0)
                    continue;

                ResourceBindings.push_back(ResDescToPipelineResBinding(ResDesc, ShaderStage, BaseBindings[Range] + ResAttr.CacheOffset, 0 /*space*/));
            }
        }
        pSignature->ShiftBindings(BaseBindings);
    }
}

#if !DILIGENT_NO_GLSLANG
void SerializableShaderImpl::CreateShaderGL(IReferenceCounters* pRefCounters, ShaderCreateInfo& ShaderCI, String& CompilationLog)
{
    GLSLangUtils::GLSLtoSPIRVAttribs Attribs;

    Attribs.ShaderType = ShaderCI.Desc.ShaderType;
#    if GL_SUPPORTED
    Attribs.Version = GLSLangUtils::SpirvVersion::GL;
#    elif GLES_SUPPORTED
    Attribs.Version = GLSLangUtils::SpirvVersion::GLES;
#    endif

    // Mem leak when used RefCntAutoPtr
    IDataBlob* pLog          = nullptr;
    Attribs.ppCompilerOutput = &pLog;

    try
    {
        if (ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_HLSL)
        {
            if (GLSLangUtils::HLSLtoSPIRV(ShaderCI, Attribs.Version, "", Attribs.ppCompilerOutput).empty())
                LOG_ERROR_AND_THROW("HLSLtoSPIRV failed");
        }
        else if (ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_DEFAULT ||
                 ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_GLSL)
        {
            const auto GLSLSourceString = BuildGLSLSourceString(ShaderCI, m_pDevice->GetDeviceInfo(), m_pDevice->GetAdapterInfo(), TargetGLSLCompiler::glslang, "");

            Attribs.ShaderSource  = GLSLSourceString.c_str();
            Attribs.SourceCodeLen = static_cast<int>(GLSLSourceString.size());
            Attribs.Macros        = ShaderCI.Macros;

            if (GLSLangUtils::GLSLtoSPIRV(Attribs).empty())
                LOG_ERROR_AND_THROW("GLSLtoSPIRV failed");
        }
        else if (ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM)
        {
            if (ShaderCI.Macros != nullptr)
                LOG_WARNING_MESSAGE("Shader macros are ignored when compiling GLSL verbatim in OpenGL backend");

            Attribs.ShaderSource  = ShaderCI.Source;
            Attribs.SourceCodeLen = static_cast<int>(ShaderCI.SourceLength);

            if (GLSLangUtils::GLSLtoSPIRV(Attribs).empty())
                LOG_ERROR_AND_THROW("GLSLtoSPIRV failed");
        }
    }
    catch (...)
    {
        if (pLog && pLog->GetConstDataPtr())
        {
            CompilationLog += "Failed to compile OpenGL shader:\n";
            CompilationLog += static_cast<const char*>(pLog->GetConstDataPtr());
        }
    }

    if (pLog)
        pLog->Release();
}
#endif

} // namespace Diligent
