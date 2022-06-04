#pragma once
#include<windows.h>
#include<unordered_map>
#include "internal/DDSStruct.h"


namespace dds_format {
    class FileCache {
    public:
        /// <summary>
        /// キャッシュから読み取る
        /// </summary>
        /// <param name="filename">キャッシュのキー</param>
        /// <param name="outChunk">キャッシュの値</param>
        /// <returns><para>true キャッシュから読み取った(outChunkに値がコピーされる)</para>
        ///          <para>false キャッシュから読み取れない</para></returns>
        bool Fetch(const WCHAR* filename, dds_loader::Chunk& outChunk) const;
        /// <summary>
        /// キャッシュへ格納する
        /// </summary>
        /// <param name="filename">キャッシュのキー</param>
        /// <param name="Chunk">キャッシュの値</param>
        void Store(const WCHAR* filename, const dds_loader::Chunk& inChunk);
    private:
        std::unordered_map<std::wstring,dds_loader::Chunk> m_filenameToChunk;
    };

    class ColumnCache {
    public:
        bool Fetch(const WCHAR* filename, DWORD pid, VARIANT* outData);
        void Store(const WCHAR* filename, DWORD pid, const VARIANT* inData);
    private:
        struct Key{
            DWORD           m_pid;
            std::wstring    m_filename;

            bool    operator==(const Key& rhs) const;
            bool    operator!=(const Key& rhs) const;
        };
        struct CalcHash {
            size_t  operator()(const Key& key) const;
        };
        std::unordered_map<Key,CComVariant,CalcHash> m_filenameToColumn;
    };
};

