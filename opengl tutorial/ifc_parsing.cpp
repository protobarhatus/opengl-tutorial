#include <iostream>
#include <cmath>
// necessary includes
#include <boost/preprocessor/seq/for_each.hpp>
#include "ifcparse/Ifc2x3.h"
#include "ifcparse/Ifc4.h"

#include "ifcparse/IfcParse.h"
#include "ifcparse/IfcFile.h"
#include "gl_utils.h"
#include "ifc_parsing.h"
#include "primitives.h"
#include "objects_fabric.h"
#include "ComposedObject.h"

#define IFC_SCHEMA_SEQ (4x3_rc2)(4)(2x3) // TODO: Enumerate through all IFC schemas you want to be able to process
#define EXPAND_AND_CONCATENATE(elem) Ifc##elem
#define PROCESS_FOR_SCHEMA(r, data, elem) if (schema_version == BOOST_PP_STRINGIZE(elem)) { parseIfc<EXPAND_AND_CONCATENATE(elem)>(file); } else

/*template<class Schema>
void parseIfc(IfcParse::IfcFile& file) {
	const typename Schema::IfcProduct::list::ptr elements = file.instances_by_type<typename Schema::IfcProduct>();
	for (typename Schema::IfcProduct::list::it it = elements->begin();
		it != elements->end(); ++it) {
		Schema::IfcProduct* ifcProduct = *it;
		// TODO: Do something with ifcProduct
	}
}*/
void parseIfc(IfcParse::IfcFile& file) {
	const typename Ifc2x3::IfcProduct::list::ptr elements = file.instances_by_type<typename Ifc2x3::IfcProduct>();
	for (typename Ifc2x3::IfcProduct::list::it it = elements->begin();
		it != elements->end(); ++it) {
		Ifc2x3::IfcProduct* ifcProduct = *it;
		// TODO: Do something with ifcProduct
		//std::cout << ifcProduct->Name().value() << "\n";
	}
}

template<typename Schema>
void extractPropertySets(const typename Schema::IfcObject& obj,
	typename std::vector<typename Schema::IfcPropertySet*>& property_sets) {
	const auto& is_defined_by = obj.IsDefinedBy();
	if (is_defined_by == nullptr)
		return;
	std::vector<typename Schema::IfcRelDefines*> rels(is_defined_by->size());
	typename std::vector<typename Schema::IfcRelDefines*>::iterator it = std::copy_if(
		is_defined_by->begin(), is_defined_by->end(), rels.begin(),
		[](const typename Schema::IfcRelDefines* x) {
			if (x == nullptr) return false;
			const typename Schema::IfcRelDefinesByProperties* defines_by_properties =
				x->template as<typename Schema::IfcRelDefinesByProperties>();
			if (defines_by_properties == nullptr) return false;
			const auto* relating_property_definition =
				defines_by_properties->RelatingPropertyDefinition();
			if (relating_property_definition == nullptr) return false;
			return relating_property_definition->template as<typename Schema::IfcPropertySet>() !=
				nullptr;
		});
	rels.resize(std::distance(rels.begin(), it));
	property_sets.resize(rels.size());
	std::transform(rels.begin(), rels.end(), property_sets.begin(),
		[](const typename Schema::IfcRelDefines* x) {
			const typename Schema::IfcRelDefinesByProperties* defines_by_properties = x->template as<typename Schema::IfcRelDefinesByProperties>();
			return defines_by_properties->RelatingPropertyDefinition()->template as<typename Schema::IfcPropertySet>();
		});
}
void CheckGeometricRepresentation(Ifc4::IfcGeometricRepresentationItem* item) {
	return;
	if (!item) {
		std::cout << "Invalid item or nullptr passed.\n";
		return;
	}

	if (dynamic_cast<Ifc4::IfcExtrudedAreaSolid*>(item)) {
		std::cout << "The item is of type IfcExtrudedAreaSolid.\n";
	}
	else if (dynamic_cast<Ifc4::IfcPolyline*>(item)) {
		std::cout << "The item is of type IfcPolyline.\n";
	}
	else if (dynamic_cast<Ifc4::IfcFacetedBrep*>(item)) {
		std::cout << "The item is of type IfcFacetedBrep.\n";
	}
	else if (dynamic_cast<Ifc4::IfcSurfaceCurve*>(item)) {
		std::cout << "The item is of type IfcSurfaceCurve.\n";
	}
	else if (dynamic_cast<Ifc4::IfcCompositeCurve*>(item)) {
		std::cout << "The item is of type IfcCompositeCurve.\n";
	}
	else if (dynamic_cast<Ifc4::IfcCircle*>(item)) {
		std::cout << "The item is of type IfcCircle.\n";
	}
	else if (dynamic_cast<Ifc4::IfcPolygonalFaceSet*>(item)) {
		std::cout << "The item is of type IfcPolygonalFaceSet.\n";
	}
	else if (dynamic_cast<Ifc4::IfcTextLiteral*>(item)) {
		std::cout << "The item is of type IfcTextLiteral.\n";
	}
	else if (dynamic_cast<Ifc4::IfcPoint*>(item)) {
		std::cout << "The item is of type IfcPoint.\n";
	}
	else if (dynamic_cast<Ifc4::IfcLine*>(item)) {
		std::cout << "The item is of type IfcLine.\n";
	}
	else if (dynamic_cast<Ifc4::IfcMappedItem*>(item)) {
		std::cout << "The item is of type IfcMappedItem.\n";
	}
	// Add more checks for additional subclasses as needed
	else {
		std::cout << "The item is of an unknown or unsupported type.\n";
	}
}

template<class Schema>
typename Schema::IfcRepresentationItem* unmapMappedRepresentation(typename Schema::IfcMappedItem* item)
{
	auto list = item->MappingSource()->MappedRepresentation()->Items();
	for (auto& repr : *list)
		if (repr->as<typename Schema::IfcGeometricRepresentationItem>() || repr->as<typename Schema::IfcMappedItem>())
			return repr;
	return nullptr;
}



template<class Schema>
typename Schema::IfcGeometricRepresentationItem* getToGeometry(typename Schema::IfcProductRepresentation* repr)
{
	auto representation = repr->Representations();
	for (auto& it : *representation)
	{
		
		if (it->as<typename Schema::IfcShapeModel>())
		{
			if (it->RepresentationIdentifier().value() != "Body")
				continue;
			if (it->as<Schema::IfcShapeModel>()->as< Schema::IfcShapeRepresentation>())
			{
				auto shape = it->as<Schema::IfcShapeModel>()->as< Schema::IfcShapeRepresentation>();
				auto items = shape->Items();
				for (auto& it1 : *items)
				{
					Ifc4::IfcRepresentationItem* repr = it1;
					while (repr->as<typename Schema::IfcMappedItem>())
						repr = unmapMappedRepresentation<Schema>(repr->as<typename Schema::IfcMappedItem>());

					if (repr)
					{
						return repr->as<typename Schema::IfcGeometricRepresentationItem>();
						/*std::cout << repr->as<Ifc4::IfcGeometricRepresentationItem>()->Class().is_abstract();
						CheckGeometricRepresentation(repr->as<Ifc4::IfcGeometricRepresentationItem>());
						std::cout << "GEOM\n";
						if (repr->as< Ifc4::IfcBoundingBox>())
						{
							auto wall_bb = it1->as< Ifc4::IfcBoundingBox>();
							auto corner = wall_bb->Corner()->Coordinates();
							std::cout << "Corner: " << corner[0] << ", " << corner[1] << ", " << corner[2] << '\n';
							std::cout << "Dimensions: " << wall_bb->XDim() << ", " << wall_bb->YDim() << ", " << wall_bb->ZDim() << '\n';
						}

						if (repr->as < Ifc4::IfcTessellatedItem  >())
						{
							std::cout << "Ifc4::IfcTesselatedModel\n";
						}*/

					}
				}
			}
		}
	}
	return nullptr;
}

template<int dim>
Vector<dim> readPoint(Ifc4::IfcCartesianPoint* point);
template<>
Vector<2> readPoint(Ifc4::IfcCartesianPoint* point)
{
	auto vec = point->Coordinates();
	return { vec[0], vec[1] };
}
template<>
Vector<3> readPoint(Ifc4::IfcCartesianPoint* point)
{
	auto vec = point->Coordinates();
	return { vec[0], vec[1], vec[2] };
}
Vector<3> getPoint(Ifc4::IfcAxis2Placement3D* axis)
{
	auto point = axis->Location()->Coordinates();
	return { point[0], point[1], point[2] };
}
Vector<3> getPoint(Ifc4::IfcAxis2Placement2D* axis)
{
	auto point = axis->Location()->Coordinates();
	return { point[0], point[1], 0 };
}
Matrix<3> getRotation(Ifc4::IfcAxis2Placement3D* relativePlacement)
{
	Matrix<3> this_shift_mat = diagonal<3>(1);

	if (relativePlacement->RefDirection())
	{
		auto refdir = relativePlacement->RefDirection()->DirectionRatios();
		auto zdir = relativePlacement->Axis()->DirectionRatios();
		auto ydir = normalize(cross({ zdir[0], zdir[1], zdir[2] }, { refdir[0], refdir[1], refdir[2] }));
		auto xdir = cross(ydir, { zdir[0], zdir[1], zdir[2] });
		//std::cout << "X dir: (" << xdir[0] << ", " << xdir[1] << ", " << xdir[2] << ")\n";
		std::vector<std::vector<double>> mat = { {xdir[0], ydir[0], zdir[0]}, {xdir[1], ydir[1], zdir[1]}, {xdir[2], ydir[2], zdir[2]} };
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				this_shift_mat[i][j] = mat[i][j];

		//rotation = rotationMatrixToQuaternion(mat);
	}
	return this_shift_mat;
}
Matrix<3> getRotation(Ifc4::IfcAxis2Placement2D* relativePlacement)
{
	Matrix<3> this_shift_mat = diagonal<3>(1);

	if (relativePlacement->RefDirection())
	{
		auto refdir = relativePlacement->RefDirection()->DirectionRatios();
		auto zdir = Vector<3>{ 0,0,1 };
		auto ydir = normalize(cross({ zdir[0], zdir[1], zdir[2] }, { refdir[0], refdir[1], 0 }));
		auto xdir = cross(ydir, { zdir[0], zdir[1], zdir[2] });
		//std::cout << "X dir: (" << xdir[0] << ", " << xdir[1] << ", " << xdir[2] << ")\n";
		std::vector<std::vector<double>> mat = { {xdir[0], ydir[0], zdir[0]}, {xdir[1], ydir[1], zdir[1]}, {xdir[2], ydir[2], zdir[2]} };
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				this_shift_mat[i][j] = mat[i][j];

		//rotation = rotationMatrixToQuaternion(mat);
	}
	return this_shift_mat;
}




std::vector<Vector<2>> healLine(const std::vector<Vector<2>>& line)
{
	std::vector<Vector<2>> res;
	for (int i = 0; i < line.size(); ++i)
	{
		Vector<2> a = line[(i + 1) % line.size()] - line[i];
		Vector<2> b = line[(i + line.size() - 1) % line.size()] - line[i];
		if (dot(a, b) / length(a)/length(b) < -0.99)
			continue;
		res.push_back(line[i]);
	}
	return res;
}

std::vector<Vector<2>> readPolyline(Ifc4::IfcPolyline* line)
{
	auto points = line->Points();
	std::vector<Vector<2>> res;
	for (auto& it : *points)
		res.push_back(readPoint<2>(it));
	res = std::vector<Vector<2>>(res.begin(), std::prev(res.end()));

	return healLine(res);
}

bool orthogonal(const Vector<2>& a, const Vector<2>& b)
{
	return abs(dot(a, b)) < 0.08 * length(a)*length(b);
}
bool isLineRect(const std::vector<Vector<2>>& line)
{
	if (line.size() != 4)
		return false;
	return orthogonal(line[3] - line[0], line[1] - line[0]) && orthogonal(line[2] - line[1], line[0] - line[1]) && orthogonal(line[3] - line[2], line[1] - line[2]) && orthogonal(line[0] - line[3], line[2] - line[3]);
}

Vector<2> curveCenter(const std::vector<Vector<2>>& line)
{
	Vector<2> cen = { 0,0 };
	for (auto& it : line)
		cen += it;
	return cen / line.size();
}

template<typename MatrixT>
Quat rotationMatrixToQuaternion(const MatrixT& R);
std::unique_ptr<Object> getPrimitiveFromExtrusion(Ifc4::IfcExtrudedAreaSolid* solid, const Vector<3>& position, const Matrix<3>& rotation_mat)
{
	auto dir = solid->ExtrudedDirection()->DirectionRatios();
	Vector<3> direction = { dir[0], dir[1], dir[2] };

	auto area = solid->SweptArea();

	if (area->as<Ifc4::IfcRectangleProfileDef>())
	{
		auto area_shift = getPoint(area->as<Ifc4::IfcRectangleProfileDef>()->Position());
		auto area_rotation = getRotation(area->as<Ifc4::IfcRectangleProfileDef>()->Position());
		auto rotation = rotationMatrixToQuaternion( rotation_mat*area_rotation );
		return makeBox(Vector<3>{ area->as<Ifc4::IfcRectangleProfileDef>()->XDim(), area->as<Ifc4::IfcRectangleProfileDef>()->YDim(), solid->Depth() }*0.5,
			position + rotation_mat * Vector<3>{area_shift.x(), area_shift.y(), solid->Depth() / 2},
			rotation);
	}
	if (area->as<Ifc4::IfcCircleProfileDef>())
	{
		auto rotation = rotationMatrixToQuaternion(rotation_mat);
		return makeCylinder(solid->Depth(), area->as<Ifc4::IfcCircleProfileDef>()->Radius(), position + rotation_mat * Vector<3>{0, 0, solid->Depth() / 2}, rotation);
	}

	if (area->as<Ifc4::IfcArbitraryClosedProfileDef>())
	{

		
		auto curve = area->as<Ifc4::IfcArbitraryClosedProfileDef>()->OuterCurve();
		if (curve->as<Ifc4::IfcPolyline>())
		{
			auto base = readPolyline(curve->as<Ifc4::IfcPolyline>());
			Vector<2> cent = curveCenter(base);
			for (auto& it : base)
				it = it - cent;
			if (isLineRect(base))
			{

				Vector<2> xdir = normalize(base[3] - base[0]);
				Vector<2> ydir = -cross(Vector<3>(xdir,0), { 0,0,1 });
				Matrix<3> rot_to_box = { {xdir.x(), ydir.x(), 0}, {xdir.y(), ydir.y(), 0}, {0, 0, 1} };
				auto rotation = rotationMatrixToQuaternion(rotation_mat * rot_to_box);
				return makeBox(Vector<3>(length(base[0] - base[3]), length(base[1] - base[0]), solid->Depth()) * 0.5, position + rotation_mat * Vector<3>{ cent.x(), cent.y(), solid->Depth() / 2 },
					rotation);
			}
			auto rotation = rotationMatrixToQuaternion(rotation_mat);
			/*auto bases_split = splitPolygonIntoConvexParts(base);

			std::vector<std::unique_ptr<Object>> prizms_vec;
			for (auto& it : bases_split)
			{
				//Vector<2> cent1 = curveCenter(it);
				//for (auto& it1 : it)
				//	it1 = it1 - cent;
				prizms_vec.push_back(makePrizm(it, solid->Depth(), position + rotation_mat * Vector<3>{ cent.x(), cent.y(), solid->Depth() / 2 }, rotation));
			}*/
			auto obj = makePrizm(base, solid->Depth(), position + rotation_mat* Vector<3>{ cent.x(), cent.y(), solid->Depth() / 2 }, rotation);
			return std::move(obj);
			//return makeAnHierarchy(std::move(prizms_vec));
		}
	}
	return nullptr;
}

template<typename MatrixT>
Quat rotationMatrixToQuaternion(const MatrixT& R) {
	Quat q;
	
	double trace = R[0][0] + R[1][1] + R[2][2];

	if (trace > 0.0) {
		double s = std::sqrt(trace + 1.0) * 2.0; // s = 4 * q.w
		q._a0 = 0.25 * s;
		q.a.nums[0] = (R[2][1] - R[1][2]) / s;
		q.a.nums[1] = (R[0][2] - R[2][0]) / s;
		q.a.nums[2] = (R[1][0] - R[0][1]) / s;
	}
	else if ((R[0][0] > R[1][1]) && (R[0][0] > R[2][2])) {
		double s = std::sqrt(1.0 + R[0][0] - R[1][1] - R[2][2]) * 2.0; // s = 4 * q.x
		q._a0 = (R[2][1] - R[1][2]) / s;
		q.a.nums[0] = 0.25 * s;
		q.a.nums[1] = (R[0][1] + R[1][0]) / s;
		q.a.nums[2] = (R[0][2] + R[2][0]) / s;
	}
	else if (R[1][1] > R[2][2]) {
		double s = std::sqrt(1.0 + R[1][1] - R[0][0] - R[2][2]) * 2.0; // s = 4 * q.y
		q._a0 = (R[0][2] - R[2][0]) / s;
		q.a.nums[0] = (R[0][1] + R[1][0]) / s;
		q.a.nums[1] = 0.25 * s;
		q.a.nums[2] = (R[1][2] + R[2][1]) / s;
	}
	else {
		double s = std::sqrt(1.0 + R[2][2] - R[0][0] - R[1][1]) * 2.0; // s = 4 * q.z
		q._a0 = (R[1][0] - R[0][1]) / s;
		q.a.nums[0] = (R[0][2] + R[2][0]) / s;
		q.a.nums[1] = (R[1][2] + R[2][1]) / s;
		q.a.nums[2] = 0.25 * s;
	}

	return q;
}


#include <stack>
std::pair<Matrix<3>, Vector<3>> uncoverPlacement(Ifc4::IfcObjectPlacement* placement, Matrix<3> rotation, Vector<3> position)
{
	//Matrix<3> rotation = diagonal<3>(1);
	//Vector<3> position = { 0,0,0 };

	do
	{
		if (placement->as<Ifc4::IfcLocalPlacement>())
		{
			auto relativePlacement = placement->as<Ifc4::IfcLocalPlacement>()->RelativePlacement()->as<Ifc4::IfcAxis2Placement3D>();
			auto point = relativePlacement->Location()->Coordinates();
			//std::cout << "Wall Position: (" << point[0] << ", " << point[1] << ", " << point[2] << ")\n";
			auto this_shift_mat = getRotation(relativePlacement);
			position = this_shift_mat * position + Vector<3>{point[0], point[1], point[2]};
			//rotation = rotation * this_shift_mat;
			rotation =this_shift_mat  * rotation;
			placement = placement->as<Ifc4::IfcLocalPlacement>()->PlacementRelTo();
		}
		else
		{
			std::cout << "REL PALCEMENT\n";
			system("pause");
		}
	}while (placement != nullptr);
	/*std::stack< Ifc4::IfcObjectPlacement*> stack;
	do
	{
		if (placement->as<Ifc4::IfcLocalPlacement>())
		{
			stack.push(placement);
			placement = placement->as<Ifc4::IfcLocalPlacement>()->PlacementRelTo();
		}
		else
		{
			std::cout << "REL PALCEMENT\n";
			system("pause");
		}
	} while (placement != nullptr);
	Vector<3> point = { 0,0,0 };
	Matrix<3> rot = diagonal<3>(1);

	while (stack.size() > 0)
	{
		auto relativePlacement = stack.top()->as<Ifc4::IfcLocalPlacement>()->RelativePlacement()->as<Ifc4::IfcAxis2Placement3D>();
		auto this_shift_mat = getRotation(relativePlacement);

		point += rot * getPoint(relativePlacement);
		rot = rot* this_shift_mat;
		
		stack.pop();
	}
	rotation = rot * rotation;
	position = rotation * position + point;*/

	return { rotation, position };
}
Vector<3> readColor(Ifc4::IfcGeometricRepresentationItem* repr)
{
	auto styleitem = repr->StyledByItem();
	for (auto& st : *styleitem)
	{
		auto styles = st->Styles();
		for (auto& it : *styles)
		{
			auto layer_style_assign = dynamic_cast<Ifc4::IfcPresentationStyleAssignment*>(it);
			if (layer_style_assign)
			{
				auto layer_styles = layer_style_assign->Styles();
				for (auto& it1 : *layer_styles)
				{
					auto surface_style = dynamic_cast<Ifc4::IfcSurfaceStyle*>(it1);
					if (surface_style)
					{
						auto surface_styles_rendering = surface_style->Styles();
						for (auto& it2 : *surface_styles_rendering)
						{
							auto rend_style = dynamic_cast<Ifc4::IfcSurfaceStyleRendering*>(it2);
							if (rend_style)
							{
								auto color = rend_style->SurfaceColour();
								if (color)
								{
									return { color->Red(), color->Green(), color->Blue() };
								}
							}
						}
					}
				}
			}
		}
	}
	return { 1,1,1 };
}
std::unique_ptr<Object> getObjectFromProduct(Ifc4::IfcElement* wall)
{
	Ifc4::IfcObjectPlacement* placement = wall->ObjectPlacement();
	Vector<3> position;
	Quat rotation = { 1,0,0,0 };

	auto repr = getToGeometry<Ifc4>(wall->Representation());
	if (!repr)
		return nullptr;
	CheckGeometricRepresentation(repr);
	std::unique_ptr<Object> obj = nullptr;
	if (repr->as<Ifc4::IfcExtrudedAreaSolid>())
	{
		
		std::pair<Matrix<3>, Vector<3>> coord;
		if (repr->as<Ifc4::IfcSweptAreaSolid>()->Position() != nullptr)
		{
			coord = uncoverPlacement(placement, getRotation(repr->as<Ifc4::IfcSweptAreaSolid>()->Position()), getPoint(repr->as<Ifc4::IfcSweptAreaSolid>()->Position()));
			//rotation_mat = getRotation(repr->as<Ifc4::IfcSweptAreaSolid>()->Position()) * rotation_mat;
			//position = position + rotation_mat * getPoint(repr->as<Ifc4::IfcSweptAreaSolid>()->Position());
		}
		else
			coord = uncoverPlacement(placement, diagonal<3>(1), { 0,0,0 });
		position = coord.second;
		auto rotation_mat = coord.first;
		//rotation = rotationMatrixToQuaternion(rotation_mat);
		 obj = getPrimitiveFromExtrusion(repr->as<Ifc4::IfcExtrudedAreaSolid>(), position, rotation_mat);
	}
	if (obj)
	{
		auto color = readColor(repr);
		obj->setColor(color);
	}
	
	return obj;
}


Vector<3> readColor(Ifc4::IfcBuildingElement* product)
{
	auto representation = product->Representation()->Representations();
	for (auto& it : *representation)
	{
		auto items = it->Items();
		for (auto& it1 : *items)
		{
			Ifc4::IfcRepresentationItem* repr = it1;
			while (repr->as<typename Ifc4::IfcMappedItem>())
				repr = unmapMappedRepresentation<Ifc4>(repr->as<typename Ifc4::IfcMappedItem>());
			if (repr->as<Ifc4::IfcStyledItem>()) {
				auto styledItem = repr->as<Ifc4::IfcStyledItem>();
				for (auto& styleAssignment : *styledItem->Styles()) {
					auto surfaceStyle = styleAssignment->as<Ifc4::IfcSurfaceStyle>();
					if (surfaceStyle) {
						auto rendering = surfaceStyle->as<Ifc4::IfcSurfaceStyleRendering>();
						if (rendering) {
							auto color = rendering->DiffuseColour();
							auto rgbColor = color->as<Ifc4::IfcColourRgb>();
							if (rgbColor) {
								double red = rgbColor->Red();
								double green = rgbColor->Green();
								double blue = rgbColor->Blue();

								return { red, green, blue };
							}
						}
					}
				}
			}
		}
	}
	return { 1,1,1 };
}


std::vector<std::unique_ptr<Object>> parseIfc4(IfcParse::IfcFile& file) {
	using itertype = Ifc4::IfcElement;
	//using itertype = Ifc4::IfcWall;
	const typename itertype::list::ptr elements = file.instances_by_type<typename itertype>();
	/*Ifc4::IfcSweptAreaSolid;
	auto walls = file.instances_by_type<typename Ifc4::IfcWall>();
	int wc = 0;
	for (typename Ifc4::IfcProduct::list::it it = elements->begin();
		it != elements->end(); ++it)
		wc++;
	std::cout << wc;*/
	int count = 0;

	std::vector<std::unique_ptr<Object>> solids_vec;
	for (typename itertype::list::it it = elements->begin();
		it != elements->end(); ++it) 
	{
		itertype* wall = *it;
		if (wall->GlobalId() == "32fzzHdg17e8sB7N2ML9G5")
		{
			//continue;
			//std::cout << wall->identity() << "\n";
		}
		if (wall->as<Ifc4::IfcOpeningElement>() || wall->as<Ifc4::IfcSpace>())
			continue;
		
		if (!wall->Representation())
			continue;
		auto obj = getObjectFromProduct(wall);
		if (obj == nullptr)
			continue;
		//obj->setColor(readColor(wall));
		if (wall->HasOpenings() != nullptr && wall->HasOpenings()->size() > 0 )
		{
			auto opening_list = wall->HasOpenings();
			std::vector<std::unique_ptr<Object>> openings;
			for (auto& it : *opening_list)
			{
				auto elem = it->RelatedOpeningElement();
				auto sub = getObjectFromProduct(elem);
				
				if (sub)
				{
					sub->setId(count++);
					openings.push_back(std::move(sub));
				}
			}

			if (openings.size() > 0)
			{
				//if (solids_vec.size() == 3)
					//std::cout << "ASDAS\n";
				solids_vec.push_back(objectsSubtraction(std::move(obj), makeAnHierarchy(std::move(openings)), { 0,0,0 }, { 1,0,0,0 }));
				//solids_vec.push_back(objectsSubtraction(std::move(obj), std::move(openings[0]), { 0,0,0 }, { 1,0,0,0 }));
				/*for (auto& it1 : openings)
				{
					it1->setColor({ 1,0,0 });
					solids_vec.push_back(std::move(it1));
				}
				solids_vec.push_back(std::move(obj));*/
			}
			else
				solids_vec.push_back(std::move(obj));
		}
		else
			solids_vec.push_back(std::move(obj));


		solids_vec.back()->propagateId(count);

	}


	//return makeAnHierarchy(std::move(solids_vec));
	return solids_vec;
}
std::vector<std::unique_ptr<Object>> process(const std::string& schema_version, IfcParse::IfcFile& file) {
	// Syntactic sugar for iterating through all IFC schemas and passing them to main processing method
	if (schema_version == "2x3")
	{
		//parseIfc<Ifc2x3>(file);
		parseIfc(file);
	}
	else
	{
		return parseIfc4(file);
	}
	return {};

}

SceneStruct ifc(const std::string& input_file_path) {

	
	auto file_con = readFile(input_file_path);
	std::ifstream stream(input_file_path, std::ios::binary | std::ios::ate);
	if (!stream.is_open())
		throw std::runtime_error("IFC Importer: Failed to open file " + input_file_path + " for reading");

	int length = (int)stream.tellg();
	stream.seekg(0, std::ios::beg);
	IfcParse::IfcFile file(stream, length);
	if (!file.good()) {
		std::cerr << "Unable to parse .ifc file" << std::endl;
		system("pause");
	}
	// file of IfcParse::IfcFile previously defined
	auto schema_version = file.schema()->name();
	schema_version = schema_version.substr(3);
	std::transform(schema_version.begin(), schema_version.end(), schema_version.begin(), [](unsigned char c) { return std::tolower(c); });
	auto vecres = process(schema_version, file);
	SceneStruct res;
	res.type = SceneStruct::Type::VECTOR;
	res.vec_scene = std::move(vecres);
	return res;
}