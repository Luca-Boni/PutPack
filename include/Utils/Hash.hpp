#pragma once

#include <unordered_map>

struct ObjHasher
{
    template <typename keyType, typename valueType>
    std::size_t operator()(const keyType &key) const
    {
        return std::hash<keyType>((unsigned long long)&key);
    }
};

template <typename keyType, typename valueType>
class ObjHash
{
private:
    std::unordered_map<keyType, valueType, ObjHasher> hash;
public:
    ObjHash(){};
    ~ObjHash(){};
    void insert(keyType key, valueType value)
    {
        hash.insert({key, value});
    }
    valueType get(keyType key)
    {
        return hash[key];
    }
    void remove(keyType key)
    {
        hash.erase(key);
    }
    bool contains(keyType key)
    {
        return hash.find(key) != hash.end();
    }
    void clear()
    {
        hash.clear();
    }
    valueType operator[](keyType key)
    {
        return hash[key];
    }
};