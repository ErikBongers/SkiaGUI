#include "../pch.h"
#include "Zip.h"

class Resources
    {
    private:
        std::string dirName = "";
        std::map<std::string, sk_sp<SkImage>> resources;
    public:
        Resources(std::string archiveName, std::string dirName = "");
        sk_sp<SkImage> get(std::string resourceName);
    };

