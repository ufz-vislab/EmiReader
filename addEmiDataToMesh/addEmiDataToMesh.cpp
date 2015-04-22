/**
 * @file   addEmiDataToMesh.cpp
 * @author Karsten Rink
 * @date   2015/04/17
 * @brief  Adds EMI Data as an additional scalar array to a mesh
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

// GeoLib
#include "GeoLib/Grid.h"
#include "GeoLib/AnalyticalGeometry.h"

// MeshLib
#include "MeshLib/Mesh.h"
#include "MeshLib/Node.h"
#include "MeshLib/Properties.h"
#include "MeshLib/PropertyVector.h"
#include "MeshLib/Elements/Element.h"
#include "MeshLib/MeshEditing/projectMeshOntoPlane.h"

std::vector<double> getDataFromCSV(MeshLib::Mesh const& mesh, std::vector<GeoLib::Point*> data_points)
{
	GeoLib::Point const origin(0, 0, 0);
	MathLib::Vector3 const normal(0,0,-1);
	MeshLib::Mesh* flat_mesh (MeshLib::projectMeshOntoPlane(mesh, origin, normal));
	std::size_t n_elems (flat_mesh->getNElements());
	std::vector<double> data(n_elems, 0.0);
	std::vector<std::size_t> counter(n_elems, 0);

	std::vector<MeshLib::Node*> const& nodes (flat_mesh->getNodes());
	GeoLib::Grid<MeshLib::Node> grid(nodes.cbegin(), nodes.cend());

	std::size_t const n_points (data_points.size());
	for (std::size_t i=0; i<n_points; ++i)
	{
		MeshLib::Node const pnt((*data_points[i])[0], (*data_points[i])[1], 0.0);
		MeshLib::Node const*const node = grid.getNearestPoint(pnt);
		std::vector<MeshLib::Element*> const& conn_elems (node->getElements());
		std::size_t const n_conn_elems (conn_elems.size());
		for (std::size_t j=0; j<n_conn_elems; ++j)
		{
			if (GeoLib::gaussPointInTriangle(pnt, *conn_elems[j]->getNode(0), *conn_elems[j]->getNode(1), *conn_elems[j]->getNode(2)))
			{
				std::size_t const idx (conn_elems[j]->getID());
				data[idx] += ((*data_points[i])[2]);
				counter[idx]++;
				break;
			}
		}
	}

	for (std::size_t i=0; i<n_elems; ++i)
		if (counter[i] > 0)
			data[i] /= static_cast<double>(counter[i]);

	delete flat_mesh;
	return data;
}

std::vector<double> addFilesAsArrays(std::string csv_base_name, MeshLib::Mesh *mesh, std::string const& name_specifier)
{
	std::vector<GeoLib::Point*> points;
	std::vector<GeoLib::Point*> points2;
	std::vector<GeoLib::Point*> points3;

	std::string file_name = csv_base_name + "_A_" + name_specifier + ".txt";
	INFO ("Reading file %s.", file_name.c_str());
	int e1 = FileIO::CsvInterface::readPoints(file_name, '\t', points,  1, 2, 3);
	file_name = csv_base_name + "_B_" + name_specifier + ".txt";
	INFO ("Reading file %s.", file_name.c_str());
	int e2 = FileIO::CsvInterface::readPoints(file_name, '\t', points2, 1, 2, 3);
	file_name = csv_base_name + "_C_" + name_specifier + ".txt";
	INFO ("Reading file %s.", file_name.c_str());
	int e3 = FileIO::CsvInterface::readPoints(file_name, '\t', points3, 1, 2, 3);

	points.insert(points.end(), points2.begin(), points2.end());
	points.insert(points.end(), points3.begin(), points3.end());

	if (e1 < 0 || e2 < 0 || e3 < 0 || points.empty())
	{
		ERR ("Error reading CSV-file.");
		delete mesh;
		std::vector<double> no_data;
		return no_data;
	}

	std::vector<double> data = getDataFromCSV(*mesh, points);
	std::for_each(points.begin(), points.end(), std::default_delete<GeoLib::Point>());

	return data;
}


int main (int argc, char* argv[])
{
	LOGOG_INITIALIZE();
	logog::Cout* logog_cout (new logog::Cout);
	BaseLib::LogogSimpleFormatter *custom_format (new BaseLib::LogogSimpleFormatter);
	logog_cout->SetFormatter(*custom_format);

	TCLAP::CmdLine cmd("Add EMI data as a scalar cell array to a 2d mesh.", ' ', "0.1");

	// I/O params
	TCLAP::ValueArg<std::string> mesh_out("o", "mesh-output-file",
	                                      "the name of the file the mesh will be written to", true,
	                                      "", "file name of output mesh");
	cmd.add(mesh_out);
	TCLAP::ValueArg<std::string> mesh_in("i", "mesh-input-file",
	                                     "the name of the file containing the input mesh", true,
	                                     "", "file name of input mesh");
	cmd.add(mesh_in);

	TCLAP::ValueArg<std::string> csv_in("", "csv",
	                                    "csv-file containing EMI data to be added as a scalar array.",
	                                    true, "", "name of the csv input file");
	cmd.add(csv_in);
	cmd.parse(argc, argv);

	INFO ("Reading mesh %s.", mesh_in.getValue().c_str());
	MeshLib::Mesh* mesh (FileIO::VtuInterface::readVTUFile(mesh_in.getValue()));
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
	INFO("Mesh read: %d nodes, %d elements.", mesh->getNNodes(), mesh->getNElements());

	/*
	std::vector<GeoLib::Point*> points;
	int e = FileIO::CsvInterface::readPoints(csv_in.getValue(), '\t', points);
	if (e < 0 || points.empty())
	{
		ERR ("Error reading CSV-file.");
		delete mesh;
		return 0;
	}
	data = getDataFromCSV(*mesh, points);
	std::for_each(points.begin(), points.end(), std::default_delete<MathLib::Point3d>());
	*/

	std::vector<double> data = addFilesAsArrays(csv_in.getValue(), mesh, "H");
	if (data.empty())
		return -1;
	std::string const h_prop_name("TM_DD_H");
	boost::optional< MeshLib::PropertyVector<double>&> h_vector = mesh->getProperties().createNewPropertyVector<double>(h_prop_name, MeshLib::MeshItemType::Cell);
	std::copy(data.cbegin(), data.cend(), std::back_inserter(*h_vector));

	data = addFilesAsArrays(csv_in.getValue(), mesh, "V");
	if (data.empty())
		return -1;
	std::string const v_prop_name("TM_DD_V");
	boost::optional< MeshLib::PropertyVector<double>&> v_vector = mesh->getProperties().createNewPropertyVector<double>(v_prop_name, MeshLib::MeshItemType::Cell);
	std::copy(data.cbegin(), data.cend(), std::back_inserter(*v_vector));

	INFO ("Writing result...");
	FileIO::VtuInterface vtu(mesh);
	vtu.writeToFile(mesh_out.getValue());

	delete mesh;
	delete custom_format;
	delete logog_cout;
	LOGOG_SHUTDOWN();

	return 0;
}


