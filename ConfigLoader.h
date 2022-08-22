#ifndef _CONFIGLOADER_H_
#define _CONFIGLOADER_H_

// by git@github.com:yuwf/jsonbind.git

#include<memory>
#include<iosfwd>
#include "jsonbind.hpp"

#include "LLog.h"
#define ConfigLogError LLOG_FATAL
#define ConfigLogInfo LLOG_INFO

// 多线程安全
// 要求 T 定义 Normalize 函数
template <class T>
class ConfigLoader
{
public:
	// 获取配置对象，数据对象是只读的，可以被多线程共享
	// 获取的对象一定不为空，外层无需判断
	std::shared_ptr<const T> GetConfig()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_data;
	}

	// 获取原始配置
	std::string GetSrc()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_src;
	}

	// 依赖json_bind
	bool LoadFromJsonString(const std::string& src)
	{
		T* data = new T();
		std::string err;
		if (!data->from_string(src, &err))
		{
			delete data;
			ConfigLogError("LoadJsonFromString err=%s src=%s", err.c_str(), src.c_str());
			return false;
		}

		ConfigLogInfo("LoadJsonFromString Success src=%s", src.c_str());
		data->Normalize();

		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_data = std::shared_ptr<const T>(data);
			m_src = src;
		}
		return true;
	}

	bool LoadJsonFromFile(const std::string& filename)
	{
		std::ifstream ifs(filename.c_str(), std::ifstream::in);
		if (!ifs.is_open())
		{
			ConfigLogError("LoadJsonFromFile open fail filename=%s", filename.c_str());
			return false;
		}
		std::stringstream ss;
		try
		{
			ss << ifs.rdbuf();
		}
		catch (const exception& ex)
		{
			ConfigLogError("LoadJsonFromFile read fail filename=%s ex=%s", filename.c_str(), ex.what());
			ifs.close();
			return false;
		}
		ifs.close();
		return LoadFromJsonString(ss.str());
	}

	bool SaveToFile(const std::string& filename)
	{
		std::ofstream ofs(filename, std::ofstream::out);
		if (!ofs.is_open())
		{
			ConfigLogError("SaveToFile open fail filename=%s", filename.c_str());
			return false;
		}
		std::string src = GetSrc();
		try
		{
			ofs << src;	// 文本输出
		}
		catch (const exception& ex)
		{
			ConfigLogError("SaveToFile write fail filename=%s ex=%s", filename.c_str(), ex.what());
			ofs.close();
			return false;
		}
		ofs.close();
		return true;
	}

private:
	std::mutex m_mutex;
	std::shared_ptr<const T> m_data = std::make_shared<const T>(); // 初始值
	std::string m_src;	// 原始配置
};

#endif