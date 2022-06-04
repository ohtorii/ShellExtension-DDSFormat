#pragma once
#include<windows.h>
#include<unordered_map>
#include "internal/DDSStruct.h"


namespace dds_format {
    class FileCache {
    public:
        /// <summary>
        /// �L���b�V������ǂݎ��
        /// </summary>
        /// <param name="filename">�L���b�V���̃L�[</param>
        /// <param name="outChunk">�L���b�V���̒l</param>
        /// <returns><para>true �L���b�V������ǂݎ����(outChunk�ɒl���R�s�[�����)</para>
        ///          <para>false �L���b�V������ǂݎ��Ȃ�</para></returns>
        bool Fetch(const WCHAR* filename, dds_loader::Chunk& outChunk) const;
        /// <summary>
        /// �L���b�V���֊i�[����
        /// </summary>
        /// <param name="filename">�L���b�V���̃L�[</param>
        /// <param name="Chunk">�L���b�V���̒l</param>
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

