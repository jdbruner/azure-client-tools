// PluginCreator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>

#include "..\..\Utilities\Utils.h"

#include <experimental/filesystem>

#define TOKEN "Skeleton"

using namespace std;
using namespace experimental::filesystem;
using namespace experimental::filesystem::v1;

using namespace Microsoft::Azure::DeviceManagement::Utils;

void ReadFile(
    const string& fileName,
    vector<char>& buffer)
{
    ifstream file(fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open())
    {
        ostringstream s;
        s << "Failed to open file: " << fileName << endl;
        throw exception(s.str().c_str());
    }
    unsigned int fileSize = static_cast<unsigned int>(file.tellg());
    buffer.resize(fileSize);
    file.seekg(0, ios::beg);
    file.read(buffer.data(), fileSize);
    file.close();
}

void WriteFile(const string& fileName, const vector<char>& buffer)
{
    ofstream file(fileName, ios::out | ios::binary);
    if (!file.is_open())
    {
        ostringstream s;
        s << "Failed to open file: " << fileName << endl;
        throw exception(s.str().c_str());
    }
    file.write(buffer.data(), buffer.size());
    file.close();
}

string Replace(
    const string source,
    string token,
    string replacement)
{
    string resolved = source;

    size_t pos = 0;

    while ((pos = resolved.find(token, pos)) != string::npos)
    {
        resolved.replace(pos, token.size(), replacement);
        pos += replacement.size();
    }

    return resolved;
}

void Create(
    const string& skeletonFile,
    const string& pluginName,
    const string& targetFile)
{
    cout << "Source: " << skeletonFile << endl;
    cout << "Target: " << targetFile << endl;

    vector<char> sourceContentBuffer;
    ReadFile(skeletonFile, sourceContentBuffer);

    string sourceContentString(sourceContentBuffer.data(), sourceContentBuffer.size());

    string resolvedContentString = Replace(sourceContentString, TOKEN, pluginName);

    vector<char> resolvedContentBuffer(resolvedContentString.size());
    memcpy(resolvedContentBuffer.data(), resolvedContentString.c_str(), resolvedContentString.size());

    WriteFile(targetFile, resolvedContentBuffer);
}

void ProcessFolder(
    const string& skeletonFolder,
    const string& pluginName,
    const string& targetFolder)
{
    cout << "Source Folder:" << skeletonFolder << endl;
    cout << "Target Folder:" << targetFolder << endl;

    for (auto & pathIt : directory_iterator(skeletonFolder))
    {
        string fileSystemObjectName = WideToMultibyte(pathIt.path().c_str());
        string newFileSystemObjectName = targetFolder + "\\" + WideToMultibyte(pathIt.path().filename().c_str());
        string resolvedFileSystemObjectName = Replace(newFileSystemObjectName, TOKEN, pluginName);

        if (pathIt.status().type() == file_type::directory)
        {
            CreateDirectory(MultibyteToWide(newFileSystemObjectName.c_str()).c_str(), NULL);

            ProcessFolder(fileSystemObjectName, pluginName, newFileSystemObjectName);
            continue;
        }

        if (pathIt.status().type() != file_type::regular)
        {
            continue;
        }

        Create(fileSystemObjectName, pluginName, resolvedFileSystemObjectName);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << endl;
        cout << "Too few parameters." << endl;
        cout << endl;
        cout << "PluginCreator.exe skeletonFolder pluginName targetFolderParent" << endl;
        cout << endl;
        return 0;
    }

    try
    {
        string skeletonFolder = argv[1];
        string pluginName = argv[2];
        string targetFolderParent = argv[3];
        string targetFolder = targetFolderParent + "\\" + pluginName + "Plugin";

        CreateDirectory(MultibyteToWide(targetFolder.c_str()).c_str(), NULL);

        ProcessFolder(skeletonFolder, pluginName, targetFolder);
    }
    catch (const exception& e)
    {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}

