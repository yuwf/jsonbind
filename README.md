# jsonbind
将结构与nlohmann::json绑定，结构和json可以相互转换
支持C++接口的序列化

# 使用案例
```
struct Student
{
	struct Pen
	{
		std::string name;
		std::string count;
		JSON_BIND((name)(count)); // 绑定结构字段和json
	};
	struct HomeWork
	{
		std::string name;
		std::vector<int> finish;
		JSON_BIND_MAP(("n", name)("f", finish)); // json中n对应name字段 此种方式可以缩小key的大小
	};

	std::string name;
	int age = 0;
	std::vector<Pen> pens;
	std::map<std::string, HomeWork> homeworks;

	JSON_BIND((name)(age)(homeworks)(pens)); // 绑定结构字段和json
};

Student student;

// 结构和json格式相互转化
json j;
student.to_json(j);
student.from_json(j);

// 结构和json格式的string相互转化
std::string str;
student.to_string(str);
student.from_string(str);
student.from_buff(str.data(), str.size());

// 结构和msgpack格式相互转化 经测试对比性能要比msgpack官方提供的包慢4倍 json这个库的性能并不是最优的，但使用是比较便捷
std::vector<char> msgpack;
student.to_msgpack(msgpack);
student.from_msgpack(msgpack.data(), msgpack.size());
```
