#include "BearCore.hpp"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
BEARTOOL_API  BearCore::BearFileSystem*BearCore::FS = 0;
void BearCore::BearFileSystem::decoder(BearINI & ini)
{
	auto filesystem=ini.sections.find(BearStringConteniar(TEXT("filesystem"), false));
	BEAR_FATALERROR(filesystem != ini.sections.end(), TEXT("не найдена секция filesystem"));
	{
		auto begin = filesystem->second.keys.begin();
		auto end = filesystem->second.keys.end();
		while (begin != end)
		{
			SourcePath path;
			decoderPath(path, begin->second[0]);
			BearVector< SourcePath> vpath;
			vpath.push_back(path);
			m_source_paths.insert(begin->first, vpath);
			begin++;
		}
	}
	{
		auto begin = ini.sections.begin();
		auto end = ini.sections.end();
		while (begin != end)
		{
			if (begin == filesystem) { begin++; continue; }
			auto begink = begin->second.keys.begin();
			auto endk = begin->second.keys.end();
			BEAR_FATALERROR(m_source_paths.find(begin->first) == m_source_paths.end(), TEXT("Путь [%s]  уже существует"), *begin->first);
			m_source_paths.insert(begin->first, BearVector<SourcePath>());
			auto item=m_source_paths.find(begin->first);
			while (begink != endk)
			{
				SourcePath path;
				BearString var1(*begink->first);
				decoderPath(path,var1);
				begink->second[0].scanf(TEXT("%d"), &path.priority);
				item->second.insert(bear_lower_bound(item->second.begin(), item->second.end(), path), path);
				begink++;
			}
			begin++;
		}
	}
}

void BearCore::BearFileSystem::update(const bchar * name)
{
	BEAR_FATALERROR(name[0], TEXT("Пустой путь"));
	if (m_paths.find(BearStringConteniar(name, false)) == m_paths.end())
	{
		if (BearString::Compare(name, TEXT("%fs_work%")) == 0)
		{
			BearStringPath path;
			BearFileManager::GetWorkPath(path);
			BearString::ToLower(path);
			BearVector<BearString> paths;
			paths.push_back(path);
			m_paths.insert(name, paths);
		}
		else if (BearString::Compare(name, TEXT("%fs_api%")) == 0)
		{
			BearStringPath path;
			BearFileManager::GetApplicationPath(path);
			BearString::ToLower(path);
			BearVector<BearString> paths;
			paths.push_back(path);
			m_paths.insert(name, paths);
		}
		else
		{
			BearVector<BearString> paths;
			auto item = m_source_paths.find(BearStringConteniar(name, false));
			BEAR_FATALERROR(item != m_source_paths.end(), TEXT("������ �� ������ ���� [%s]"), name);
			auto begin = item->second.begin();
			auto end = item->second.end();
			while (begin != end)
			{
				if (begin->parent[0])
				{
					update(begin->parent);
					auto begink = m_paths[begin->parent].begin();
					auto endk = m_paths[begin->parent].end();
					while (begink != endk)
					{
						BearString path = *begink;
						path.append(BEAR_PATH).append(begin->path);
						BearFileManager::PathOptimization(*path);
						paths.push_back(*path);
						begink++;
					}
				}
				else
				{
					paths.push_back(begin->path);
				}
				begin++;
			}
			m_paths.insert(name, paths);
		}
	}
}

void BearCore::BearFileSystem::decoderPath(SourcePath & path, BearString & string)
{
	string.sub_space_in_begin();
	BearStringPath temp;
	const bchar*temp2 = *string;
	temp2 = BearString::ReadTo(temp2, TEXT(','), temp);
	BearString::Copy(path.parent, temp);
	temp2 = BearString::ReadTo(temp2, TEXT(','), temp);
	path.path[0] = 0;
	do
	{
		if (path.path[0])BearString::Contact(path.path, BEAR_PATH);
		BearString::Contact(path.path, temp);
		temp2 = BearString::ReadTo(temp2, TEXT(','), temp);
	} while (temp[0] );
	if (!path.path[0])
	{
		BearString::Copy(path.path, path.parent);
		path.parent[0] = 0;
	}
	BearString::ToLower(path.path);
	BearFileManager::PathOptimization(path.path);
	path.priority = 0;
}
void BearCore::BearFileSystem::GetFilesFromPackage(BearVector<BearString>& files, const bchar * path, const bchar * e, bool subPath)
{
	auto begin = m_packages.begin();
	auto end = m_packages.end();
	while (begin != end)
	{
		BearStringPath fullpath;
		UpdatePath(*begin->GetFS(), 0, fullpath);
		BearString::Contact(fullpath, BEAR_PATH);
		if (BearString::Find(path, fullpath))
		{
			const bchar* file_path_in_bpk = path + BearString::GetSize(fullpath);
			BearStringPath E;
			BearString::Copy(E, file_path_in_bpk);
			BearFileManager::PathCombine(E, e);
			begin->GetFiles( files, E, subPath);
		}
		begin++;
	}
}
void BearCore::BearFileSystem::GetDirectoriesFromPackage(BearVector<BearString>& files, const bchar * path)
{
	auto begin = m_packages.begin();
	auto end = m_packages.end();
	while (begin != end)
	{
		BearStringPath fullpath;
		UpdatePath(*begin->GetFS(), 0, fullpath);
		BearString::Contact(fullpath, BEAR_PATH);
		if (BearString::Find(path, fullpath))
		{
			const bchar* file_path_in_bpk = path + BearString::GetSize(fullpath);
			begin->GetDirectories(files, file_path_in_bpk);
		}
		begin++;
	}
}
bool BearCore::BearFileSystem::RegisterFile(const bchar * full_path)
{
	if (m_files.find(BearStringConteniar( full_path,false)) != m_files.end())return  true;
	if (BearFileManager::FileExists(full_path))
	{
		m_files.insert(full_path);
		return true;
	}
	else
	{
		auto begin = m_packages.begin();
		auto end = m_packages.end();
		while (begin != end)
		{
			BearStringPath fullpath;
			UpdatePath(*begin->GetFS(), 0, fullpath);
			BearString::Contact(fullpath, BEAR_PATH);
			if (BearString::Find(full_path, fullpath))
			{
				const bchar* file_path_in_bpk = full_path + BearString::GetSize(fullpath);
				if (begin->ExistFile(file_path_in_bpk))
				{
					m_files.insert(full_path);
					auto&file_ = m_files[full_path];
					file_.package = &*begin;
					file_.package_path = file_path_in_bpk;
					return true;
				}
			}
			begin++;
		}
	}
	return  false;
}
void BearCore::BearFileSystem::RegisterFiles()
{
	if (m_files.size())return;
	BearVector<BearString> files;
	{
		auto begin = m_packages.begin();
		auto end = m_packages.end();
		while (begin != end)
		{
			begin->GetFiles(files);
			BearStringPath path;
			auto begin1 = files.begin();
			auto end1 = files.end();
			while (begin1 != end1)
			{
				Update(*begin->GetFS(), **begin1, path);
				begin1++;
			}
			files.clear_not_free();
			begin++;
		}
	}
		
}
bool BearCore::BearFileSystem::LoadFromFile(const bchar * file, BearEncoding::Encoding type)
{
	BearINI ini;
	if (!ini.LoadFromFile(file, type))
		return false;
	decoder(ini);
	return true;
}

void BearCore::BearFileSystem::LoadFromStream(const BearInputStream & file, BearEncoding::Encoding type)
{
	BearINI ini;
	ini.LoadFromStream(file, type);
	decoder(ini);
}

void BearCore::BearFileSystem::LoadFromBuffer(const BearBufferedReader & file, BearEncoding::Encoding type)
{
	BearINI ini;
	ini.LoadFromBuffer(file, type);
	decoder(ini);
}

void BearCore::BearFileSystem::SetPackage(const bchar * fspath, const bchar * path)
{
	m_packages.clear_not_free();
	m_files.clear_not_free();
	BearVector<BearString> bpks;
	{	
		BearStringPath fullpath;
		BearString::Copy(fullpath, path);
		BearFileManager::PathCombine(fullpath, TEXT("cooked"));
		AppendPath(TEXT("$temp"), fullpath, fspath, 0);
	}
	GetFiles(bpks, TEXT("$temp"), TEXT("*.bpk"));
	SubPath(TEXT("$temp"));
	auto begin = bpks.begin();
	auto end = bpks.end();
	while (begin != end)
	{
		BearStringPath fullpath;
		UpdatePath(fspath, 0, fullpath);
		BearFileManager::PathCombine(fullpath,path, TEXT("cooked"), **begin);
		m_packages.push_back(BearPackage());
		BEAR_ASSERT( m_packages.back().LoadFromFile(fullpath));
		begin++;
	}
#ifdef RELEASE
	RegisterFiles();
#endif
}

void BearCore::BearFileSystem::ReadConfig(const bchar * path, const bchar * file, const bchar * e, BearINI & ini, BearEncoding::Encoding coding,  BearIncluder*includer )
{
	BearStringPath fullpath;
	Update(path, file,e, fullpath);
	ini.LoadFromFile(fullpath, coding, includer);
}

void BearCore::BearFileSystem::ReadConfig(const bchar * path, const bchar * file, BearINI & ini, BearEncoding::Encoding coding, BearIncluder*includer)
{
	BearStringPath fullpath;
	Update(path, file, fullpath);
	ini.LoadFromFile(fullpath, coding, includer);
}

void BearCore::BearFileSystem::WriteConfig(const bchar * path, const bchar * file, const bchar * e, bsize id, const BearINI & ini, BearEncoding::Encoding coding)
{
	BearStringPath fullpath;
	UpdatePath(path, id, fullpath);
	BearString::Contact(fullpath, BEAR_PATH);
	BearString::Contact(fullpath, file); BearString::Contact(fullpath, e);
	ini.SaveToFile(fullpath, coding);

}

void BearCore::BearFileSystem::WriteConfig(const bchar * path, const bchar * file, bsize id, const BearINI & ini, BearEncoding::Encoding coding)
{
	BearStringPath fullpath;
	UpdatePath(path, id, fullpath);
	BearString::Contact(fullpath, BEAR_PATH);
	BearString::Contact(fullpath, file); 
	ini.SaveToFile(fullpath, coding);
}

BearCore::BearStreamRef< BearCore::BearInputStream> BearCore::BearFileSystem::Read(const bchar * path, const bchar * file, const bchar * e) 
{
	BearStringPath fullpath;
	Update(path, file, e, fullpath);
	auto&filedata = m_files[fullpath];
	if (filedata.package)
		return bear_new<BearFilePackageStream>(filedata.package->FileOpen(*filedata.package_path));
	return bear_new<BearFileStream>(fullpath);
}

BearCore::BearStreamRef<BearCore::BearInputStream> BearCore::BearFileSystem::Read(const bchar * path, const bchar * file)
{
	BearStringPath fullpath;
	Update(path, file, fullpath);
	auto&filedata = m_files[fullpath];
	if(filedata.package)
		return bear_new<BearFilePackageStream>(filedata.package->FileOpen(*filedata.package_path));
	return bear_new<BearFileStream>(fullpath);
}
BearCore::BearStreamRef<BearCore::BearOutputStream> BearCore::BearFileSystem::Write(const bchar * path, const bchar * file, bsize id) 
{
	BearStringPath fullpath;
	UpdatePath(path,id,fullpath);
	BearString::Contact(fullpath, BEAR_PATH);
	BearString::Contact(fullpath, file);
	return bear_new<BearFileStream>(fullpath, BearFlags<uint8> (BearFileStream::M_Write));
}

BearCore::BearStreamRef<BearCore::BearOutputStream> BearCore::BearFileSystem::Write(const bchar * path, const bchar * file, const bchar * e, bsize id) 
{
	BearStringPath fullpath;
	UpdatePath(path, id, fullpath);
	BearString::Contact(fullpath, BEAR_PATH);
	BearString::Contact(fullpath, file); BearString::Contact(fullpath, e);
	return bear_new<BearFileStream>(fullpath, BearFlags<uint8>(BearFileStream::M_Write));
}

bool BearCore::BearFileSystem::Delete(const bchar * path, const bchar * file) 
{
	BearStringPath fullpath;
	if (ExistFileAndUpdate(path, file, fullpath))
	{
		return BearFileManager::FileDelete(fullpath);
	}
	return false;
}

bool BearCore::BearFileSystem::Delete(const bchar * path, const bchar * file, const bchar * e) 
{
	BearStringPath fullpath;
	if (ExistFileAndUpdate(path, file,e, fullpath))
	{
		return BearFileManager::FileDelete(fullpath);
	}
	return false;
}

void BearCore::BearFileSystem::GetDirectories(BearVector<BearString>& files, const bchar * path)
{
	update(path);
	auto item = m_paths.find(BearStringConteniar(path, false));
	BEAR_ASSERT(m_paths.end() != item);
	auto begin = item->second.begin();
	auto end = item->second.end();
	while (begin != end)
	{
		GetDirectoriesFromPackage(files, **begin);
		BearVector<BearString> temp;
		BearFileManager::FindDirectories(temp, **begin,TEXT("*"), false, false);
		auto begins = temp.begin();
		auto ends = temp.end();
		while (begins != ends)
		{
			auto item2 = bear_lower_bound(files.begin(), files.end(), *begins);
			if (item2 == files.end() || *item2 != *begins)
			{
				files.insert(bear_lower_bound(files.begin(), files.end(), *begins), *begins);
			}
			begins++;
		}
		begin++;
	}
}

void BearCore::BearFileSystem::GetFiles(BearVector<BearString>& files, const bchar * path, const bchar * e, bool subPath)
{
	update(path);
	auto item = m_paths.find(BearStringConteniar(path,false));
	BEAR_ASSERT(m_paths.end() != item);
	auto begin = item->second.begin();
	auto end = item->second.end();
	while (begin != end)
	{
		GetFilesFromPackage(files,**begin, e, subPath);
		BearVector<BearString> temp;
		BearFileManager::FindFiles(temp, **begin, e, false, subPath);
		auto begins = temp.begin();
		auto ends= temp.end();
		while (begins != ends)
		{
			auto item2 = bear_lower_bound(files.begin(), files.end(), *begins);
			if (item2 == files.end() || *item2 != *begins)
			{
				files.insert(bear_lower_bound(files.begin(), files.end(), *begins), *begins);
			}
			begins++;
		}
		
		begin++;
	}
}


BearCore::BearFileSystem::BearFileSystem()
{
}

BearCore::BearFileSystem::~BearFileSystem()
{
	Clear();
}

void BearCore::BearFileSystem::UpdatePath(const bchar * floder, bsize id,BearStringPath&path) 
{
	update(floder);
	BEAR_FATALERROR(m_paths[floder].size() > id, TEXT("�������� ����� �� ������� ������ ���� [%s]"), id);
	BearString::Copy(path, *m_paths[floder][id]);
}

bsize BearCore::BearFileSystem::CountPath(const bchar * floder) 
{
	update(floder);
	return m_paths[floder].size();
}

void BearCore::BearFileSystem::Clear()
{
	m_paths.clear();
	m_source_paths.clear();
}


void BearCore::BearFileSystem::AppendPath(const bchar * name, const bchar * path, const bchar * parent_path, int32 priority)
{
	m_paths.clear();
	auto item = m_source_paths.find(BearStringConteniar(name, false));
	if (item == m_source_paths.end())
	{
		m_source_paths.insert(name, BearVector<SourcePath>());
		item = m_source_paths.find(BearStringConteniar(name, false));
	}
	SourcePath path_;
	path_.priority = priority;
	BearString::Copy(path_.parent, parent_path);
	BearString::Copy(path_.path, path);
	BearFileManager::PathOptimization(path_.path);
	item->second.insert(bear_lower_bound(item->second.begin(), item->second.end(), path_), path_);
}

void BearCore::BearFileSystem::SubPath(const bchar * name)
{
	m_paths.clear();
	auto item = m_source_paths.find(BearStringConteniar(name, false));
	if (item != m_source_paths.end())
	{
		m_source_paths.erase(item);
	}
}


void BearCore::BearFileSystem::Update(const bchar * floder, const bchar * file, BearStringPath&path)
{
	
	BEAR_FATALERROR(ExistFileAndUpdate(floder,file,path), TEXT("�� ������ ���� [%s] � ���� [%s]"), file, floder);
}

void BearCore::BearFileSystem::Update(const bchar * floder, const bchar * file, const bchar * e, BearStringPath & path)
{
	BearStringPath f;
	BearString::Copy(f, file);
	BearString::Contact(f, e);
	Update(floder, f, path);
}

bool BearCore::BearFileSystem::ExistPath(const bchar * floder)
{
	return m_source_paths.find(floder) != m_source_paths.end();
}

bool BearCore::BearFileSystem::ExistDirectory(const bchar * path)
{
	if (!ExistPath(path))return false;
	update(path);

	auto item = m_paths.find(BearStringConteniar(path));
	BEAR_ASSERT(m_paths.end() != item);

	auto b = item->second.begin();
	auto e = item->second.end();

	while (b != e)
	{
		if (BearFileManager::DirectoryExists(**b))
			return true;
		b++;
	}
	return false;
}

bool BearCore::BearFileSystem::ExistDirectory(const bchar * path, const bchar * dir)
{
	if (!ExistPath(path))return false;
	update(path);

	auto item = m_paths.find(BearStringConteniar(path));
	BEAR_ASSERT(m_paths.end() != item);

	auto b = item->second.begin();
	auto e = item->second.end();

	while (b != e)
	{
		BearStringPath path_full;
		BearString::Copy(path_full, **b);
		BearString::Contact(path_full,BEAR_PATH);
		BearString::Contact(path_full, dir);
		if (BearFileManager::DirectoryExists(path_full))
			return true;
		b++;
	}
	return false;
}

bool BearCore::BearFileSystem::ExistDirectory(const bchar * floder, bsize id)
{
	if (!ExistPath(floder))return false;
	update(floder);

	auto item = m_paths.find(BearStringConteniar(floder));
	BEAR_ASSERT(m_paths.end() != item);

	BEAR_FATALERROR(item->second.size() > id, TEXT("�������� ����� �� ������� ������ ���� [%s]"), id);
	return BearFileManager::DirectoryExists(*item->second[id]);
}

bool BearCore::BearFileSystem::ExistDirectoryAndUpdate(const bchar * floder, bsize id, BearStringPath&full)
{
	if (!ExistPath(floder))return false;
	update(floder);

	auto item = m_paths.find(BearStringConteniar(floder));
	BEAR_ASSERT(m_paths.end() != item);

	BEAR_FATALERROR(item->second.size() > id, TEXT("�������� ����� �� ������� ������ ���� [%s]"), id);
	BearString::Copy(full, *item->second[id]);
	return BearFileManager::DirectoryExists(*item->second[id]);
}

bool BearCore::BearFileSystem::ExistFileAndUpdate(const bchar * floder, const bchar * file, BearStringPath&path)
{
	if (!ExistPath(floder))return false;
	update(floder);
	auto item = m_paths.find(BearStringConteniar(floder));
	BEAR_ASSERT(m_paths.end() != item);
	auto begin = item->second.begin();
	auto end = item->second.end();
	while (begin != end)
	{
		BearString::Copy(path, **begin);
		BearString::Contact(path, BEAR_PATH);
		BearString::Contact(path, file);
		BearString::ToLower(path);
		if (RegisterFile(path))return true;
		begin++;
	}
	return false;
}

void BearCore::BearFileSystem::CreateDirectory(const bchar * floder, bsize id)
{
	BearStringPath p, f,t;
	t[0] = 0;
	bchar*path = p;
	UpdatePath(floder, id, p);
	path = BearString::ReadTo(path, BEAR_PATH, f);
	do
	{
		BearString::Contact(t, f);
		BearString::Contact(t, BEAR_PATH);
		if (f[1] != ':')
		if (!BearFileManager::DirectoryExists(t))
		{
			BEAR_ASSERT(BearFileManager::DirectoryCreate(t));
		}
		path = BearString::ReadTo(path, BEAR_PATH, f);
	} while (f[0]);
}

bool BearCore::BearFileSystem::ExistFile(const bchar * floder, const bchar * file)
{
	BearStringPath path;
	return ExistFileAndUpdate(floder, file,path);
}

bool BearCore::BearFileSystem::ExistFileAndUpdate(const bchar * floder, const bchar * file, const bchar * e, BearStringPath& path)
{
	BearStringPath f;
	BearString::Copy(f, file);
	BearString::Contact(f, e);
	return ExistFileAndUpdate(floder, f, path);
}

bool BearCore::BearFileSystem::ExistFile(const bchar * floder, const bchar * file, const bchar * e)
{
	BearStringPath path;
	return ExistFileAndUpdate(floder, file,e, path);
}
