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
namespace Parser
{
	x3::rule<class vec2, Vector<2>> vec2= "Vector<2>";
	auto vec2_setx = [](auto& ctx) { x3::_val(ctx).nums[0] = x3::_attr(ctx); };
	auto vec2_sety = [](auto& ctx) { x3::_val(ctx).nums[1] = x3::_attr(ctx); };
	auto vec3_setz = [](auto& ctx) { x3::_val(ctx).nums[2] = x3::_attr(ctx); };
	auto vec4_setw = [](auto& ctx) { x3::_val(ctx).nums[3] = x3::_attr(ctx); };
	auto const vec2_def = ('{' >> x3::double_ [vec2_setx] >> ',' >> x3::double_[vec2_sety] >> '}');
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

	
	x3::rule<class tuplebox, std::tuple<Vector<3>, Vector<3>, Quat>> box_tuple_rule = "tuple_Box";
	auto box_tuple_rule_def = x3::lit("Box") >> '(' >> x3::lit("hsize") >> ':' >> vec3 >> ';' >> x3::lit("position") >> ':' >> vec3 >> ';' >>
		x3::lit("rotation") >> ':' >> quat_rule >> ')';
	BOOST_SPIRIT_DEFINE(box_tuple_rule);

	x3::rule<class boxrule, std::unique_ptr<Object>> box_rule = "Box";
	auto box_rule_def = box_tuple_rule[([](auto& ctx) { x3::_val(ctx) = std::apply(makeBox, x3::_attr(ctx)); })];
	BOOST_SPIRIT_DEFINE(box_rule);


	x3::rule<class tupleprizm, std::tuple<std::vector<Vector<2>>, double, Vector<3>, Quat>> prizm_tuple_rule = "tuple_Prizm";
	auto prizm_tuple_rule_def = x3::lit("Prizm") >> '(' >> x3::lit("base") >> ':' >> vector_vec2 >> ';' >> x3::lit("height") >> ':' >>
		x3::double_ >> ';' >> x3::lit("position") >> ':' >> vec3 >> ';' >> x3::lit("rotation") >> ':' >> quat_rule >> ')';
	BOOST_SPIRIT_DEFINE(prizm_tuple_rule);

	x3::rule<class prizmrule, std::unique_ptr<Object>> prizm_rule = "Prizm";
	auto prizm_rule_def = prizm_tuple_rule[([](auto& ctx) { x3::_val(ctx) = std::apply(makePrizm, x3::_attr(ctx)); })];
	BOOST_SPIRIT_DEFINE(prizm_rule);

	x3::rule<class tuplecone, std::tuple<double, double, Vector<3>, Quat>> cone_tuple_rule = "tuple_Cone";
	auto cone_tuple_rule_def = x3::lit("Cone") >> '(' >> x3::lit("height") >> ':' >> x3::double_ >> ';' >> x3::lit("radius") >> ':' >>
		x3::double_ >> ';' >> x3::lit("position") >> ':' >> vec3 >> ';' >> x3::lit("rotation") >> ':' >> quat_rule >> ')';
	BOOST_SPIRIT_DEFINE(cone_tuple_rule);

	x3::rule<class conerule, std::unique_ptr<Object>> cone_rule = "Cone";
	auto cone_rule_def = cone_tuple_rule[([](auto& ctx) {
		x3::_val(ctx) = std::apply(makeCone, x3::_attr(ctx));
		})];
	BOOST_SPIRIT_DEFINE(cone_rule);

#define PROP(name, rule) (x3::lit(#name) >> ':' >> rule >> ';')
#define END_PROP(name, rule) (x3::lit(#name) >> ':' >> rule)
	x3::rule<class tuplepiramid, std::tuple<std::vector<Vector<2>>, double, Vector<3>, Quat>> piramid_tuple_rule = "tuple_Piramid";
	auto piramid_tuple_rule_def = x3::lit("Piramid") >> '(' >>
		PROP(base, vector_vec2) >>
		PROP(height, x3::double_) >>
		PROP(position, vec3) >>
		END_PROP(rotation, quat_rule) >> ')';
	BOOST_SPIRIT_DEFINE(piramid_tuple_rule);

	x3::rule<class piramidrule, std::unique_ptr<Object>> piramid_rule = "Piramid";
	auto piramid_rule_def = piramid_tuple_rule[([](auto& ctx) { x3::_val(ctx) = std::apply(makePiramid, x3::_attr(ctx)); })];
	BOOST_SPIRIT_DEFINE(piramid_rule);

	x3::rule<class tuplecylinder, std::tuple<double, double, Vector<3>, Quat>> tuple_cylinder_rule = "tuple_Cylinder";
	auto tuple_cylinder_rule_def = x3::lit("Cylinder") >> '(' >>
		PROP(height, x3::double_) >>
		PROP(radius, x3::double_) >>
		PROP(position, vec3) >>
		END_PROP(rotation, quat_rule) >> ')';

#define DEFINE_RULE(prim, func) x3::rule<class prim##rule, std::unique_ptr<Object>> prim##_rule = #prim; \
	auto prim##_rule_def = tuple_##prim##_rule[([](auto& ctx) { x3::_val(ctx) = std::apply(func, x3::_attr(ctx)); })];\
	BOOST_SPIRIT_DEFINE(tuple_##prim##_rule)\
	BOOST_SPIRIT_DEFINE(prim##_rule);

	DEFINE_RULE(cylinder, makeCylinder);

	x3::rule<class tuplesphere, std::tuple<double, Vector<3>>> tuple_sphere_rule = "tuple_Sphere";
	auto tuple_sphere_rule_def = x3::lit("Sphere") >> '(' >>
		PROP(radius, x3::double_) >>
		END_PROP(position, vec3) >> ')';

	DEFINE_RULE(sphere, makeSphere);
	
	x3::rule<class tuplepoly, std::tuple< std::vector<Vector<3>>, std::vector<std::vector<int>>, Vector<3>, Quat> > tuple_polyhedron_rule = "tuple_Polyhedron";
	auto tuple_polyhedron_rule_def = x3::lit("Polyhedron") >> '(' >>
		PROP(points, vector_vec3) >>
		PROP(edges, vec_vec_int) >>
		PROP(position, vec3) >>
		END_PROP(rotation, quat_rule) >> ')';

	DEFINE_RULE(polyhedron, makePolyhedron);

	x3::rule<class primitive, std::unique_ptr<Object>> primitive_rule;
	auto primitive_rule_def = box_rule | prizm_rule | cone_rule | piramid_rule | cylinder_rule | sphere_rule | polyhedron_rule;
	BOOST_SPIRIT_DEFINE(primitive_rule);
	//ƒŒÀ∆Õ¿ Œ◊»Ÿ¿“‹—ﬂ œ≈–≈ƒ Ô‡–—»ÕŒ√ŒÃ
	std::map<std::string, std::unique_ptr<Object>> __vars_map;

	x3::rule<class word, std::string> liter_rule;
	auto liter_rule_def = x3::alpha >> *(x3::alnum) ;
	BOOST_SPIRIT_DEFINE(liter_rule);

	x3::rule<class assign_tuple, std::tuple<std::string, std::unique_ptr<Object>>> tuple_assignation_rule;
	auto tuple_assignation_rule_def = liter_rule >> '=' >> primitive_rule;
	void assignVariable(const std::string& str, std::unique_ptr<Object>&& prim) { __vars_map.insert({ str, std::move(prim) }); }
	BOOST_SPIRIT_DEFINE(tuple_assignation_rule);

	x3::rule<class assign, x3::unused_type> assignation_rule;
	auto assignation_rule_def = tuple_assignation_rule[([](auto& ctx)
		{ assignVariable(std::get<0>(x3::_attr(ctx)), std::move(std::get<1>(x3::_attr(ctx)))); })];
	BOOST_SPIRIT_DEFINE(assignation_rule);

	x3::rule<class var, std::unique_ptr<Object>> variable_rule;
	auto variable_rule_def = liter_rule[([](auto& ctx) { x3::_val(ctx) = __vars_map.find(x3::_attr(ctx))->second->copy(); })];
	BOOST_SPIRIT_DEFINE(variable_rule);




	x3::rule<class expr, std::unique_ptr<Object>> expression_rule;

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



}


std::unique_ptr<Object> parse(const std::string & str)
{
	std::unique_ptr<Object> res;

	Parser::__vars_map.clear();
	std::string::const_iterator begin = str.begin();
	boost::spirit::x3::phrase_parse(begin, str.end(), Parser::final_rule, x3::space, res);
	if (begin != str.end())
		return nullptr;
	return res;
}