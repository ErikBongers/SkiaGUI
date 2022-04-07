#pragma once
#include "../pch.h"
#include "miniz.h"

struct FileBuffer
    {
    void* p = nullptr;
    size_t length = -1;
    std::string fileName = "";
    };

class Zip
    {
    private:
        mz_zip_archive zip_archive;
    public:
        std::string errorMessage = "";
        Zip(const char* archiveName);
        int getFileCount();
        FileBuffer getFile(int i);
        FileBuffer getFile(const char* fileName);
        void close();

    };