/**
 * @file   ErtData2Mesh.cpp
 * @author Karsten Rink
 * @date   2015/04/22
 * @brief  Converts an ERT-CSV-file into a mesh
 *
 * @copyright
 * Copyright (c) 2012-2015, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include <vector>
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
#include "GeoLib/Point.h"

// MeshLib
#include "MeshLib/Mesh.h"
#include "MeshLib/Node.h"
#include "MeshLib/Properties.h"
#include "MeshLib/PropertyVector.h"
#include "MeshLib/Elements/Element.h"
#include "MeshLib/Elements/Quad.h"
#include "MeshLib/MeshEditing/MeshRevision.h"


int main (int argc, char* argv[])
{
	LOGOG_INITIALIZE();
	logog::Cout* logog_cout (new logog::Cout);
	BaseLib::LogogSimpleFormatter *custom_format (new BaseLib::LogogSimpleFormatter);
	logog_cout->SetFormatter(*custom_format);

	TCLAP::CmdLine cmd("Converts a CSV file containing ERT data to a quad mesh.", ' ', "0.1");

	// I/O params
	TCLAP::ValueArg<std::string> mesh_out("o", "mesh-output-file",
	                                      "The name of the new mesh file", true,
	                                      "", "file name of output mesh");
	cmd.add(mesh_out);
	TCLAP::ValueArg<std::string> csv_in("i", "csv-input-file",
	                                    "CSV-file containing ERT information.",
	                                    true, "", "name of the csv input file");
	cmd.add(csv_in);
	cmd.parse(argc, argv);

	std::vector<GeoLib::Point*> points1;
	std::vector<GeoLib::Point*> points2;
	int const r1 = FileIO::CsvInterface::readPoints("hang.txt", '\t', points1, "E1", "N1", "H1");
	int const r2 = FileIO::CsvInterface::readPoints("hang.txt", '\t', points2, "E2", "N2", "H2");
	std::vector<double> z1;
	std::vector<double> z2;
	int const c1 = FileIO::CsvInterface::readColumn<double>("hang.txt", '\t', z1, "z1/m");
	int const c2 = FileIO::CsvInterface::readColumn<double>("hang.txt", '\t', z2, "z2/m");

	if (r1 < 0 || r2 < 0 || c1 < 0 || c2 < 0)
	{
		ERR("Error reading data from file");
		return 1;
	}

	std::vector<MeshLib::Node*> nodes;
	std::size_t const n_quads (points1.size());
	std::vector<MeshLib::Element*> quads;
	quads.reserve(n_quads);
	std::vector<int> materials;
	materials.reserve(n_quads);
	int mat_id(0);
	for (std::size_t i=0; i<n_quads; ++i)
	{
		std::array<MeshLib::Node*, 4> quad_nodes;
		quad_nodes[0] = new MeshLib::Node((*points1[i])[0], (*points1[i])[1], (*points1[i])[2] - z1[i]);
		quad_nodes[1] = new MeshLib::Node((*points1[i])[0], (*points1[i])[1], (*points1[i])[2] - z2[i]);
		quad_nodes[2] = new MeshLib::Node((*points2[i])[0], (*points2[i])[1], (*points2[i])[2] - z2[i]);
		quad_nodes[3] = new MeshLib::Node((*points2[i])[0], (*points2[i])[1], (*points2[i])[2] - z1[i]);
		if (i>0 && (z1[i] != z1[i-1]))
			mat_id++;
		for (std::size_t j=0; j<4; ++j)
			nodes.push_back(quad_nodes[j]);
		quads.push_back(new MeshLib::Quad(quad_nodes));
		materials.push_back(mat_id);
	}
	
	MeshLib::Mesh mesh("ERT Mesh", nodes, quads);
	boost::optional<std::vector<int>&> mat_prop (mesh.getProperties().createNewPropertyVector<int>("MaterialIDs", MeshLib::MeshItemType::Cell));
	mat_prop->insert(mat_prop->end(), materials.begin(), materials.end());

	/*
	MeshLib::MeshRevision rev(mesh);
	MeshLib::Mesh* final_mesh (rev.collapseNodes("ERT Mesh", std::numeric_limits<double>::epsilon()));
	*/
	INFO ("Writing result...");
	FileIO::VtuInterface vtu(&mesh);
	vtu.writeToFile(mesh_out.getValue());

	//delete mesh;
	delete custom_format;
	delete logog_cout;
	LOGOG_SHUTDOWN();

	return 0;
}



