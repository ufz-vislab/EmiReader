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
#include "MeshLib/IO/VtkIO/VtuInterface.h"
#include "FileIO/CsvInterface.h"
#include "GeoLib/IO/AsciiRasterInterface.h"

// GeoLib
#include "GeoLib/Point.h"
#include "GeoLib/Raster.h"

// MeshLib
#include "MeshLib/Mesh.h"
#include "MeshLib/Node.h"
#include "MeshLib/Properties.h"
#include "MeshLib/PropertyVector.h"
#include "MeshLib/Elements/Element.h"
#include "MeshLib/Elements/Quad.h"
#include "MeshLib/MeshEditing/MeshRevision.h"


std::vector<double> getElevationCorrectionValues(GeoLib::Raster const& dem, std::vector<MeshLib::Node*> &nodes)
{
	std::size_t const n_sfc_nodes (nodes.size());
	std::vector<double> elevation_correction;
	for (std::size_t i=0; i<n_sfc_nodes; ++i)
	{
		double a = (*nodes[i])[2];
		double b = dem.getValueAtPoint(*nodes[i]);
		elevation_correction.push_back((*nodes[i])[2] - dem.getValueAtPoint(*nodes[i]));
		(*nodes[i])[2] -= elevation_correction[i];
	}
	return elevation_correction;
}

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
	TCLAP::ValueArg<std::string> dem_in("s", "DEM-file",
	                                    "Surface DEM for mapping ERT data", false,
	                                    "", "file name of the Surface DEM");
	cmd.add(dem_in);
	cmd.parse(argc, argv);

	std::vector<std::size_t> ids;
	int const e = FileIO::CsvInterface::readColumn<std::size_t>(csv_in.getValue(), '\t', ids, "x2/m");
	std::size_t const n_nodes_per_layer (ids.back()+1);
	std::vector<GeoLib::Point*> points1;
	std::vector<GeoLib::Point*> points2;
	int const r1 = FileIO::CsvInterface::readPoints(csv_in.getValue(), '\t', points1, "E1", "N1", "H1");
	int const r2 = FileIO::CsvInterface::readPoints(csv_in.getValue(), '\t', points2, "E2", "N2", "H2");
	std::vector<double> z1;
	std::vector<double> z2;
	int const c1 = FileIO::CsvInterface::readColumn<double>(csv_in.getValue(), '\t', z1, "z1/m");
	int const c2 = FileIO::CsvInterface::readColumn<double>(csv_in.getValue(), '\t', z2, "z2/m");

	if (e!= 0 || r1 != 0 || r2 != 0 || c1 != 0 || c2 != 0)
	{
		ERR("Error reading data from file");
		return 1;
	}
	std::size_t const n_quads (points1.size());
	for (std::size_t i=1; i<n_quads; ++i)
	{
		if ((*points1[i-1])[0] == (*points2[i])[0] ||
		    (*points1[i-1])[1] == (*points2[i])[1] ||
		    (*points1[i-1])[2] == (*points2[i])[2])
		{
			ERR ("Error in ERT file.");
			return 1;
		}
	}

	std::vector<MeshLib::Node*> nodes;
	std::vector<MeshLib::Element*> quads;
	quads.reserve(n_quads);
	std::vector<int> materials;
	materials.reserve(n_quads);

	nodes.push_back(new MeshLib::Node((*points1[0])[0], (*points1[0])[1], (*points1[0])[2] - z1[0], nodes.size() ));
	for (std::size_t i=0; i<n_nodes_per_layer; ++i)
		nodes.push_back(new MeshLib::Node((*points2[i])[0], (*points2[i])[1], (*points2[i])[2] - z1[i], nodes.size() ));

	std::vector<double> elevation_correction (nodes.size(), 0.0);
	if (dem_in.isSet())
	{
		GeoLib::Raster* dem = GeoLib::IO::AsciiRasterInterface::readRaster(dem_in.getValue());
		elevation_correction = getElevationCorrectionValues(*dem, nodes);
		delete dem;
	}
	std::size_t const n_layers ((n_quads / n_nodes_per_layer));
	for (std::size_t mat_id=0; mat_id<n_layers; ++mat_id)
	{
		std::size_t const base_idx (mat_id * n_nodes_per_layer);
		double z = (*points1[base_idx])[2] - elevation_correction[0] - z2[base_idx];
		nodes.push_back(new MeshLib::Node((*points1[base_idx])[0], (*points1[base_idx])[1], z, nodes.size() ));
		for (std::size_t i=0; i<n_nodes_per_layer; ++i)
		{
			std::size_t const idx (base_idx + i);
			std::array<MeshLib::Node*, 4> quad_nodes;
			quad_nodes[0] = nodes[base_idx + mat_id + i];
			quad_nodes[1] = nodes[nodes.size()-1];
			z = (*points2[idx])[2] - elevation_correction[i+1] - z2[base_idx+i];
			nodes.push_back(new MeshLib::Node((*points2[idx])[0], (*points2[idx])[1], z, nodes.size() ));
			quad_nodes[2] = nodes.back();
			quad_nodes[3] = nodes[base_idx + mat_id + i+1];
			quads.push_back(new MeshLib::Quad(quad_nodes));
			materials.push_back(mat_id);
		}
	}

	MeshLib::Mesh mesh("ERT Mesh", nodes, quads);
	
	boost::optional<std::vector<int>&> mat_prop (mesh.getProperties().createNewPropertyVector<int>("MaterialIDs", MeshLib::MeshItemType::Cell));
	mat_prop->insert(mat_prop->end(), materials.begin(), materials.end());

	boost::optional<std::vector<double>&> resistance (mesh.getProperties().createNewPropertyVector<double>("Resistance", MeshLib::MeshItemType::Cell));
	int const errors_res = FileIO::CsvInterface::readColumn<double>(csv_in.getValue(), '\t', *resistance, "rho/Ohmm ");
	if (errors_res != 0 || resistance->size() != n_quads)
		WARN ("Erros reading resistance values.");

	boost::optional<std::vector<double>&> coverage (mesh.getProperties().createNewPropertyVector<double>("Coverage", MeshLib::MeshItemType::Cell));
	int const errors_cov = FileIO::CsvInterface::readColumn<double>(csv_in.getValue(), '\t', *coverage, "coverage");
	if (errors_cov != 0 || coverage->size() != n_quads)
		WARN ("Erros reading coverage values.");
	
	/* TODO writing arrays with tuple size > 1 needs fixing
	double const max_cov = *std::max_element(coverage->cbegin(), coverage->cend());
	boost::optional<std::vector<double>&> conduct (mesh.getProperties().createNewPropertyVector<double>("Conductivity", MeshLib::MeshItemType::Cell, 1));
	if (errors_res == 0 && errors_cov == 0)
	{
		for (std::size_t i=0; i<n_quads; ++i)
		{
			conduct->push_back(1.0 / (*resistance)[i]);
			conduct->push_back((*coverage)[i] / max_cov);
		}
	}
	*/
	
	INFO ("Writing result...");
	MeshLib::IO::VtuInterface vtu(&mesh);
	vtu.writeToFile(mesh_out.getValue());

	//delete mesh;
	delete custom_format;
	delete logog_cout;
	LOGOG_SHUTDOWN();

	return 0;
}



