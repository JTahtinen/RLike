#include "assetcollection.h"
#include "imageload.h"

template <typename T>
T *getElement(const char *name, jadel::Vector<T> &elements, jadel::Vector<jadel::String> &names)
{
    for (size_t i = 0; i < names.size; ++i)
    {
        if (strncmp(names[i].c_str(), name, names[i].size) == 0)
        {
            return &elements[i];
        }
    }
    return NULL;
}

jadel::Surface *AssetCollection::getSurface(const char *name)
{
    jadel::Surface* result = getElement<jadel::Surface>(name, this->surfaces, this->surfaceNames);
    if (!result)
    {
        return getSurface("res/missing.png");
    }
    return result;
}

void AssetCollection::pushSurface(jadel::Surface surface, const char *name)
{
    this->surfaces.push(surface);
    this->surfaceNames.push(name);
}

bool AssetCollection::loadSurface(const char *filepath)
{
    jadel::Surface target;
    if (!load_PNG(filepath, &target))
        return false;
    pushSurface(target, filepath);
    return true;
}

Font *AssetCollection::getFont(const char *name)
{
    Font* result = getElement<Font>(name, this->fonts, this->fontNames);
    if (!result)
    {
        return NULL;
    }
    return result;
}

void AssetCollection::pushFont(const Font &font, const char *name)
{
    this->fonts.push(font);
    this->fontNames.push(name);
}

bool AssetCollection::loadFont(const char *filepath)
{
    Font target;
    if (!target.loadFont(filepath))
    {
        return false;
    }
    pushFont(target, filepath);
    return true;
}