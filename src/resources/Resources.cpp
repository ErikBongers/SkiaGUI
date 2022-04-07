#include "../pch.h"
#include "Resources.h"

/**

*/
Resources::Resources(std::string archiveName, std::string dirName)
    :dirName(dirName)
    {
    Zip zip(archiveName.c_str());
    for (int i = 0; i < zip.getFileCount(); i++)
        {
        FileBuffer fb = zip.getFile(i);
        sk_sp<SkData> data = SkData::MakeFromMalloc(fb.p, fb.length);
        sk_sp<SkImage> img = SkImage::MakeFromEncoded(data);
        resources.emplace(fb.fileName, img);
        }
    }

sk_sp<SkImage> Resources::get(std::string resourceName)
    {
    if (dirName != "")
        {
        std::string path = dirName + "\\" + resourceName;
        std::ifstream ifile(path);
        if (ifile) {
            sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
            return SkImage::MakeFromEncoded(data);
            }

        }
    return resources[resourceName];
    }
