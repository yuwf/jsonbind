#ifndef INCLUDE_JSONBIND_HPP_
#define INCLUDE_JSONBIND_HPP_

// by git@github.com:yuwf/jsonbind.git

// 将结构与nlohmann::json绑定，结构和json可以相互转换
// https://github.com/nlohmann/json
#include <json.hpp>
using json = nlohmann::json;

/* 使用案例
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
*/

// 自定义类型支持嵌套时，json内部调用转化成json
namespace nlohmann
{
	// 支持结构和json转化
	template <typename T>
	struct adl_serializer<T, typename std::enable_if<T::JSON_BIND_SUPPORT>::type>
	{
		static void to_json(json& j, const T& o)
		{
			o.to_json(j);
		}
		static void from_json(const json& j, T& o)
		{
			if (!j.is_object())
			{
				throw detail::type_error::create(302, std::string("type must be object[") + typeid(T).name() + "], but is " + std::string(j.type_name()), j);
			}
			o._from_json(j);
		}
	};

	// std::map的key类型为int时，json内部按数组处理，这里给调整下
	template <typename Key, typename Value, typename Compare, typename Allocator>
	struct adl_serializer<std::map<Key, Value, Compare, Allocator>, typename std::enable_if<std::is_integral<Key>::value>::type>
	{
		using T = std::map<Key, Value, Compare, Allocator>;

		static void to_json(json& j, const T& o)
		{
			for (const auto& it : o)
			{
				j[std::to_string(it.first)] = it.second;
			}
		}
		static void from_json(const json& j, T& o)
		{
			if (!j.is_object())
			{
				throw detail::type_error::create(302, std::string("type must be object[") + typeid(T).name() + "], but is " + std::string(j.type_name()), j);
			}

			const auto* inner_object = j.template get_ptr<const typename json::object_t*>();
			std::transform(
				inner_object->begin(), inner_object->end(),
				std::inserter(o, o.begin()),
				[](typename json::object_t::value_type const& p)
				{
					return T::value_type(atoi(p.first.c_str()), p.second.template get<typename T::mapped_type>());
				});
		}
	};
}

#define JSON_BIND_SEQ_FOR(...) JSON_BIND_SEQ_FOR_TAIL(__VA_ARGS__)
#define JSON_BIND_SEQ_FOR_TAIL(...) __VA_ARGS__##0

// seq中取第一个字段形成新的seq
// (first, second)(first, second)(first, second) -> (first)(first)(first)
#define JSON_BIND_SEQFIRST_TO_SEQ_A(first, second) JSON_BIND_SEQFIRST_TO_SEQ_I(first, second) JSON_BIND_SEQFIRST_TO_SEQ_B
#define JSON_BIND_SEQFIRST_TO_SEQ_B(first, second) JSON_BIND_SEQFIRST_TO_SEQ_I(first, second) JSON_BIND_SEQFIRST_TO_SEQ_A
#define JSON_BIND_SEQFIRST_TO_SEQ_A0
#define JSON_BIND_SEQFIRST_TO_SEQ_B0
#define JSON_BIND_SEQFIRST_TO_SEQ_I(first, second) (first)
// seq中取第二个字段形成新的seq
// (first, second)(first, second)(first, second) -> (second)(second)(second)
#define JSON_BIND_SEQSECOND_TO_SEQ_A(first, second) JSON_BIND_SEQSECOND_TO_SEQ_I(first, second) JSON_BIND_SEQSECOND_TO_SEQ_B
#define JSON_BIND_SEQSECOND_TO_SEQ_B(first, second) JSON_BIND_SEQSECOND_TO_SEQ_I(first, second) JSON_BIND_SEQSECOND_TO_SEQ_A
#define JSON_BIND_SEQSECOND_TO_SEQ_A0
#define JSON_BIND_SEQSECOND_TO_SEQ_B0
#define JSON_BIND_SEQSECOND_TO_SEQ_I(first, second) (second)

// seq中取第一个字段形成新的tuple
// (first, second)(first, second)(first, second) -> first,first,first,
#define JSON_BIND_SEQFIRST_TO_TUPLE_A(first, second) JSON_BIND_SEQFIRST_TO_TUPLE_I(first, second) JSON_BIND_SEQFIRST_TO_TUPLE_B
#define JSON_BIND_SEQFIRST_TO_TUPLE_B(first, second) JSON_BIND_SEQFIRST_TO_TUPLE_I(first, second) JSON_BIND_SEQFIRST_TO_TUPLE_A
#define JSON_BIND_SEQFIRST_TO_TUPLE_A0
#define JSON_BIND_SEQFIRST_TO_TUPLE_B0
#define JSON_BIND_SEQFIRST_TO_TUPLE_I(first, second) first,
// seq中取第二个字段形成新的tuple
// (first, second)(first, second)(first, second) -> second,second,second,
#define JSON_BIND_SEQSECOND_TO_TUPLE_A(first, second) JSON_BIND_SEQSECOND_TO_TUPLE_I(first, second) JSON_BIND_SEQSECOND_TO_TUPLE_B
#define JSON_BIND_SEQSECOND_TO_TUPLE_B(first, second) JSON_BIND_SEQSECOND_TO_TUPLE_I(first, second) JSON_BIND_SEQSECOND_TO_TUPLE_A
#define JSON_BIND_SEQSECOND_TO_TUPLE_A0
#define JSON_BIND_SEQSECOND_TO_TUPLE_B0
#define JSON_BIND_SEQSECOND_TO_TUPLE_I(first, second) second,

//seq转化成新的tuple
// (member)(member)(member) -> member,member,member,
#define JSON_BIND_SEQ_TO_TUPLE_A(member) JSON_BIND_SEQ_TO_TUPLE_I(member) JSON_BIND_SEQ_TO_TUPLE_B
#define JSON_BIND_SEQ_TO_TUPLE_B(member) JSON_BIND_SEQ_TO_TUPLE_I(member) JSON_BIND_SEQ_TO_TUPLE_A
#define JSON_BIND_SEQ_TO_TUPLE_A0
#define JSON_BIND_SEQ_TO_TUPLE_B0
#define JSON_BIND_SEQ_TO_TUPLE_I(member) #member,

// seq转化成json
// (member)(member)(member) -> j.emplace(#member, member);j.emplace(#member, member);j.emplace(#member, member);
#define JSON_BIND_SEQ_TO_JSON_A(member) JSON_BIND_SEQ_TO_JSON_I(member) JSON_BIND_SEQ_TO_JSON_B
#define JSON_BIND_SEQ_TO_JSON_B(member) JSON_BIND_SEQ_TO_JSON_I(member) JSON_BIND_SEQ_TO_JSON_A
#define JSON_BIND_SEQ_TO_JSON_A0
#define JSON_BIND_SEQ_TO_JSON_B0
#define JSON_BIND_SEQ_TO_JSON_I(member) j.emplace(#member, member);

//seq转化成json，内部嵌套的是pair结构
// (first, second)(first, second)(first, second) -> j.emplace(first, second);j.emplace(first, second);j.emplace(first, second);
#define JSON_BIND_SEQMAP_TO_JSON_A(first, second) JSON_BIND_SEQMAP_TO_JSON_I(first, second) JSON_BIND_SEQMAP_TO_JSON_B
#define JSON_BIND_SEQMAP_TO_JSON_B(first, second) JSON_BIND_SEQMAP_TO_JSON_I(first, second) JSON_BIND_SEQMAP_TO_JSON_A
#define JSON_BIND_SEQMAP_TO_JSON_A0
#define JSON_BIND_SEQMAP_TO_JSON_B0
#define JSON_BIND_SEQMAP_TO_JSON_I(first, second) j.emplace(first, second);

// json转化成结构
#define JSON_BIND_SEQ_FROM_JSON_A(member) JSON_BIND_SEQ_FROM_JSON_I(member) JSON_BIND_SEQ_FROM_JSON_B
#define JSON_BIND_SEQ_FROM_JSON_B(member) JSON_BIND_SEQ_FROM_JSON_I(member) JSON_BIND_SEQ_FROM_JSON_A
#define JSON_BIND_SEQ_FROM_JSON_A0
#define JSON_BIND_SEQ_FROM_JSON_B0
#define JSON_BIND_SEQ_FROM_JSON_I(member) \
	{ \
		json::const_iterator it = j.find(#member); \
		if (it != j.end() && !it->is_null()) \
		{ \
			it->get_to(member); \
		} \
	}
// json转化成结构
#define JSON_BIND_SEQMAP_FROM_JSON_A(first, second) JSON_BIND_SEQMAP_FROM_JSON_I(first, second) JSON_BIND_SEQMAP_FROM_JSON_B
#define JSON_BIND_SEQMAP_FROM_JSON_B(first, second) JSON_BIND_SEQMAP_FROM_JSON_I(first, second) JSON_BIND_SEQMAP_FROM_JSON_A
#define JSON_BIND_SEQMAP_FROM_JSON_A0
#define JSON_BIND_SEQMAP_FROM_JSON_B0
#define JSON_BIND_SEQMAP_FROM_JSON_I(first, second) \
	{ \
		json::const_iterator it = j.find(first); \
		if (it != j.end() && !it->is_null()) \
		{ \
			it->get_to(second); \
		} \
	}

// 定义的常用一些函数
#define JSON_BIND_FUNCTION() \
	void to_string(std::string& str, int indent = -1) \
	{ \
		json j; \
		to_json(j); \
		nlohmann::detail::serializer<json> s(nlohmann::detail::output_adapter<char, std::string>(str), ' ', nlohmann::detail::error_handler_t::replace); \
		if (indent >= 0) \
			s.dump(*this, true, true, static_cast<unsigned int>(indent)); \
		else \
			s.dump(*this, false, true, 0); \
	} \
	bool from_string(const std::string& str, std::string* err = NULL) \
	{ \
		json j; \
		nlohmann::detail::span_input_adapter adapter(str.data(), str.size()); \
		auto ia = adapter.get(); \
		nlohmann::detail::parser<json, decltype(ia)> parser(std::move(ia), nullptr, true, false); \
		try \
		{ \
			parser.parse(true, j); \
		} \
		catch (const std::exception& ex) \
		{ \
			if (err) { *err = ex.what(); } \
			return false; \
		} \
		return from_json(j, err); \
	} \
	bool from_buff(const char* buff, std::size_t len, std::string* err = NULL) \
	{ \
		json j; \
		nlohmann::detail::span_input_adapter adapter(buff, len); \
		auto ia = adapter.get(); \
		nlohmann::detail::parser<json, decltype(ia)> parser(std::move(ia), nullptr, true, false); \
		try \
		{ \
			parser.parse(true, j); \
		} \
		catch (const std::exception& ex) \
		{ \
			if (err) { *err = ex.what(); } \
			return false; \
		} \
		return from_json(j, err); \
	} \
	void to_msgpack(std::vector<char>& buff) \
	{ \
		json j; \
		to_json(j); \
		nlohmann::detail::output_adapter<char> oa(buff); \
		nlohmann::json::to_msgpack(j, oa); \
	} \
	bool from_msgpack(const char* buff, std::size_t len, std::string* err = NULL) \
	{ \
		json j; \
		nlohmann::detail::json_sax_dom_parser<json> sdp(j, true); \
		nlohmann::detail::span_input_adapter adapter(buff, len); \
		auto ia = adapter.get(); \
		nlohmann::detail::binary_reader<json, decltype(ia)> reader(std::move(ia)); \
		try \
		{ \
			reader.sax_parse(nlohmann::detail::input_format_t::msgpack, &sdp, true); \
		} \
		catch (const std::exception& ex) \
		{ \
			if (err) { *err = ex.what();} \
		} \
		if (sdp.is_errored()) return false;\
		return from_json(j, err); \
	}

// 参数seq，结构字段
// (member)(member)
// member为结构字段
// _from_json 是给内部使用的 他不包异常，异常一直往上传导
#define JSON_BIND(...) \
	enum { JSON_BIND_SUPPORT = 1 };\
	static const std::vector<std::string>& json_key() \
	{ \
		static std::vector<std::string> keys = { JSON_BIND_SEQ_FOR(JSON_BIND_SEQ_TO_TUPLE_A __VA_ARGS__) }; \
		return keys; \
	} \
	void to_json(json& j) const \
	{ \
		JSON_BIND_SEQ_FOR(JSON_BIND_SEQ_TO_JSON_A __VA_ARGS__) \
	} \
	bool from_json(const json& j, std::string* err = NULL) \
	{ \
		if (!j.is_object()) \
		{ \
			if (err) { *err = std::string("type must be object[") + typeid(*this).name() + "], but is " + std::string(j.type_name()); } \
			return false; \
		} \
		try \
		{ \
			JSON_BIND_SEQ_FOR(JSON_BIND_SEQ_FROM_JSON_A __VA_ARGS__) \
			return true; \
		} \
		catch (const std::exception& ex) \
		{ \
			if (err) { *err = ex.what(); } \
		} \
		return false; \
	} \
	void _from_json(const json& j) \
	{ \
		JSON_BIND_SEQ_FOR(JSON_BIND_SEQ_FROM_JSON_A __VA_ARGS__) \
	} \
	JSON_BIND_FUNCTION();

// 参数seq结构，内部嵌套的是pair结构
// (first, second)(first, second)
// first为std::string的类型或可以隐世转出std::string类型
// second为结构字段
// _from_json 是给内部使用的 他不包异常，异常一直往上传导
#define JSON_BIND_MAP(...) \
	enum { JSON_BIND_SUPPORT = 1 };\
	static const std::vector<std::string>& json_key() \
	{ \
		static std::vector<std::string> keys = { JSON_BIND_SEQ_FOR(JSON_BIND_SEQFIRST_TO_TUPLE_A __VA_ARGS__) }; \
		return keys; \
	} \
	void to_json(json& j) const \
	{ \
		JSON_BIND_SEQ_FOR(JSON_BIND_SEQMAP_TO_JSON_A __VA_ARGS__) \
	} \
	bool from_json(const json& j, std::string* err = NULL) \
	{ \
		if (!j.is_object()) \
		{ \
			if (err) { *err = std::string("type must be object[") + typeid(*this).name() + "], but is " + std::string(j.type_name()); } \
			return false; \
		} \
		try \
		{ \
			JSON_BIND_SEQ_FOR(JSON_BIND_SEQMAP_FROM_JSON_A __VA_ARGS__) \
			return true; \
		} \
		catch (const std::exception& ex) \
		{ \
			if (err){ *err = ex.what();} \
		} \
		return false; \
	} \
	void _from_json(const json& j) \
	{ \
		JSON_BIND_SEQ_FOR(JSON_BIND_SEQMAP_FROM_JSON_A __VA_ARGS__) \
	} \
	JSON_BIND_FUNCTION();

	
#endif