/**
 * @file   makeBildings.cpp
 * @author Karsten Rink
 * @date   2016/03/03
 * @brief  Takes polygons of building plans and creates simple 3d objects
 *
 * @copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include <algorithm>
#include <memory>
#include <vector>

#include <tclap/CmdLine.h>
#include "Applications/ApplicationsLib/LogogSetup.h"

#include "GeoLib/GEOObjects.h"
#include "GeoLib/Triangle.h"
#include "GeoLib/IO/XmlIO/Qt/XmlGmlInterface.h"

#include <QCoreApplication>


std::unique_ptr<std::vector<GeoLib::Polyline*>> copyPolylinesVector(
	std::vector<GeoLib::Polyline*> const& polylines,
	std::vector<GeoLib::Point*> const& points)
{
	std::size_t n_plys = polylines.size();
	std::unique_ptr<std::vector<GeoLib::Polyline*>> new_lines (new std::vector<GeoLib::Polyline*>(n_plys, nullptr));

	for (std::size_t i=0; i<n_plys; ++i)
	{
		if (polylines[i] == nullptr)
			continue;
		(*new_lines)[i] = new GeoLib::Polyline(points);
		std::size_t nLinePnts (polylines[i]->getNumberOfPoints());
		for (std::size_t j=0; j<nLinePnts; ++j)
			(*new_lines)[i]->addPoint(polylines[i]->getPointID(j));
	}
	return new_lines;
}

std::unique_ptr<std::vector<GeoLib::Surface*>> copySurfacesVector(
	std::vector<GeoLib::Surface*> const& surfaces,
	std::vector<GeoLib::Point*> const& points)
{
	std::size_t n_sfc = surfaces.size();
	std::unique_ptr<std::vector<GeoLib::Surface*>> new_surfaces (new std::vector<GeoLib::Surface*>(n_sfc, nullptr));

	for (std::size_t i=0; i<n_sfc; ++i)
	{
		if (surfaces[i] == nullptr)
			continue;
		(*new_surfaces)[i] = new GeoLib::Surface(points);

		std::size_t n_tris (surfaces[i]->getNTriangles());
		for (std::size_t j=0; j<n_tris; ++j)
		{
			GeoLib::Triangle const* t = (*surfaces[i])[j];
			(*new_surfaces)[i]->addTriangle(t->getPoint(0)->getID(), t->getPoint(1)->getID(), t->getPoint(2)->getID());
		}
	}
	return new_surfaces;
}

void makeBuildings(GeoLib::GEOObjects &geo_objects, std::string const& geo_name, std::string &output_name, double height)
{
	std::vector<GeoLib::Point*> const* pnts (geo_objects.getPointVec(geo_name));
	std::vector<GeoLib::Polyline*> const* plys (geo_objects.getPolylineVec(geo_name));
	std::vector<GeoLib::Surface*> const* sfcs (geo_objects.getSurfaceVec(geo_name));
	std::size_t const n_pnts (pnts->size());
	std::size_t const n_plys (plys->size());
	std::size_t const n_sfcs (sfcs->size());

	std::unique_ptr< std::vector<GeoLib::Point*> > new_pnts (new std::vector<GeoLib::Point*>);
	for (GeoLib::Point* point : *pnts)
		if (point)
			new_pnts->push_back(new GeoLib::Point(*point));

	std::unique_ptr< std::vector<GeoLib::Polyline*> > new_plys = copyPolylinesVector(*plys, *new_pnts);
	std::unique_ptr< std::vector<GeoLib::Surface*> > new_sfcs = copySurfacesVector(*sfcs, *new_pnts);

	for (GeoLib::Point* point : *pnts)
	{
		if (point)
			new_pnts->push_back(new GeoLib::Point((*point)[0], (*point)[1], (*point)[2]+height, point->getID()+n_pnts));
		else
			new_pnts->push_back(nullptr);
	}

	for (GeoLib::Polyline* p : *plys)
	{
		if (p == nullptr)
			continue;
		std::size_t const np (p->getNumberOfPoints());
		GeoLib::Surface* s = new GeoLib::Surface(*new_pnts);
		for (std::size_t i=1; i<np; ++i)
		{
			s->addTriangle(p->getPoint(i)->getID(), p->getPoint(i-1)->getID(), p->getPoint(i-1)->getID()+n_pnts);
			s->addTriangle(p->getPoint(i)->getID(), p->getPoint(i-1)->getID()+n_pnts, p->getPoint(i)->getID()+n_pnts);
		}
		new_sfcs->push_back(s);
	}

	for (std::size_t j=0; j<n_sfcs; ++j)
	{
		if ((*sfcs)[j] == nullptr)
			continue;
		std::size_t const n_tris ((*sfcs)[j]->getNTriangles());
		GeoLib::Surface* s = new GeoLib::Surface(*new_pnts);
		for (std::size_t i = 0; i<n_tris; i++)
		{
			GeoLib::Triangle const* t = (*(*sfcs)[j])[i];
			s->addTriangle(t->getPoint(0)->getID()+n_pnts, t->getPoint(1)->getID()+n_pnts, t->getPoint(2)->getID()+n_pnts);
		}
		new_sfcs->push_back(s);
	}

	geo_objects.addPointVec(std::move(new_pnts), output_name);
	geo_objects.addPolylineVec(std::move(new_plys), output_name);
	geo_objects.addSurfaceVec(std::move(new_sfcs), output_name);
}

int main (int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	ApplicationsLib::LogogSetup logog_setup;

	TCLAP::CmdLine cmd("Uses polygons from building plans to create 3d objects.", ' ', "0.1");

	TCLAP::ValueArg<double> height ("s", "size", "height of the 3d objects (buildings) in metres", true,
	                                1.0, "height of objects");
	cmd.add(height);
	TCLAP::ValueArg<std::string> geo_out("o", "geo-output-file",
	                                     "the name of the file the 3d geometry will be written to", true,
	                                     "", "file name of output geometry");
	cmd.add(geo_out);
	TCLAP::ValueArg<std::string> geo_in("i", "geo-input-file",
	                                    "the name of the file containing the input geometry", true,
	                                    "", "file name of input geometry");
	cmd.add(geo_in);

	cmd.parse(argc, argv);

	INFO ("Reading geometry %s.", geo_in.getValue().c_str());

	GeoLib::GEOObjects geo_objects;
	GeoLib::IO::XmlGmlInterface xml(geo_objects);
	if (!xml.readFile(geo_in.getValue()))
	{
		ERR ("Error reading geometry.")
		return 1;
	}
	std::vector<std::string> geo_names;
	geo_objects.getGeometryNames(geo_names);
	std::string output_name ("output");

	makeBuildings(geo_objects, geo_names[0], output_name, height.getValue());

	xml.setNameForExport(output_name);
	xml.writeToFile(geo_out.getValue());

	return 0;
}

