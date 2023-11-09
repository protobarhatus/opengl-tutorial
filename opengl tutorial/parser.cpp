#include <boost/spirit/home/x3.hpp>
#include <string>
#include "ComposedObject.h"
#include "parser.h"
using namespace boost::spirit;

namespace Parser
{
	x3::rule<class vec2, Vector<2>> vec2= "Vector<2>";
	auto vec2_setx = [](auto& ctx) { x3::_val(ctx).nums[0] = x3::_attr(ctx); };
	auto vec2_sety = [](auto& ctx) { x3::_val(ctx).nums[1] = x3::_attr(ctx); };
	auto const vec2_def = ('{' >> x3::double_ [vec2_setx] >> ',' >> x3::double_[vec2_sety] >> '}');

	BOOST_SPIRIT_DEFINE(vec2);

	x3::rule<class vectorvec2, std::vector<Vector<2>>> vector_vec2 = "std::vector<Vector<2>>";
	auto vector_vec2_def = '{' >> vec2 % ',' >> '}';
	BOOST_SPIRIT_DEFINE(vector_vec2);
}

std::vector<Vector<2>> parse(const std::string & str)
{

	//boost::spirit::x3::phrase_parse(str.begin(), str.end(), str, str);

	x3::double_type asd = x3::double_;
	std::vector<Vector<2>> res;

	boost::spirit::x3::phrase_parse(str.begin(), str.end(), Parser::vector_vec2, x3::space, res);

	return res;
}