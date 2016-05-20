/**
 * @file   RappbodeTracer.cpp
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
#include "BaseLib/FileTools.h"

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

MeshLib::Mesh*  createMesh()
{
	std::string const x_file ("utm_x.csv");
	std::string const y_file ("utm_y.csv");
	std::string const z_file ("Z.csv");
	std::vector<double> x;
	int const e1 = FileIO::CsvInterface::readColumn<double>(x_file, '\t', x, 0);
	std::vector<double> y;
	int const e2 = FileIO::CsvInterface::readColumn<double>(y_file, '\t', y, 0);
	std::vector<double> z;
	int const e3 = FileIO::CsvInterface::readColumn<double>(z_file, '\t', z, 0);

	if (e1!=0 || e2!=0 || e3!=0)
		return nullptr;

	if (x.size() != y.size())
		return nullptr;

	std::size_t const n_cols (x.size());
	std::size_t const n_rows (z.size());
	std::vector<MeshLib::Node*> nodes;
	nodes.reserve(n_rows*n_cols);
	for (std::size_t r=0; r<n_rows; ++r)
		for (std::size_t c=0; c<n_cols; ++c)
			nodes.push_back(new MeshLib::Node(x[c], y[c], z[r], nodes.size()));

	std::vector<MeshLib::Element*> elems;
	elems.reserve((n_rows-1)*(n_cols-1));
	std::vector<std::size_t> mats;
	mats.reserve((n_rows-1)*(n_cols-1));
	for (std::size_t r=0; r<n_rows-2; ++r)
	{
		std::size_t base_idx = r*n_cols;
		for (std::size_t c=0; c<n_cols-1; ++c)
		{
			std::array<MeshLib::Node*, 4> quad_nodes;
			quad_nodes[0] = nodes[base_idx + c];
			quad_nodes[1] = nodes[base_idx + c + n_cols];
			quad_nodes[2] = nodes[base_idx + c + n_cols + 1];
			quad_nodes[3] = nodes[base_idx + c + 1];
			elems.push_back(new MeshLib::Quad(quad_nodes));
			mats.push_back(r);
		}
	}

	MeshLib::Mesh* mesh = new MeshLib::Mesh("Mesh", nodes, elems);
	boost::optional<std::vector<int>&> mat_prop (mesh->getProperties().createNewPropertyVector<int>("MaterialIDs", MeshLib::MeshItemType::Cell));
	mat_prop->insert(mat_prop->end(), mats.begin(), mats.end());

	return mesh;
}

std::string number2str(std::size_t n)
{
	std::ostringstream convert;
	convert << n;
	return convert.str();
}

std::string output_question(std::string const& output_name)
{
	WARN ("Output file %s already exists. Overwrite? (y/n)", output_name);
	std::string input;
	std::cin >> input;
	return input;
}

bool overwriteFiles(std::string const& output_name)
{
	if (!BaseLib::IsFileExisting(output_name))
		return true;

	std::string input ("");
	while (input != "y" && input != "Y" && input != "n" && input != "N")
		input = output_question(output_name);
		
	if (input == "y" || input == "Y")
		return true;
	return false;
}

int main (int argc, char* argv[])
{
	LOGOG_INITIALIZE();
	logog::Cout* logog_cout (new logog::Cout);
	BaseLib::LogogSimpleFormatter *custom_format (new BaseLib::LogogSimpleFormatter);
	logog_cout->SetFormatter(*custom_format);

	TCLAP::CmdLine cmd("Adds a scalar array time series from a csv-file to an existing mesh or a time series of meshes.", ' ', "0.1");

	// I/O params
	TCLAP::ValueArg<std::string> mesh_new("b", "base",
	                                      "Use this if a time series of vtu-files should be created based on a single vtu-file. If a time series *is* already existing, this parameter need not be set.",
	                                       false, "", "base mesh input");
	cmd.add(mesh_new);
	TCLAP::ValueArg<std::string> mesh_add("t", "output",
	                                      "This is the base name of the output files, e.g. \'output\' will result in files called \'output0.vtu\', \'output1.vtu\', etc. If a time series is already existing, a new array will simply be added to each time step.",
	                                       true, "", "name of mesh output");
	cmd.add(mesh_add);
	TCLAP::ValueArg<std::string> csv_in("i", "csv",
	                                    "CSV-file containing the input information for the scalar arrays. It is assumed that all timesteps are in one file with an empty line between timesteps and with one value per grid cell per time step.",
	                                    true, "", "csv input file");
	cmd.add(csv_in);
	cmd.parse(argc, argv);

	//MeshLib::Mesh* mesh = createMesh();

	if (!csv_in.isSet())
	{
		ERR ("Name of csv-file is missing.");
		return -5;
	}

	if (!mesh_add.isSet())
	{
		ERR ("Base name of output files is missing.");
		return -4;
	}

	int n_rows = -1;
	MeshLib::Mesh* mesh = nullptr;
	if (mesh_new.isSet())
	{
		mesh = MeshLib::IO::VtuInterface::readVTUFile(mesh_new.getValue());
		if (mesh == nullptr)
		{

			return -1;
		}
		boost::optional<MeshLib::PropertyVector<int>&> materials (mesh->getProperties().getPropertyVector<int>("MaterialIDs"));
		n_rows = (*std::max_element(materials->cbegin(), materials->cend())) + 1;
	}

	std::ifstream in( csv_in.getValue().c_str() );
	if (!in.is_open())
	{
		ERR ("Could not open CSV file.");
		return -2;
	}

	double const nan_value = 0.0;

	std::string line;
	std::size_t file_counter(0);
	bool overwrite (false);
	while (getline(in, line))
	{
		if (!mesh_new.isSet())
		{
			mesh = MeshLib::IO::VtuInterface::readVTUFile(mesh_add.getValue() + number2str(file_counter) + ".vtu");
			if (mesh==nullptr)
			{
				ERR("No base mesh given and no mesh for time step %d found.", file_counter);
				return -6;
			}
			boost::optional<MeshLib::PropertyVector<int>&> materials (mesh->getProperties().getPropertyVector<int>("MaterialIDs"));
			n_rows = (*std::max_element(materials->cbegin(), materials->cend())) + 1;
		}
		std::string const prop_name(BaseLib::extractBaseNameWithoutExtension(csv_in.getValue()));
		boost::optional<MeshLib::PropertyVector<double>&> prop (mesh->getProperties().createNewPropertyVector<double>(prop_name, MeshLib::MeshItemType::Cell));
		prop->resize(mesh->getNNodes(), 0);

		for (int i=0; i<=n_rows; ++i)
		{
			std::list<std::string> fields = BaseLib::splitString(line, ',');
			int const n_cols (fields.size());
			if (n_cols != (mesh->getNElements() / n_rows) + 1)
				return -3;

			int idx_cnt(i * (n_cols-1));
			auto it = fields.cbegin();
			it++;
			for (; it != fields.cend(); ++it)
			{
				if ((*it).compare("NaN") == 0)
					(*prop)[idx_cnt++] = nan_value;
				else 
					(*prop)[idx_cnt++] = std::stod(*it);
			}
			getline(in, line);
		}

		INFO ("Writing result #%d...", file_counter);
		std::string output_name (mesh_add.getValue() + number2str(file_counter) + ".vtu");
		if (overwrite == false)
		{
			overwrite = overwriteFiles(output_name);

			if (overwrite == false)
				return -7;
		}
		MeshLib::IO::VtuInterface vtu(mesh);
		vtu.writeToFile(output_name);
		file_counter++;
	
		getline(in, line);
		if (!line.empty())
			ERR("something is wrong here.");
	}
	
	//delete mesh;
	delete custom_format;
	delete logog_cout;
	LOGOG_SHUTDOWN();

	return 0;
}



