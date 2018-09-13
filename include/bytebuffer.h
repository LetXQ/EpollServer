#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H
#include <list>
#include <map>
#include <set>
#include <vector>
#include <assert.h>
#include <cstdint>
#include <string>
#include <cstring>
#include <stdio.h>
#include <algorithm>

using uint8_vec_t = std::vector<uint8_t>;
constexpr size_t DEFAULT_SIZE = 0x100; // 256byte

class ByteBuffer
{
protected:
    size_t _rpos = 0;
    size_t _wpos = 0;
    uint8_vec_t _storage;

public:
    class error{};

    ByteBuffer()
    {
        _storage.reserve(DEFAULT_SIZE);
    }

    ByteBuffer(size_t res)
    {
        _storage.reserve(res);
    }

    ByteBuffer(const ByteBuffer& buf)
        : _rpos(buf._rpos)
        , _wpos(buf._wpos)
        , _storage(buf._storage)
    {}

    virtual ~ByteBuffer()
    {}

    void clear()
    {
        _storage.clear();
        _rpos = 0;
        _wpos = 0;
    }

    template<typename T> void append(T value)
    {
        append((uint8_t*)&value, sizeof(value));
    }

    template<typename T> void put(size_t pos, T value)
    {
        put(pos, (uint8_t*)&value, sizeof(value));
    }

    void append(const std::string& str)
    {
        append((uint8_t*)str.c_str(), str.size() + 1);
    }

    void append(const char* src, size_t cnt)
    {
        append((const uint8_t*)src, cnt);
    }

    void append(const uint8_t* src, size_t cnt)
    {
        if (!cnt)
            return;
        assert(size() < 1000000);

        if (_storage.size() < _wpos + cnt)
        {
            _storage.resize(_wpos + cnt);
        }
        memcpy(&_storage[_wpos], src, cnt);
        _wpos += cnt;
    }

    void append(const ByteBuffer& buffer)
    {
        if (buffer.size() > 0)
            append(buffer.contents(), buffer.size());
    }

    void resize(size_t new_size)
    {
        _storage.resize(new_size);
        _rpos = 0;
        _wpos = size();
    }

    void reserve(size_t resize)
    {
        if (resize > size())
            _storage.reserve(resize);
    }

    void put(size_t pos, const uint8_t* src, size_t cnt)
    {
        if (pos + cnt <= size())
            memcpy(&_storage[pos], src, cnt);
    }

    inline size_t size() const
    {
        return _storage.size();
    }

    const char* contents() const
    {
        if(_storage.empty())
            return nullptr;
        return (char*)&_storage[0];
    }

    const char* remain_contents() const
    {
        if(_storage.empty())
            return nullptr;

        return (char*)&_storage[_rpos];
    }

    template<typename T> T read()
    {
        T r = read<T>(_rpos);
        _rpos += sizeof(T);
        return r;
    }

    template<typename T> T read(size_t pos) const
    {
        if (pos + sizeof(T) > size())
            return (T)0;
        else
            return *((T*)&_storage[pos]);
    }

    void read(uint8_t* dest, size_t len)
    {
        if (_rpos + len <= size())
        {
            memcpy(dest, & _storage[_rpos], len);
            _rpos += len;
        }
        else
        {
            memset(dest, 0, len);
        }
    }

    size_t wpos(size_t wpos)
    {
        _wpos = wpos;
        return _wpos;
    }

    size_t wpos() const
    {
        return _wpos;
    }

    size_t rpos(size_t rpos)
    {
        _rpos = rpos;
        return _rpos;
    }

    size_t rpos() const
    {
        return _rpos;
    }

    int32_t remain_size() const
    {
        return _wpos - _rpos -1;
    }

    inline void reverse()
    {
        std::reverse(_storage.begin(), _storage.end());
    }


    ByteBuffer &operator << (bool value)
    {
        append<char>((char)value);
        return *this;
    }
    ByteBuffer &operator << (uint8_t value)
    {
        append<uint8_t>(value);
        return *this;
    }
    ByteBuffer &operator << (uint16_t value)
    {
        append<uint16_t>(value);
        return *this;
    }
    ByteBuffer &operator << (uint32_t value)
    {
        append<uint32_t>(value);
        return *this;
    }
    ByteBuffer &operator << (uint64_t value)
    {
        append<uint64_t>(value);
        return *this;
    }
    ByteBuffer &operator << (int8_t value)
    {
        append<int8_t>(value);
        return *this;
    }
    ByteBuffer &operator << (int16_t value)
    {
        append<int16_t>(value);
        return *this;
    }
    ByteBuffer &operator << (int32_t value)
    {
        append<int32_t>(value);
        return *this;
    }
    ByteBuffer &operator << (int64_t value)
    {
        append<int64_t>(value);
        return *this;
    }
    ByteBuffer &operator << (float value)
    {
        append<float>(value);
        return *this;
    }
    ByteBuffer &operator << (double value)
    {
        append<double>(value);
        return *this;
    }

    ByteBuffer &operator >> (bool& value)
    {
        value = read<char>() > 0 ? true : false;
        return *this;
    }
    ByteBuffer &operator >> (uint8_t& value)
    {
        value = read<uint8_t>();
        return *this;
    }
    ByteBuffer &operator >> (uint16_t& value)
    {
        value = read<uint16_t>();
        return *this;
    }
    ByteBuffer &operator >> (uint32_t& value)
    {
        value = read<uint32_t>();
        return *this;
    }
    ByteBuffer &operator >> (uint64_t& value)
    {
        value = read<uint64_t>();
        return *this;
    }
    ByteBuffer &operator >> (int8_t& value)
    {
        value = read<int8_t>();
        return *this;
    }
    ByteBuffer &operator >> (int16_t& value)
    {
        value = read<int16_t>();
        return *this;
    }
    ByteBuffer &operator >> (int32_t& value)
    {
        value = read<int32_t>();
        return *this;
    }
    ByteBuffer &operator >> (int64_t& value)
    {
        value = read<int64_t>();
        return *this;
    }
    ByteBuffer &operator >> (float value)
    {
        value = read<float>();
        return *this;
    }
    ByteBuffer &operator >> (double value)
    {
        value = read<double>();
        return *this;
    }

    ByteBuffer& operator <<(const std::string& value)
    {
        append((uint32_t)value.length());
        append((uint8_t*)value.c_str(), value.length());
        append((uint8_t)0);
    }

    ByteBuffer& operator <<(const char* src)
    {
        append((uint8_t*)src, strlen(src));
        append((uint8_t)0);
    }

    ByteBuffer& operator >>(std::string& value)
    {
        uint32_t len = read<uint32_t>();
        if (len > _storage.size() - _rpos)
        {
            // error
            return *this;
        }
        value.reserve(len);
        value.append((char*)(&_storage[_rpos]), len);
        _rpos += (len + 1);
        return *this;
    }

    uint8_t operator [](size_t pos)
    {
        return read<uint8_t>(pos);
    }

};


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

template<typename T> ByteBuffer& operator << (ByteBuffer& b, std::vector<T> o)
{
    b << (uint32_t)o.size();
    for (auto& elem : o)
    {
        b << elem;
    }
    return b;
}

template<typename T> ByteBuffer& operator >> (ByteBuffer& b, std::vector<T>& o)
{
    uint32_t size = 0;
    b >> size;
    o.clear();
    while (size--)
    {
        T t;
        b >> t;
        o.push_back(t);
    }
    return b;
}

template<typename T> ByteBuffer& operator << (ByteBuffer& b, std::set<T> o)
{
    b << (uint32_t)o.size();
    for (auto& elem : o)
    {
        b << elem;
    }
    return b;
}

template<typename T> ByteBuffer& operator >> (ByteBuffer& b, std::set<T>& o)
{
    uint32_t size = 0;
    b >> size;
    o.clear();
    while (size--)
    {
        T t;
        b >> t;
        o.insert(t);
    }
    return b;
}

template<typename T> ByteBuffer& operator << (ByteBuffer& b, std::list<T> o)
{
    b << (uint32_t)o.size();
    for (auto& elem : o)
    {
        b << elem;
    }
    return b;
}

template<typename T> ByteBuffer& operator >> (ByteBuffer& b, std::list<T>& o)
{
    uint32_t size = 0;
    b >> size;
    o.clear();
    while (size--)
    {
        T t;
        b >> t;
        o.push_back(t);
    }
    return b;
}

template<typename K, typename V> ByteBuffer& operator << (ByteBuffer& b, std::map<K, V> o)
{
    b << (uint32_t)o.size();
    for (auto& elem : o)
    {
        b << elem.first << elem.second;
    }
    return b;
}

template<typename K, typename V> ByteBuffer& operator >> (ByteBuffer& b, std::map<K, V>& o)
{
    uint32_t size = 0;
    b >> size;
    o.clear();
    while (size--)
    {
        K k;
        V v;
        b >> k >> v;
        o.insert(std::make_pair(k, v));
    }
    return b;
}
#endif // BYTEBUFFER_H
