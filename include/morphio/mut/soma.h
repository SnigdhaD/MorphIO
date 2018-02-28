#pragma once

#include <morphio/properties.h>

namespace morphio
{
namespace mut
{
class Soma
{
public:
    Soma() {}
    Soma(const Property::PointLevel &pointProperties);
    Soma(const morphio::Soma& soma);
    std::vector<Point>& points() { return _pointProperties._points; }
    const std::vector<Point>& points() const { return _pointProperties._points; }

    std::vector<float> diameters() { return _pointProperties._diameters; }
    const std::vector<float> diameters() const { return _pointProperties._diameters; }

private:
    friend class Morphology;
    Property::PointLevel _pointProperties;
};

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Soma> sectionPtr);
std::ostream& operator<<(std::ostream& os, const Soma& soma);

} // namespace mut
} // namespace morphio