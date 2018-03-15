#include <cassert>
#include <iostream>
#include <fstream>

#include <morphio/mut/writers.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>

#include "../plugin/errorMessages.h"

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5Object.hpp>

namespace morphio
{
namespace mut
{
namespace writer
{
void swc(const Morphology& morphology, const std::string& filename)
{
    std::ofstream myfile;
    myfile.open (filename);
    using std::setw;

    myfile << "# index" << setw(9)
           << "type" << setw(9)
           << "X" << setw(12)
           << "Y" << setw(12)
           << "Z" << setw(12)
           << "radius" << setw(13)
           << "parent\n" << std::endl;


    int segmentIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    auto soma = morphology.soma();

    if(morphology.soma()->points().size() < 1)
        throw WriterError(plugin::ErrorMessages().ERROR_WRITE_NO_SOMA());

    for (int i = 0; i < soma->points().size(); ++i){
        myfile << segmentIdOnDisk++ << setw(12) << SECTION_SOMA << setw(12)
               << soma->points()[i][0] << setw(12) << soma->points()[i][1] << setw(12)
               << soma->points()[i][2] << setw(12) << soma->diameters()[i] / 2. << setw(12)
                  << (i==0 ? -1 : segmentIdOnDisk-1) << std::endl;
    }

    for(auto it = morphology.depth_begin(); it != morphology.depth_end(); ++it) {
        int32_t sectionId = *it;
        auto section = morphology.section(sectionId);
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(points.size() > 0 && "Empty section");
        bool isRootSection = morphology.parent(sectionId) < 0;
        for (int i = (isRootSection ? 0 : 1); i < points.size(); ++i)
        {
            myfile << segmentIdOnDisk << setw(12) << section->type() << setw(12)
                   << points[i][0] << setw(12) << points[i][1] << setw(12)
                   << points[i][2] << setw(12) << diameters[i] / 2. << setw(12);

            if (i > (isRootSection ? 0 : 1))
                myfile << segmentIdOnDisk - 1 << std::endl;
            else {
                int32_t parentId = morphology.parent(sectionId);
                myfile << (parentId != -1 ? newIds[parentId] : 1) << std::endl;
            }

            ++segmentIdOnDisk;
        }
        newIds[section->id()] = segmentIdOnDisk - 1;
    }

    myfile.close();

}

void _write_asc_points(const Points& points,
                       const std::vector<float>& diameters, int indentLevel)
{
    for (int i = 0; i < points.size(); ++i)
    {
        std::cout << std::string(indentLevel, ' ') << "(" << points[i][0] << ' '
                  << points[i][1] << ' ' << points[i][2] << ' ' << diameters[i]
                  << ')' << std::endl;
    }
}

void _write_asc_section(const Morphology& morpho, uint32_t id, int indentLevel)
{
    std::string indent(indentLevel, ' ');
    auto section = morpho.section(id);
    _write_asc_points(section->points(), section->diameters(), indentLevel);

    if (!morpho.children(id).empty())
    {
        auto children = morpho.children(id);
        size_t nChildren = children.size();
        for (int i = 0; i<nChildren; ++i)
        {
            std::cout << indent << (i == 0 ? "(" : "|") << std::endl;
            _write_asc_section(morpho, children[i], indentLevel + 2);
        }
        std::cout << indent << ")" << std::endl;
    }
}

void asc(const Morphology& morphology, const std::string& filename)
{
    std::map<morphio::SectionType, std::string> header;
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";

    const auto soma = morphology.soma();
    std::cout << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
    _write_asc_points(soma->points(), soma->diameters(), 2);
    std::cout << ")\n\n";

    for (auto& id : morphology.rootSections())
    {
        std::cout << header[morphology.section(id)->type()];
        _write_asc_section(morphology, id, 2);
        std::cout << ")\n\n";
    }
}

void h5(const Morphology& morpho, const std::string& filename)
{

    HighFive::File h5_file(filename, HighFive::File::ReadWrite | HighFive::File::Create |
                           HighFive::File::Truncate);



    int sectionIdOnDisk = 1;
    int i = 0;
    std::map<uint32_t, int32_t> newIds;

    std::vector<std::vector<float>> raw_points;
    std::vector<std::vector<int32_t>> raw_structure;

    const auto &points = morpho.soma()->points();
    const auto &diameters = morpho.soma()->diameters();
    const std::size_t numberOfPoints = points.size();

    if(numberOfPoints < 1)
        throw WriterError(plugin::ErrorMessages().ERROR_WRITE_NO_SOMA());

    for(int i = 0;i<numberOfPoints; ++i)
        raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});
    raw_structure.push_back({0, -1, SECTION_SOMA});
    int offset = 0;
    offset += morpho.soma()->points().size();


    for(auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        uint32_t sectionId = *it;
        uint32_t parentId = morpho.parent(sectionId);
        int parentOnDisk = (parentId != -1 ? newIds[parentId] : 0);

        auto section = morpho.section(sectionId);
        const auto &points = section->points();
        const auto &diameters = section->diameters();
        const std::size_t numberOfPoints = points.size();
        raw_structure.push_back({offset, parentOnDisk, section->type()});
        for(int i = 0;i<numberOfPoints; ++i)
            raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});

        newIds[section->id()] = sectionIdOnDisk++;
        offset += numberOfPoints;
    }

    std::vector<std::string> comment{" created out by morpho_tool v1"};

    HighFive::DataSet dpoints =
        h5_file.createDataSet<double>("/points", HighFive::DataSpace::From(raw_points));
    HighFive::DataSet dstructures = h5_file.createDataSet<int32_t>(
        "/structure", HighFive::DataSpace::From(raw_structure));
    HighFive::Attribute acomment = h5_file.createAttribute<std::string>(
        "comment", HighFive::DataSpace::From(comment));

    dpoints.write(raw_points);
    dstructures.write(raw_structure);
    acomment.write(comment);

}

} // end namespace writer
} // end namespace mut
} // end namespace morphio