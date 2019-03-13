#pragma once

#include <queue>
#include <stack>
#include <set>

#include <morphio/types.h>
//#include <morphio/section.h>
//#include <morphio/vascSection.h>
namespace morphio {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class Iterator
{
    friend class Section;
    friend class Morphology;

    T container;

    Iterator();

public:
    Iterator(const Section& section);
    Iterator(const Morphology& morphology);
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    Section operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};

class graph_iterator
{
    friend class VasculatureSection;
    friend class VasculatureMorphology;
    std::set<VasculatureSection> visited;

    std::stack<VasculatureSection> container;

    graph_iterator();

public:
    graph_iterator(const VasculatureSection& vasculatureSection);
    graph_iterator(const VasculatureMorphology& vasculatureMorphology);
    bool operator==(graph_iterator other) const;
    bool operator!=(graph_iterator other) const;
    VasculatureSection operator*() const;
    graph_iterator& operator++();
    graph_iterator operator++(int);
};

} // namespace morphio