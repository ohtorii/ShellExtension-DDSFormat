#include "pch.h"
#include"DDSFormatColExtCache.h"
#include "Logger.h"

namespace {
    //キャッシュの最大サイズ
    //キャッシュの使用メモリ量が肥大化する（メモリリークと間違われる恐れあり）のを抑えるために必要
    const size_t sg_maxFileChacheSize   = 10000;
    const size_t sg_maxColumnChacheSize = 10000;
};

namespace dds_format {
    /////////////////////////////////////////////////////////////////////////////////
    //  FileCache
    /////////////////////////////////////////////////////////////////////////////////
    bool FileCache::Fetch(const WCHAR* filename, dds_loader::Chunk& outChunk) const {
        Logger::Write(L"[File:Fetch]%s",filename);
        auto it = m_filenameToChunk.find(filename);
        if (it == m_filenameToChunk.end()) {
            Logger::WriteLine(L"  Miss");
            return false;
        }
        outChunk=it->second;
        Logger::WriteLine(L"  Hit");
        return true;
    }
    void FileCache::Store(const WCHAR* filename, const dds_loader::Chunk& chunk) {
        Logger::Write(L"[File:Store]%s",filename);
        auto pair = m_filenameToChunk.insert_or_assign(filename, chunk);
        if (pair.second) {
            //Insert
            if (sg_maxColumnChacheSize < m_filenameToChunk.size()) {
                //Memo: 先頭要素を削除して御茶を濁す
                //      本来ならばLeast Recently Used (LRU)で管理すべき
                m_filenameToChunk.erase(m_filenameToChunk.begin());
            }
            Logger::WriteLine(L"  Success");
        }
        else {
            Logger::WriteLine(L"  Failed");
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    //  ColumnCache
    /////////////////////////////////////////////////////////////////////////////////
    bool ColumnCache::Fetch(const WCHAR* filename, DWORD pid, VARIANT* outData) {
        Logger::WriteLine(L"[File:Fetch]%s (pid=%d)",filename,pid);
        auto it = m_filenameToColumn.find(ColumnCache::Key{.m_pid=pid,.m_filename=filename});
        if (it == m_filenameToColumn.end()) {
            Logger::WriteLine(L"  Miss");
            return false;
        }
        HRESULT hr = ::VariantCopy(outData, static_cast<VARIANT*>(&(it->second)));
        if (FAILED(hr)) {
            Logger::WriteLine(L" Miss");
            return false;
        }
        Logger::WriteLine(L"  Hit");
        return true;
    }
    void ColumnCache::Store(const WCHAR* filename, DWORD pid, const VARIANT* inData) {
        Logger::Write(L"[Column:Store]%s (pid=%d)",filename,pid);
        auto pair = m_filenameToColumn.insert_or_assign(ColumnCache::Key{.m_pid=pid,.m_filename=filename}, *inData);
        if (pair.second) {
            //Insert
            if (sg_maxFileChacheSize < m_filenameToColumn.size()) {
                //Memo: 先頭要素を削除して御茶を濁す
                //      本来ならばLeast Recently Used (LRU)で管理すべき
                m_filenameToColumn.erase(m_filenameToColumn.begin());
            }
            Logger::WriteLine(L"  Success");
        }
        else {
            Logger::WriteLine(L"  Failed");
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    //  ColumnCache::Key
    /////////////////////////////////////////////////////////////////////////////////
    bool ColumnCache::Key::operator==(const Key& rhs) const {
        return (m_pid == rhs.m_pid) && (m_filename == rhs.m_filename);
    }
    bool ColumnCache::Key::operator!=(const Key& rhs) const {
        return !(this->operator==(rhs));
    }

    /////////////////////////////////////////////////////////////////////////////////
    //  ColumnCache::CalcHash
    /////////////////////////////////////////////////////////////////////////////////
    size_t  ColumnCache::CalcHash::operator()(const Key& key) const {
        //Memo: 二つのハッシュ値を合体する（とりあえずxorでお茶を濁しています）
        return std::hash<std::wstring>()(key.m_filename) ^ key.m_pid;
    }
};
