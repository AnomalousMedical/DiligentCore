/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include "FileSystem.hpp"

#include "gtest/gtest.h"

using namespace Diligent;

namespace
{

TEST(Platforms_FileSystem, SplitPath)
{
    auto TestSplitPath = [](const char* Path, bool Simplify, std::vector<std::string> RefComponents) {
        const auto Components = FileSystem::SplitPath(Path, Simplify);
        EXPECT_EQ(Components.size(), RefComponents.size());
        if (Components.size() != RefComponents.size())
            return;
        for (size_t i = 0; i < RefComponents.size(); ++i)
        {
            EXPECT_EQ(Components[i], RefComponents[i]);
        }
    };

    TestSplitPath("", true, {});
    TestSplitPath("", false, {});

    TestSplitPath("/", true, {});
    TestSplitPath("\\", true, {});
    TestSplitPath("/", false, {});
    TestSplitPath("\\", false, {});

    TestSplitPath("a/", true, {"a"});
    TestSplitPath("a\\", true, {"a"});
    TestSplitPath("a/", false, {"a"});
    TestSplitPath("a\\", false, {"a"});

    TestSplitPath("/a", true, {"a"});
    TestSplitPath("\\a", true, {"a"});
    TestSplitPath("/a", false, {"a"});
    TestSplitPath("\\a", false, {"a"});

    TestSplitPath("/a/", true, {"a"});
    TestSplitPath("\\a/", true, {"a"});
    TestSplitPath("/a/", false, {"a"});
    TestSplitPath("\\a/", false, {"a"});

    TestSplitPath("a/b", true, {"a", "b"});
    TestSplitPath("a\\b", true, {"a", "b"});
    TestSplitPath("a//b", true, {"a", "b"});
    TestSplitPath("a\\\\b", true, {"a", "b"});
    TestSplitPath("a/b", false, {"a", "b"});
    TestSplitPath("a\\b", false, {"a", "b"});
    TestSplitPath("a//b", false, {"a", "b"});
    TestSplitPath("a\\\\b", false, {"a", "b"});

    TestSplitPath("a/./b", true, {"a", "b"});
    TestSplitPath("a\\.\\b", true, {"a", "b"});
    TestSplitPath("a/./b", false, {"a", ".", "b"});
    TestSplitPath("a\\.\\b", false, {"a", ".", "b"});

    TestSplitPath("./a", true, {"a"});
    TestSplitPath(".\\a", true, {"a"});
    TestSplitPath("./a", false, {".", "a"});
    TestSplitPath(".\\a", false, {".", "a"});

    TestSplitPath("a/.", true, {"a"});
    TestSplitPath("a\\.", true, {"a"});
    TestSplitPath("a/.", false, {"a", "."});
    TestSplitPath("a\\.", false, {"a", "."});

    TestSplitPath("a./b", true, {"a.", "b"});
    TestSplitPath("a.\\b", true, {"a.", "b"});
    TestSplitPath("a./b", false, {"a.", "b"});
    TestSplitPath("a.\\b", false, {"a.", "b"});

    TestSplitPath("a/.b", true, {"a", ".b"});
    TestSplitPath("a\\.b", true, {"a", ".b"});
    TestSplitPath("a/.b", false, {"a", ".b"});
    TestSplitPath("a\\.b", false, {"a", ".b"});

    TestSplitPath("a.b/c", true, {"a.b", "c"});
    TestSplitPath("a.b\\c", true, {"a.b", "c"});
    TestSplitPath("a.b/c", false, {"a.b", "c"});
    TestSplitPath("a.b\\c", false, {"a.b", "c"});

    TestSplitPath("..", true, {".."});
    TestSplitPath("../a", true, {"..", "a"});
    TestSplitPath("..\\a", true, {"..", "a"});
    TestSplitPath("..", false, {".."});
    TestSplitPath("../a", false, {"..", "a"});
    TestSplitPath("..\\a", false, {"..", "a"});

    TestSplitPath("a/..", true, {});
    TestSplitPath("a\\..", true, {});
    TestSplitPath("a/..", false, {"a", ".."});
    TestSplitPath("a\\..", false, {"a", ".."});

    TestSplitPath("a/b/../c", true, {"a", "c"});
    TestSplitPath("a\\b\\..\\c", true, {"a", "c"});
    TestSplitPath("a/b/../c", false, {"a", "b", "..", "c"});
    TestSplitPath("a\\b\\..\\c", false, {"a", "b", "..", "c"});

    TestSplitPath("a../b", true, {"a..", "b"});
    TestSplitPath("a..\\b", true, {"a..", "b"});
    TestSplitPath("a../b", false, {"a..", "b"});
    TestSplitPath("a..\\b", false, {"a..", "b"});

    TestSplitPath("a/..b", true, {"a", "..b"});
    TestSplitPath("a\\..b", true, {"a", "..b"});
    TestSplitPath("a/..b", false, {"a", "..b"});
    TestSplitPath("a\\..b", false, {"a", "..b"});

    TestSplitPath("a..b/c", true, {"a..b", "c"});
    TestSplitPath("a..b\\c", true, {"a..b", "c"});
    TestSplitPath("a..b/c", false, {"a..b", "c"});
    TestSplitPath("a..b\\c", false, {"a..b", "c"});

    TestSplitPath("../..", true, {"..", ".."});
    TestSplitPath("..\\..", true, {"..", ".."});
    TestSplitPath("../..", false, {"..", ".."});
    TestSplitPath("..\\..", false, {"..", ".."});
}


TEST(Platforms_FileSystem, SimplifyPath)
{
    EXPECT_STREQ(FileSystem::SimplifyPath("", '/').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("", '\\').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("a", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("/", '/').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("/", '\\').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\", '/').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\", '\\').c_str(), "");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/", '\\').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("/a", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("/a", '\\').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\a", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\a", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("/a/", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("/a/", '\\').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\a/", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("\\a/", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/b", '\\').c_str(), "a\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\b", '\\').c_str(), "a\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a//b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\\\b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a//b", '\\').c_str(), "a\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\\\b", '\\').c_str(), "a\\b");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/./b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.\\b", '/').c_str(), "a/b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/./b", '\\').c_str(), "a\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.\\b", '\\').c_str(), "a\\b");

    EXPECT_STREQ(FileSystem::SimplifyPath("./a", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath(".\\a", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("./a", '\\').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath(".\\a", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/.", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.", '/').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/.", '\\').c_str(), "a");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.", '\\').c_str(), "a");

    EXPECT_STREQ(FileSystem::SimplifyPath("a./b", '/').c_str(), "a./b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a.\\b", '/').c_str(), "a./b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a./b", '\\').c_str(), "a.\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a.\\b", '\\').c_str(), "a.\\b");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/.b", '/').c_str(), "a/.b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.b", '/').c_str(), "a/.b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/.b", '\\').c_str(), "a\\.b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\.b", '\\').c_str(), "a\\.b");

    EXPECT_STREQ(FileSystem::SimplifyPath("a.b/c", '/').c_str(), "a.b/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a.b\\c", '/').c_str(), "a.b/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a.b/c", '\\').c_str(), "a.b\\c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a.b\\c", '\\').c_str(), "a.b\\c");

    EXPECT_STREQ(FileSystem::SimplifyPath("..", '/').c_str(), "..");
    EXPECT_STREQ(FileSystem::SimplifyPath("..", '\\').c_str(), "..");
    EXPECT_STREQ(FileSystem::SimplifyPath("../a", '/').c_str(), "../a");
    EXPECT_STREQ(FileSystem::SimplifyPath("../a", '\\').c_str(), "..\\a");
    EXPECT_STREQ(FileSystem::SimplifyPath("..\\a", '/').c_str(), "../a");
    EXPECT_STREQ(FileSystem::SimplifyPath("..\\a", '\\').c_str(), "..\\a");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/..", '/').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/..", '\\').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\..", '/').c_str(), "");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\..", '\\').c_str(), "");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/b/../c", '/').c_str(), "a/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/b/../c", '\\').c_str(), "a\\c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\b\\..\\c", '/').c_str(), "a/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\b\\..\\c", '\\').c_str(), "a\\c");

    EXPECT_STREQ(FileSystem::SimplifyPath("a../b", '/').c_str(), "a../b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a..\\b", '/').c_str(), "a../b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a../b", '\\').c_str(), "a..\\b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a..\\b", '\\').c_str(), "a..\\b");

    EXPECT_STREQ(FileSystem::SimplifyPath("a/..b", '/').c_str(), "a/..b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\..b", '/').c_str(), "a/..b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a/..b", '\\').c_str(), "a\\..b");
    EXPECT_STREQ(FileSystem::SimplifyPath("a\\..b", '\\').c_str(), "a\\..b");

    EXPECT_STREQ(FileSystem::SimplifyPath("a..b/c", '/').c_str(), "a..b/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a..b\\c", '/').c_str(), "a..b/c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a..b/c", '\\').c_str(), "a..b\\c");
    EXPECT_STREQ(FileSystem::SimplifyPath("a..b\\c", '\\').c_str(), "a..b\\c");

    EXPECT_STREQ(FileSystem::SimplifyPath("../..", '/').c_str(), "../..");
    EXPECT_STREQ(FileSystem::SimplifyPath("..\\..", '/').c_str(), "../..");
    EXPECT_STREQ(FileSystem::SimplifyPath("../..", '\\').c_str(), "..\\..");
    EXPECT_STREQ(FileSystem::SimplifyPath("..\\..", '\\').c_str(), "..\\..");
}

TEST(Platforms_FileSystem, SplitPathList)
{
    auto TestPaths = [](const char* PathList, std::vector<std::string> Expected) {
        std::vector<std::string> Paths;
        FileSystem::SplitPathList(PathList,
                                  [&Paths](const char* Path, size_t Len) //
                                  {
                                      Paths.emplace_back(std::string{Path, Len});
                                      return true;
                                  });
        EXPECT_EQ(Paths.size(), Expected.size());
        if (Paths.size() != Expected.size())
            return;

        for (size_t i = 0; i < Expected.size(); ++i)
        {
            EXPECT_EQ(Paths[i], Expected[i]);
        }
    };
    TestPaths("", {});
    TestPaths(";", {});
    TestPaths(";;", {});
    TestPaths("path", {{"path"}});
    TestPaths(";path", {{"path"}});
    TestPaths("path;", {{"path"}});
    TestPaths("path;;", {{"path"}});
    TestPaths(";;path;;", {{"path"}});
    TestPaths("path1;path2", {{"path1"}, {"path2"}});
    TestPaths("path1;;path2", {{"path1"}, {"path2"}});
    TestPaths("path1;;path2;", {{"path1"}, {"path2"}});
    TestPaths("path1;;path2;", {{"path1"}, {"path2"}});
    TestPaths(";;path1;;path2;", {{"path1"}, {"path2"}});
    TestPaths("c:\\windows\\path1;c:\\windows\\path2", {{"c:\\windows\\path1"}, {"c:\\windows\\path2"}});
    TestPaths("/unix/path1;/unix/path2", {{"/unix/path1"}, {"/unix/path2"}});
}

TEST(Platforms_FileSystem, GetPathComponents)
{
    auto TestComponents = [](const char* Path, const char* RefDir, const char* RefFileName) {
        std::string Dir;
        std::string FileName;
        FileSystem::GetPathComponents(Path, &Dir, nullptr);
        EXPECT_STREQ(Dir.c_str(), RefDir);

        FileSystem::GetPathComponents(Path, nullptr, &FileName);
        EXPECT_STREQ(FileName.c_str(), RefFileName);

        FileSystem::GetPathComponents(Path, &Dir, &FileName);
        EXPECT_STREQ(Dir.c_str(), RefDir);
        EXPECT_STREQ(FileName.c_str(), RefFileName);
    };
    TestComponents("", "", "");
    TestComponents("file", "", "file");
    TestComponents("/file", "", "file");
    TestComponents("\\file", "", "file");
    TestComponents("path/file", "path", "file");
    TestComponents("path\\file", "path", "file");
    TestComponents("/path/file", "/path", "file");
    TestComponents("\\path\\file", "\\path", "file");
    TestComponents("/a/b/c/file", "/a/b/c", "file");
    TestComponents("\\a\\b\\c\\file", "\\a\\b\\c", "file");
}


TEST(Platforms_FileSystem, GetRelativePath)
{
    EXPECT_STREQ(FileSystem::GetRelativePath("", true, "", true).c_str(), "");
    EXPECT_STREQ(FileSystem::GetRelativePath("", false, "", false).c_str(), "");

    EXPECT_STREQ(FileSystem::GetRelativePath("", true, "to/dir", true).c_str(), "");
    EXPECT_STREQ(FileSystem::GetRelativePath("", false, "to/file", false).c_str(), "");
    EXPECT_STREQ(FileSystem::GetRelativePath("from/dir", true, "", true).c_str(), "from/dir");
    EXPECT_STREQ(FileSystem::GetRelativePath("from/file", false, "", false).c_str(), "from/file");

    EXPECT_STREQ(FileSystem::GetRelativePath("a/b/c", true, "a/b/c", true).c_str(), "");
    EXPECT_STREQ(FileSystem::GetRelativePath("a/b/c/file", false, "a/b/c/file", false).c_str(), "");

    auto BuildPath = [](std::vector<std::string> Components) {
        auto it   = Components.begin();
        auto Path = *(it++);
        for (; it != Components.end(); ++it)
        {
            Path.push_back(FileSystem::SlashSymbol);
            Path.append(*it);
        }
        return Path;
    };

    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from", true, "a/b/c", true).c_str(), BuildPath({".."}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir", true, "a/b/c", true).c_str(), BuildPath({"..", ".."}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/file", false, "a/b/c", true).c_str(), BuildPath({".."}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir/file", false, "a/b/c", true).c_str(), BuildPath({"..", ".."}));

    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from", true, "a/b/c/file", false).c_str(), BuildPath({"..", "file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir", true, "a/b/c/file", false).c_str(), BuildPath({"..", "..", "file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/file", false, "a/b/c/file", false).c_str(), BuildPath({"..", "file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir/file", false, "a/b/c/file", false).c_str(), BuildPath({"..", "..", "file"}));

    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c", true, "a/b/c/to", true).c_str(), BuildPath({"to"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c", true, "a/b/c/to/dir", true).c_str(), BuildPath({"to", "dir"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/file", false, "a/b/c/to", true).c_str(), BuildPath({"to"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/file", false, "a/b/c/to/dir", true).c_str(), BuildPath({"to", "dir"}));

    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c", true, "a/b/c/file", false).c_str(), BuildPath({"file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c", true, "a/b/c/to/file", false).c_str(), BuildPath({"to", "file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/file", false, "a/b/c/file2", false).c_str(), BuildPath({"file2"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/file", false, "a/b/c/to/file", false).c_str(), BuildPath({"to", "file"}));

    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir", true, "a/b/c/to/dir", true).c_str(), BuildPath({"..", "..", "to", "dir"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/file", false, "a/b/c/to/dir", true).c_str(), BuildPath({"..", "to", "dir"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/dir", true, "a/b/c/to/file", false).c_str(), BuildPath({"..", "..", "to", "file"}));
    EXPECT_EQ(FileSystem::GetRelativePath("a/b/c/from/file", false, "a/b/c/to/file", false).c_str(), BuildPath({"..", "to", "file"}));
}

} // namespace
