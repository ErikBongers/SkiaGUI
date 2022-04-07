#include "Zip.h"

Zip::Zip(const char* archiveName)
    {
    memset(&zip_archive, 0, sizeof(zip_archive));

    mz_bool status = mz_zip_reader_init_file(&zip_archive, archiveName, 0);
    if (!status)
        {
        errorMessage = "mz_zip_reader_init_file() failed!\n";
        return;
        }
    }

int Zip::getFileCount()
    {
    return (int)mz_zip_reader_get_num_files(&zip_archive);
    }

FileBuffer Zip::getFile(int i)
    {
    FileBuffer buffer;
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
        errorMessage = "mz_zip_reader_file_stat() failed!\n";
        mz_zip_reader_end(&zip_archive);
        return buffer;
        }

    //printf("Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u, Is Dir: %u\n", file_stat.m_filename, file_stat.m_comment, (unsigned int)file_stat.m_uncomp_size, (unsigned int)file_stat.m_comp_size, mz_zip_reader_is_file_a_directory(&zip_archive, i));

    if (!mz_zip_reader_is_file_a_directory(&zip_archive, i))
        {
        buffer = getFile(file_stat.m_filename);
        }
    return buffer;
    }

FileBuffer Zip::getFile(const char* fileName)
    {
    FileBuffer buffer;
    buffer.p = mz_zip_reader_extract_file_to_heap(&zip_archive, fileName, &buffer.length, 0);
    if (!buffer.p)
        {
        errorMessage = "mz_zip_reader_extract_file_to_heap() failed!\n";
        mz_zip_reader_end(&zip_archive);
        }
    buffer.fileName = fileName;
    return buffer;
    }

void Zip::close()
    {
    mz_zip_reader_end(&zip_archive);
    }
