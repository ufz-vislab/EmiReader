/**
 * @file   EmiData2PolyData.cpp
 * @author Karsten Rink
 * @date   2015/04/30
 * @brief  Converts EMI CSV files to VTK PolyData files
 *
 * @copyright
 * Copyright (c) 2012-2015, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include <algorithm>

// TCLAP
#include "tclap/CmdLine.h"

// ThirdParty/logog
#include "logog/include/logog.hpp"

// BaseLib
#include "BaseLib/LogogSimpleFormatter.h"

// FileIO
#include "FileIO/VtkIO/VtuInterface.h"
#include "FileIO/CsvInterface.h"
#include "FileIO/XmlIO/Qt/XmlGmlInterface.h"

// GeoLib
#include "GeoLib/GEOObjects.h"

// MeshLib
#include "MeshLib/Mesh.h"
#include "MeshLib/Node.h"
#include "MeshLib/Properties.h"
#include "MeshLib/PropertyVector.h"
#include "MeshLib/Elements/Element.h"

#include "MeshGeoToolsLib/GeoMapper.h"

bool getPointsFromFile(std::vector<GeoLib::Point*> &points, std::string csv_base_name, char const& name_specifier, char const& region_specifier)
{
	std::size_t const n_points (points.size());
	std::string const file_name = csv_base_name + "_" + region_specifier + "_" + name_specifier + ".txt";
	INFO ("Reading file %s.", file_name.c_str());
	int e = FileIO::CsvInterface::readPoints(file_name, '\t', points,  1, 2);

	if (e < 0 || points.size() == n_points)
	{
		ERR ("Error reading CSV-file.");
		return false;
	}
	return true;
}

void getMeasurements(std::vector<double> &emi, std::string csv_base_name, char const& name_specifier, char const& region_specifier)
{
	std::size_t const n_points (emi.size());
	std::string const file_name = csv_base_name + "_" + region_specifier + "_" + name_specifier + ".txt";
	int const e = FileIO::CsvInterface::readColumn(file_name, '\t', emi, 3);
	std::cout << "Read " << (emi.size()-n_points) << " values from " << file_name << std::endl;
}

void writeMeasurementsToFile(std::vector<double> const& emi, std::string const& base_name , char name_specifier)
{
	std::string const file_name = base_name + "_" + name_specifier + ".txt";
	std::ofstream out (file_name.c_str(), std::ios::out );
	std::size_t const n_vals (emi.size());
	for (std::size_t i=0; i<n_vals; ++i)
		out << emi[i] << "\n";
	out.close();
}

int main (int argc, char* argv[])
{
	LOGOG_INITIALIZE();
	logog::Cout* logog_cout (new logog::Cout);
	BaseLib::LogogSimpleFormatter *custom_format (new BaseLib::LogogSimpleFormatter);
	logog_cout->SetFormatter(*custom_format);

	TCLAP::CmdLine cmd("Add EMI data as a scalar cell array to a 2d mesh.", ' ', "0.1");

	// I/O params
	TCLAP::ValueArg<std::string> poly_out("o", "polydata-output-file",
	                                      "the name of the file the data will be written to", true,
	                                      "", "file name of polydata file");
	cmd.add(poly_out);
	TCLAP::ValueArg<std::string> csv_in("i", "csv-input-file",
	                                    "csv-file containing EMI data", true,
	                                    "", "name of the csv input file");
	cmd.add(csv_in);
	TCLAP::ValueArg<std::string> dem_in("s", "DEM-file",
	                                    "Surface DEM for mapping ERT data", false,
	                                    "", "file name of the Surface DEM");
	cmd.add(dem_in);
	cmd.parse(argc, argv);

	MeshLib::Mesh* mesh (nullptr);
	if (dem_in.isSet())
	{
		mesh = FileIO::VtuInterface::readVTUFile(dem_in.getValue());
		if (mesh == nullptr)
		{
			ERR ("Error reading mesh file.");
			return -2;
		}

		if (mesh->getDimension() != 2)
		{
			ERR ("This utility can handle only 2d meshes at this point.");
			delete mesh;
			return -3;
		}
		INFO("Surface mesh read: %d nodes, %d elements.", mesh->getNNodes(), mesh->getNElements());
	}

	GeoLib::GEOObjects geo_objects;
	FileIO::XmlGmlInterface xml(geo_objects);
	//std::vector<GeoLib::Polyline*> *lines = new std::vector<GeoLib::Polyline*>;
	std::array<char, 2> dipol = {{ 'H', 'V' }};
	std::array<char,3> const regions = {{'A', 'B', 'C'}};
	for (std::size_t j=0; j<dipol.size(); ++j)
	{
		std::vector<GeoLib::Point*> *points   = new std::vector<GeoLib::Point*>;
		for (std::size_t i=0; i<regions.size(); ++i)
		{
			//std::size_t const start_idx (points->size());
			getPointsFromFile(*points, csv_in.getValue(), dipol[j], regions[i]);
			//std::size_t const end_idx (points->size());
			//GeoLib::Polyline* line = new GeoLib::Polyline(*points);
			//for (std::size_t j=start_idx; j<end_idx; ++j)
			//	line->addPoint(j);
			//lines->push_back(line);
		}
		std::string geo_name (std::string("EMI Data ").append(1,dipol[j]));
		geo_objects.addPointVec(points, geo_name);
		//geo_objects.addPolylineVec(lines, geo_name);

		if (mesh != nullptr)
		{
			GeoMapper mapper(geo_objects, geo_name);
			mapper.mapOnMesh(mesh);
		}
		
		xml.setNameForExport(geo_name);
		std::string const output_name = poly_out.getValue() + "_" + dipol[j] + ".gml";
		xml.writeToFile(output_name);
		
		std::vector<double> emi;
		for (std::size_t i=0; i<regions.size(); ++i)
			getMeasurements(emi, csv_in.getValue(), dipol[j], regions[i]);
		writeMeasurementsToFile(emi, poly_out.getValue(), dipol[j]);
		std::for_each(points->begin(), points->end(), std::default_delete<GeoLib::Point>());
		delete points;
	}
	
	delete mesh;
	delete custom_format;
	delete logog_cout;
	LOGOG_SHUTDOWN();

	return 0;
}


