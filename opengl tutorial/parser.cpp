#include <boost/spirit/home/x3.hpp>
#include <string>
#include "ComposedObject.h"
#include "parser.h"
#include <map>
#include <tuple>
#include "objects_fabric.h"
using namespace boost::spirit;
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <iostream>
namespace Parser
{

	x3::rule<class word, std::string> liter_rule;
	auto liter_rule_def = x3::alpha >> *( x3::alnum | x3::char_('_'));
	BOOST_SPIRIT_DEFINE(liter_rule);

	x3::rule<class vec2, Vector<2>> vec2 = "Vector<2>";
	auto vec2_setx = [](auto& ctx) { x3::_val(ctx).nums[0] = x3::_attr(ctx); };
	auto vec2_sety = [](auto& ctx) { x3::_val(ctx).nums[1] = x3::_attr(ctx); };
	auto vec3_setz = [](auto& ctx) { x3::_val(ctx).nums[2] = x3::_attr(ctx); };
	auto vec4_setw = [](auto& ctx) { x3::_val(ctx).nums[3] = x3::_attr(ctx); };
	auto const vec2_def = ('{' >> x3::double_[vec2_setx] >> ',' >> x3::double_[vec2_sety] >> '}');
	BOOST_SPIRIT_DEFINE(vec2);
	x3::rule<class vec3, Vector<3>> vec3 = "Vector<3>";
	auto const vec3_def = ('{' >> x3::double_[vec2_setx] >> ',' >> x3::double_[vec2_sety] >> ',' >> x3::double_[vec3_setz] >> '}');
	BOOST_SPIRIT_DEFINE(vec3);
	x3::rule<class vec4, Vector<4>> vec4 = "Vector<4>";
	auto const vec4_def = ('{' >> x3::double_[vec2_setx] >> ',' >> x3::double_[vec2_sety] >> ',' >> x3::double_[vec3_setz] >> ',' >>
		x3::double_[vec4_setw] >> '}');
	BOOST_SPIRIT_DEFINE(vec4);

	x3::rule<class vectorvec2, std::vector<Vector<2>>> vector_vec2 = "std::vector<Vector<2>>";
	auto vector_vec2_def = '{' >> vec2 % ',' >> '}';
	BOOST_SPIRIT_DEFINE(vector_vec2);
	x3::rule<class vectorvec3, std::vector<Vector<3>>> vector_vec3 = "std::vector<Vector<3>>";
	auto vector_vec3_def = '{' >> vec3 % ',' >> '}';
	BOOST_SPIRIT_DEFINE(vector_vec3);
	x3::rule<class vectorvec4, std::vector<Vector<4>>> vector_vec4 = "std::vector<Vector<4>>";
	auto vector_vec4_def = '{' >> vec4 % ',' >> '}';
	BOOST_SPIRIT_DEFINE(vector_vec4);

	x3::rule<class vectorint, std::vector<int>> vector_int = "std::vector<int>";
	auto vector_int_def = x3::lit("{") >> x3::int_ % ',' >> '}';
	BOOST_SPIRIT_DEFINE(vector_int);

	x3::rule<class vectorvectorint, std::vector<std::vector<int>>> vec_vec_int;
	auto vec_vec_int_def = '{' >> vector_int % ',' >> "}";
	BOOST_SPIRIT_DEFINE(vec_vec_int);

	x3::rule<class quater, Quat> quat_rule = "Quat";
	auto set_a0 = [](auto& ctx) {x3::_val(ctx)._a0 = x3::_attr(ctx); };
	auto set_a = [](auto& ctx) {x3::_val(ctx).a = x3::_attr(ctx); };
	auto quat_rule_def = x3::double_[set_a0] >> '+' >> vec3[set_a];
	BOOST_SPIRIT_DEFINE(quat_rule);




	/* »того, при добавлении нового типа:
	* написать правило дл€ него
	* добавить в enum
	* добавить DATA_TYPE_ASSOSIATE
	* добавить в argument_rule_def
	*/
	enum class DataType
	{
		ERR,
		INT,
		DOUBLE,
		VEC_2,
		VEC_3,
		VEC_4,
		VEC_VEC_2,
		VEC_VEC_3,
		VEC_VEC_4,
		VEC_INT,
		VEC_VEC_INT,
		QUAT
	};
	//функци€ заглушка дл€ того чтобы делать специализации (без этого нельз€)
	template <typename T> DataType getType();

#define DATA_TYPE_ASSOSIATE(cpp_type, data_type) template<> DataType getType<cpp_type>() { return DataType::data_type; }

	DATA_TYPE_ASSOSIATE(int, INT);
	DATA_TYPE_ASSOSIATE(double, DOUBLE);
	DATA_TYPE_ASSOSIATE(Vector<2>, VEC_2);
	DATA_TYPE_ASSOSIATE(Vector<3>, VEC_3);
	DATA_TYPE_ASSOSIATE(Vector<4>, VEC_4);
	DATA_TYPE_ASSOSIATE(std::vector<Vector<2>>, VEC_VEC_2);
	DATA_TYPE_ASSOSIATE(std::vector<Vector<3>>, VEC_VEC_3);
	DATA_TYPE_ASSOSIATE(std::vector<Vector<4>>, VEC_VEC_4);
	DATA_TYPE_ASSOSIATE(std::vector<int>, VEC_INT);
	DATA_TYPE_ASSOSIATE(Quat, QUAT);
	DATA_TYPE_ASSOSIATE(std::vector<std::vector<int>>, VEC_VEC_INT);

	class ArgumentData
	{
		void* data;
		DataType _type;
		size_t data_size;
	public:
		template<typename T>
		ArgumentData(const T& data_obj) : _type(getType<T>()), data_size(sizeof(T)) {
			data = (void*)new T(data_obj);
		}
		ArgumentData& operator=(ArgumentData&& mov) { data = mov.data; mov.data = nullptr; _type = mov._type, data_size = mov.data_size; return *this; }
		ArgumentData() : data(nullptr), _type(DataType::ERR), data_size(0) {}
		~ArgumentData() { delete data; }
		DataType type() const { return _type; }
		ArgumentData(const ArgumentData& cop) :data_size(cop.data_size), _type(cop._type) {this->data = malloc(data_size); memcpy(data, cop.data, data_size); };
		ArgumentData(ArgumentData&& mov) : data(mov.data), _type(mov._type), data_size(mov.data_size) { mov.data = nullptr; }
		template <typename T>
		const T& getData() const { return *((T*)data); }
	};

	x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

	x3::rule<class data_obj, ArgumentData> value_rule;
	auto value_rule_def = vec2 | vec3 | vec4 | vector_vec2 | vector_vec3 | vector_vec4 | vector_int | vec_vec_int | quat_rule | strict_double | x3::int_;
	BOOST_SPIRIT_DEFINE(value_rule);

	x3::rule<class func_argument, std::tuple<std::string, ArgumentData>> argument_rule;
	auto argument_rule_def = liter_rule >> (x3::lit('=') | x3::lit(':')) >> value_rule;
	BOOST_SPIRIT_DEFINE(argument_rule);

	x3::rule<class arg_list, std::map<std::string, ArgumentData>> arguments_list_rule;
	auto ins_tuple_to_map = [](auto& ctx)->void {x3::_val(ctx).insert(std::pair<std::string, ArgumentData>( std::get<0>(x3::_attr(ctx)), std::move(std::get<1>(x3::_attr(ctx))) )); };
	auto arguments_list_rule_def = argument_rule[ins_tuple_to_map] % (x3::lit(',') | x3::lit(';'));
	BOOST_SPIRIT_DEFINE(arguments_list_rule);


	/*
	ѕри добавлении примитива:
	ƒописать функцию создани€ в objects_facric.h не забыть про макрос
	написать DEFINE_PRIMITIVE_RULE
	дописать в primitive_rule_def
	*/

	// хз на самом деле вместо того чтобы городить вот это где по разному об€зательные и опциональные аргументы считаютс€ мб нужно было уже просто все в формате фабрики делать

#define OPTIONAL_ARGUMENT_SETTER(lc_name, uc_name, Type) {#lc_name, [](std::unique_ptr<Object>& obj, ArgumentData data) { if (data.type() == DataType::INT && getType<Type>() == DataType::DOUBLE) data = ArgumentData::ArgumentData(double(data.getData<int>()));\
if (data.type() != getType<Type>()) throw "Provided optional argument " + std::string(#lc_name) + " with wrong type"; \
	obj->set##uc_name (data.getData<Type>());}}

	const std::map<std::string, std::function<void(std::unique_ptr<Object>&, ArgumentData)>> optional_arguments_setter{
		OPTIONAL_ARGUMENT_SETTER(color, Color, Vector<3>),
		OPTIONAL_ARGUMENT_SETTER(alpha, Alpha, double),
		OPTIONAL_ARGUMENT_SETTER(id, Id, int),
		OPTIONAL_ARGUMENT_SETTER(bb_hsize, BoundingBoxHSize, Vector<3>),
		OPTIONAL_ARGUMENT_SETTER(bb_position, BoundingBoxPosition, Vector<3>)
	};

	void checkProvidedArguments(std::vector<std::pair<std::string, DataType>>& acceptable, const std::map<std::string, ArgumentData>& provided)
	{
		for (auto &it : acceptable)
		{
			auto pr_it = provided.find(it.first);
			if (pr_it == provided.end())
			{
				throw "Missing argument " + it.first;
			}
			else if (it.second != pr_it->second.type() && !(it.second == DataType::DOUBLE && pr_it->second.type() == DataType::INT))
					throw "Provided argument " + it.first + " with wrong type";
		}
	}
	template<typename T>
	std::tuple<T> makeTupleOfArguments(int arg_num, const std::vector<std::pair<std::string, DataType>>& arg_list, const std::map<std::string, ArgumentData>& provided)
	{
		return std::make_tuple( provided.find(arg_list[arg_num].first)->second.getData<T>() );
	}
	template<typename T, typename U, typename ... Args>
	std::tuple<T, U, Args...> makeTupleOfArguments(int arg_num, const std::vector<std::pair<std::string, DataType>>& arg_list, const std::map<std::string, ArgumentData>& provided)
	{
		return std::tuple_cat(makeTupleOfArguments<T>(arg_num, arg_list , provided), makeTupleOfArguments<U, Args...>(arg_num + 1, arg_list, provided));
	}
	template<typename T>
	void addDataTypeToArgList(int arg_num, const std::vector<std::string>& arg_list, std::vector<std::pair<std::string, DataType>>& res)
	{
		res[arg_num] = { arg_list[arg_num], getType<T>() };
	}
	template<typename T, typename U, typename ... Args>
	void addDataTypeToArgList(int arg_num, const std::vector<std::string>& arg_list, std::vector<std::pair<std::string, DataType>>& res)
	{
		addDataTypeToArgList<T>(arg_num, arg_list, res);
		addDataTypeToArgList<U, Args...>(arg_num + 1, arg_list, res);
	}



	template<typename Primitive, typename ... Args>
	std::unique_ptr<Object> makePrimitiveFromArguments(const std::vector<std::string>& arguments, std::map<std::string, ArgumentData> provided_args)
	{
		std::vector<std::pair<std::string, DataType>> args_with_types(arguments.size());
		addDataTypeToArgList<Args...>(0, arguments, args_with_types);
		checkProvidedArguments(args_with_types, provided_args);
		//recast int to double
		for (auto& it : args_with_types)
		{
			if (it.second == DataType::DOUBLE && provided_args[it.first].type() == DataType::INT)
				provided_args[it.first] = ArgumentData::ArgumentData(double(provided_args[it.first].getData<int>()));
		}


		auto tuple = makeTupleOfArguments<Args...>(0, args_with_types, provided_args);
		auto res = std::apply(makeObject<Primitive, Args...>::get, tuple);
		for (auto& it : provided_args)
		{
			if (std::find(arguments.begin(), arguments.end(), it.first) == arguments.end())
			{
				auto opt_it = optional_arguments_setter.find(it.first);
				if (opt_it == optional_arguments_setter.end())
					throw "Uknown argument " + it.first;
				opt_it->second(res, it.second);
			}
		}
		return std::move(res);
	}

	void setUpOnlyOptionalArgumentsList(std::unique_ptr<Object>& object, const std::map<std::string, ArgumentData>& provided_args)
	{
		for (auto& it : provided_args)
		{
			auto opt_it = optional_arguments_setter.find(it.first);
			if (opt_it == optional_arguments_setter.end())
				throw "Uknown argument " + it.first;
			opt_it->second(object, it.second);
		}
	}



#define DEFINE_PRIMITIVE_RULE(prim, lower_case_pr, args_list, ...)\
	auto make_primitive_##prim##_func = [](auto &ctx) { x3::_val(ctx) = makePrimitiveFromArguments<prim, __VA_ARGS__>(std::vector<std::string> args_list, x3::_attr(ctx));};\
	x3::rule<class lower_case_pr##rule_type, std::unique_ptr<Object>> lower_case_pr##_rule;\
	auto lower_case_pr##_rule_def = x3::lit(#prim) >> '(' >> arguments_list_rule[make_primitive_##prim##_func] >> ')';  \
	BOOST_SPIRIT_DEFINE(lower_case_pr##_rule);



	


		DEFINE_PRIMITIVE_RULE(Prizm, prizm, ({ "base", "height", "position", "rotation" }), std::vector<Vector<2>>, double, Vector<3>, Quat);
		DEFINE_PRIMITIVE_RULE(Box, box, ({ "hsize", "position", "rotation" }), Vector<3>, Vector<3>, Quat);
		DEFINE_PRIMITIVE_RULE(Cone, cone, ({ "height", "radius", "position", "rotation" }), double, double, Vector<3>, Quat);
		DEFINE_PRIMITIVE_RULE(Piramid, piramid, ({ "base", "height", "position", "rotation" }), std::vector<Vector<2>>, double, Vector<3>, Quat);
		DEFINE_PRIMITIVE_RULE(Cylinder, cylinder, ({ "height", "radius", "position", "rotation" }), double, double, Vector<3>, Quat);
		DEFINE_PRIMITIVE_RULE(Sphere, sphere, ({ "radius", "position" }), double, Vector<3>);
		DEFINE_PRIMITIVE_RULE(Polyhedron, polyhedron, ({ "points", "edges", "position", "rotation" }), std::vector<Vector<3>>, std::vector<std::vector<int>>, Vector<3>, Quat);

	x3::rule<class expr, std::unique_ptr<Object>> expression_rule;




	x3::rule<class composed_tuple, std::tuple<std::unique_ptr<Object>, std::map<std::string, ArgumentData>>> composed_object_tuple_rule;
	auto composed_object_tuple_rule_def = x3::lit("Obj") >> '(' >> expression_rule >> (x3::lit(";") | x3::lit(",")) >> arguments_list_rule >> x3::lit(')');
	BOOST_SPIRIT_DEFINE(composed_object_tuple_rule);
	auto make_primitive_composed_func = [](auto& ctx) {
		x3::_val(ctx) = std::move(std::get<0>(x3::_attr(ctx))); setUpOnlyOptionalArgumentsList(x3::_val(ctx), std::get<1>(x3::_attr(ctx)));
	};
	x3::rule<class composed, std::unique_ptr<Object>> composed_object_rule;
	auto composed_object_rule_def = composed_object_tuple_rule[make_primitive_composed_func];
	BOOST_SPIRIT_DEFINE(composed_object_rule);



	x3::rule<class primitive, std::unique_ptr<Object>> primitive_rule;
	auto primitive_rule_def = box_rule | prizm_rule | cone_rule | piramid_rule | cylinder_rule | sphere_rule | polyhedron_rule | composed_object_rule;
	
	BOOST_SPIRIT_DEFINE(primitive_rule);
	//ƒќЋ∆Ќј ќ„»ўј“№—я ѕ≈–≈ƒ па–—»Ќќ√ќћ
	std::map<std::string, std::unique_ptr<Object>> __vars_map;

	

	x3::rule<class assign_tuple, std::tuple<std::string, std::unique_ptr<Object>>> tuple_assignation_rule;
	auto tuple_assignation_rule_def = liter_rule >> '=' >> (primitive_rule | expression_rule);
	void assignVariable(const std::string& str, std::unique_ptr<Object>&& prim) {
		__vars_map.insert({ str, std::move(prim) });
	}
	BOOST_SPIRIT_DEFINE(tuple_assignation_rule);

	x3::rule<class assign, x3::unused_type> assignation_rule;
	auto assignation_rule_def = tuple_assignation_rule[([](auto& ctx)
		{
			assignVariable(std::get<0>(x3::_attr(ctx)), std::move(std::get<1>(x3::_attr(ctx)))); 
		})];
	BOOST_SPIRIT_DEFINE(assignation_rule);

	x3::rule<class var, std::unique_ptr<Object>> variable_rule;
	auto variable_rule_def = liter_rule[([](auto& ctx) { x3::_val(ctx) = __vars_map.find(x3::_attr(ctx))->second->copy(); })];
	BOOST_SPIRIT_DEFINE(variable_rule);




	

	x3::rule<class group, std::unique_ptr<Object>> group_rule;
	auto group_rule_def = x3::lit("(") >> expression_rule >> ')';
	BOOST_SPIRIT_DEFINE(group_rule);

	

	x3::rule<class unit, std::unique_ptr<Object>> unit_term;
	auto unit_term_def = variable_rule | group_rule;
	BOOST_SPIRIT_DEFINE(unit_term);

	x3::rule<class tuple_multterm, std::tuple<std::unique_ptr<Object>, std::unique_ptr<Object>>> tuple_mult_term;
	auto tuple_mult_term_def = unit_term >>  '*' >> unit_term;
	BOOST_SPIRIT_DEFINE(tuple_mult_term);

	x3::rule<class multterm, std::unique_ptr<Object>> mult_term;
	auto mult_term_def = tuple_mult_term[([](auto& ctx) { x3::_val(ctx) = objectsIntersection(
		std::move(std::get<0>(x3::_attr(ctx))),
		std::move(std::get<1>(x3::_attr(ctx))),
		{ 0,0,0 }, { 1,0,0,0 }
	); })];
	BOOST_SPIRIT_DEFINE(mult_term);

	x3::rule<class tuple_subterm, std::tuple<std::unique_ptr<Object>, std::unique_ptr<Object>>> tuple_sub_term;
	auto tuple_sub_term_def = unit_term >> '\\' >> unit_term;
	BOOST_SPIRIT_DEFINE(tuple_sub_term);

	x3::rule<class subterm, std::unique_ptr<Object>> sub_term;
	auto sub_term_def = tuple_sub_term[([](auto& ctx) { x3::_val(ctx) = objectsSubtraction(
		std::move(std::get<0>(x3::_attr(ctx))),
		std::move(std::get<1>(x3::_attr(ctx))),
		{ 0,0,0 }, { 1,0,0,0 }
	); })];
	BOOST_SPIRIT_DEFINE(sub_term);

	x3::rule<class monomrule, std::unique_ptr<Object>> monom;
	auto monom_def = mult_term | sub_term | unit_term;
	BOOST_SPIRIT_DEFINE(monom);



	x3::rule<class tuple_addterm, std::tuple<std::unique_ptr<Object>, std::unique_ptr<Object>>> tuple_add_term;
	auto tuple_add_term_def = monom >> '+' >> monom;
	BOOST_SPIRIT_DEFINE(tuple_add_term);

	x3::rule<class addterm, std::unique_ptr<Object>> add_term;
	auto add_term_def = tuple_add_term[([](auto& ctx) { 

		x3::_val(ctx) = objectsUnion(
		std::move(std::get<0>(x3::_attr(ctx))),
		std::move(std::get<1>(x3::_attr(ctx))),
		{ 0,0,0 }, { 1,0,0,0 }
	); })];
	BOOST_SPIRIT_DEFINE(add_term);


	auto expression_rule_def = add_term | monom;
	BOOST_SPIRIT_DEFINE(expression_rule);


	x3::rule<class fin, std::unique_ptr<Object>> final_rule;
	auto final_rule_def = +assignation_rule >> x3::lit("__obj__") >> '=' >> expression_rule >> x3::eoi;
	BOOST_SPIRIT_DEFINE(final_rule);


	std::string toString(const Vector<2>& v)
	{
		return "{" + std::to_string(v.x()) + ", " + std::to_string(v.y()) + "}";
	}
	std::string toString(const Vector<3>& v)
	{
		return "{" + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", " + std::to_string(v.z()) + "}";
	}
	std::string toString(const Vector<4>& v)
	{
		return "{" + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", " + std::to_string(v.z()) + ", " + std::to_string(v.t()) + "}";
	}
	std::string toString(const Quat& q)
	{
		return std::to_string(q.a0()) + " + " + toString(q.a);
	}
	std::string standartString(const Object& obj)
	{
		return "; position= " + toString(obj.getPosition()) + "; rotation= " + toString(obj.getRotation()) + "; color= " + toString(Vector<3>(obj.getColor().x(), obj.getColor().y(), obj.getColor().z()))
			+ "; alpha= " + std::to_string(obj.getColor().t()) + "; id= " + std::to_string(obj.getId())
			+ (obj.haveBoundingBox() ? "; bb_hsize= " + toString(obj.getBoundingBox()) + "; bb_position= " + toString(obj.getBoundingBoxPosition()) : "") + ")";
	}
	std::string toString(const std::vector<Vector<2>>& v)
	{
		std::string res = "{";
		for (int i = 0; i < v.size(); ++i)
		{
			res += toString(v[i]) + (i < v.size() - 1 ? ",\n" : "\n");
		}
		return res + "}";
	}
	std::string toString(const std::vector<Vector<3>>& v)
	{
		std::string res = "{";
		for (int i = 0; i < v.size(); ++i)
		{
			res += toString(v[i]) + (i < v.size() - 1 ? ",\n" : "\n");
		}
		return res + "}";
	}
	std::string toString(const std::vector<int>& v)
	{
		std::string res = "{";
		for (int i = 0; i < v.size(); ++i)
		{
			res += std::to_string(v[i]) + (i < v.size() - 1 ? "," : "");
		}
		return res + "}";
	}
	std::string toString(const std::vector<std::vector<int>>& v)
	{
		std::string res = "{";
		for (int i = 0; i < v.size(); ++i)
		{
			res += toString(v[i]) + (i < v.size() - 1 ? "," : "");
		}
		return res + "}";
	}
	std::string toString(ComposedObject::Operation op)
	{
		switch (op)
		{
		case ComposedObject::PLUS:
			return "+";
		case ComposedObject::MINUS:
			return "\\";
		case ComposedObject::MULT:
			return "*";
		default:
			assert(false);
		}
		return "";
	}
	std::string toStringObject(const Object& obj)
	{
		switch (obj.getType())
		{
		case ObjectType::BOX:
		{ const Box* box_p = dynamic_cast<const Box*>(&obj);
		return "Box(hsize= " + toString(box_p->getHsize()) + standartString(obj);
		}
		case ObjectType::CONE: {
			const Cone* cone_p = dynamic_cast<const Cone*>(&obj);
			return "Cone(height= " + std::to_string(cone_p->getHeight()) + "; radius= " + std::to_string(cone_p->getRadius()) + standartString(obj);
		}
		case ObjectType::CYLINDER: {
								 const Cylinder* cyl_p = dynamic_cast<const Cylinder*>(&obj);
								 return "Cylinder(height= " + std::to_string(cyl_p->getHalfHeight() * 2) + "; radius= " + std::to_string(cyl_p->getRadius()) + standartString(obj);
		}
		case ObjectType::PIRAMID: {
			const Piramid* pir_p = dynamic_cast<const Piramid*> (&obj);
			return "Piramid(height=" + std::to_string(pir_p->getHeight()) + "; base= " + toString(pir_p->getBase()) + standartString(obj);
		}
		case ObjectType::POLYHEDRON: {
			const Polyhedron* pol_p = dynamic_cast<const Polyhedron*>(&obj);
			return "Polyhedron(points: " + toString(pol_p->getPoints()) + ";\n edges: " + toString(pol_p->getEdges()) + standartString(obj);
		}
		case ObjectType::PRIZM: {
			const Prizm* pr_p = dynamic_cast<const Prizm*>(&obj);
			return "Prizm(height= " + std::to_string(pr_p->getHalfHeight() * 2) + "; base= " + toString(pr_p->getBase()) + standartString(obj);
		}
		case ObjectType::SPHERE: {
			const Sphere* sp_p = dynamic_cast<const Sphere*>(&obj);
			return "Sphere(radius= " + std::to_string(sp_p->getRadius()) + standartString(obj);
		}
		case ObjectType::COMPOSED_OBJECT: {
			const ComposedObject* c_p = dynamic_cast<const ComposedObject*>(&obj);
			return "Obj(O" + std::to_string(c_p->getLeft()->getId()) + " " + toString(c_p->getOperation()) + " O" + std::to_string(c_p->getRight()->getId()) + "; id=" + std::to_string(c_p->getId()) +
				(obj.haveBoundingBox() ? "; bb_hsize= " + toString(obj.getBoundingBox()) + "; bb_position= " + toString(obj.getBoundingBoxPosition()) : "") + ")";
		}
		default: assert(false);
		}
	}
	std::string traverseAndDisplay(const Object& obj)
	{
		std::string res = "";
		if (obj.getType() == ObjectType::COMPOSED_OBJECT)
		{
			const ComposedObject* c_p = dynamic_cast<const ComposedObject*>(&obj);
			res += traverseAndDisplay(*c_p->getLeft());
			res += traverseAndDisplay(*c_p->getRight());
		}
		res += "O" + std::to_string(obj.getId()) + " = " + toStringObject(obj) + "\n";
		return res;
	}
	
}
#include <stack>
std::string toStringScene(const Object& obj)
{
	std::string res = "";
	std::stack<std::pair<const Object*, int>> st;
	st.push({ &obj, 0 });
	while (!st.empty())
	{
		if (st.top().first->getType() == ObjectType::COMPOSED_OBJECT)
		{
			if (st.top().second == 0)
			{
				st.top().second = 1;
				st.push({ ((const ComposedObject*)(st.top().first))->getLeft().get(), 0 });
			}
			else if (st.top().second == 1)
			{
				st.top().second = 2;
				st.push({ ((const ComposedObject*)(st.top().first))->getRight().get(), 0 });
			}
			else
			{
				res += "O" + std::to_string(st.top().first->getId()) + " = " + Parser::toStringObject(*st.top().first) + "\n";
				st.pop();
			}
		}
		else
		{
			res += "O" + std::to_string(st.top().first->getId()) + " = " + Parser::toStringObject(*st.top().first) + "\n";
			st.pop();
		}
	}
	return res + "\n__obj__ = O" + std::to_string(obj.getId());
}

std::unique_ptr<Object> parse(const std::string & str)
{
	std::unique_ptr<Object> res;

	Parser::__vars_map.clear();
	std::string::const_iterator begin = str.begin();
	try
	{
		boost::spirit::x3::phrase_parse(begin, str.end(), Parser::final_rule, x3::space, res);
	}
	catch (std::string mess)
	{
		std::cout << mess << '\n';
		system("pause");
	}
	
	if (begin != str.end())
		return nullptr;
	return res;
}