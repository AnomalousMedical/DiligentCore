/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

#include <stdio.h>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <mutex>

#include "LinuxFileSystem.hpp"
#include "Errors.hpp"
#include "DebugUtilities.hpp"

namespace Diligent
{

LinuxFile* LinuxFileSystem::OpenFile(const FileOpenAttribs& OpenAttribs)
{
    LinuxFile* pFile = nullptr;
    try
    {
        pFile = new LinuxFile{OpenAttribs};
    }
    catch (const std::runtime_error& err)
    {
    }
    return pFile;
}

bool LinuxFileSystem::FileExists(const Char* strFilePath)
{
    FileOpenAttribs OpenAttribs;
    OpenAttribs.strFilePath = strFilePath;
    BasicFile   DummyFile{OpenAttribs};
    const auto& Path   = DummyFile.GetPath(); // This is necessary to correct slashes
    FILE*       pFile  = fopen(Path.c_str(), "r");
    bool        Exists = (pFile != nullptr);
    if (Exists)
        fclose(pFile);
    return Exists;
}

bool LinuxFileSystem::PathExists(const Char* strPath)
{
    std::string path(strPath);
    CorrectSlashes(path);

    auto res = access(path.c_str(), R_OK);
    return res == 0;
}

bool LinuxFileSystem::CreateDirectory(const Char* strPath)
{
    if (strPath == nullptr || strPath[0] == '\0')
    {
        UNEXPECTED("Path must not be null or empty");
        return false;
    }

    std::string path = strPath;
    CorrectSlashes(path);

    bool   result   = true;
    size_t position = 0;
    while (result != false && position != std::string::npos)
    {
        position     = path.find(SlashSymbol, position + 1);
        auto subPath = path.substr(0, position);
        if (!PathExists(subPath.c_str()))
            result = mkdir(subPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
    }
    return result;
}

void LinuxFileSystem::ClearDirectory(const Char* strPath)
{
    UNSUPPORTED("Not implemented");
}

void LinuxFileSystem::DeleteFile(const Char* strPath)
{
    remove(strPath);
}

bool LinuxFileSystem::DeleteDirectory(const Char* strPath)
{
    std::string path(strPath);
    CorrectSlashes(path);

    auto Callaback = [](const char* Path, const struct stat* pStat, int Type, FTW* pFTWB) -> int {
        if (remove(Path) < 0)
            return -1;
        return 0;
    };

    constexpr Int32 MaxOpenDirectory = 16;

    const auto res = nftw(path.c_str(), Callaback, MaxOpenDirectory, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
    return res == 0;
}

std::vector<std::unique_ptr<FindFileData>> LinuxFileSystem::Search(const Char* SearchPattern)
{
    UNSUPPORTED("Not implemented");
    return std::vector<std::unique_ptr<FindFileData>>();
}

// popen/pclose are not thread-safe
static std::mutex g_popen_mtx{};

FILE* LinuxFileSystem::popen(const char* command, const char* type)
{
    std::lock_guard<std::mutex> Lock{g_popen_mtx};
    return ::popen(command, type);
}

int LinuxFileSystem::pclose(FILE* stream)
{
    std::lock_guard<std::mutex> Lock{g_popen_mtx};
    return ::pclose(stream);
}

} // namespace Diligent
