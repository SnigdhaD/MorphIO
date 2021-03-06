#pragma once

#include <map> // std::map
#include <memory> // std::shared_ptr
#include <set> // std::set
#include <string> // std::string

#include <morphio/mut/modifiers.h>
#include <morphio/mut/section.h>

namespace morphio {
/**
   Set the maximum number of warnings to be printed on screen
**/
void set_maximum_warnings(int n_warnings);
void set_ignored_warning(Warning warning, bool ignore = true);
void set_ignored_warning(const std::vector<Warning>& warning,
    bool ignore = true);

void LBERROR(Warning warning, const std::string& msg);

namespace readers {
enum ErrorLevel
{
    INFO,
    WARNING,
    ERROR
};

struct DebugInfo
{
public:
    DebugInfo(std::string filename = "")
        : _filename(filename)
    {
    }

    void setLineNumber(uint32_t sectionId, unsigned int line)
    {
        _lineNumbers[sectionId] = static_cast<int>(line);
    }

    int32_t getLineNumber(uint32_t sectionId) const
    {
        try {
            return _lineNumbers.at(sectionId);
        } catch (const std::out_of_range&) {
            return -1;
        }
    }
    std::string _filename;

private:
    std::map<unsigned int, int> _lineNumbers;
};

static std::set<Warning> _ignoredWarnings;

struct Sample
{
    Sample()
        : valid(false)
        , type(SECTION_UNDEFINED)
        , parentId(-1)
        , lineNumber(0)
    {
    }

    explicit Sample(const char* line, unsigned int lineNumber_)
        : lineNumber(lineNumber_)
    {
        float radius;
        int int_type;
        valid = sscanf(line, "%20u%20d%20f%20f%20f%20f%20d", &id, &int_type,
                    &point[0], &point[1], &point[2], &radius, &parentId) == 7;

        type = static_cast<SectionType>(int_type);
        diameter = radius * 2; // The point array stores diameters.
    }

    float diameter;
    bool valid;
    Point point; // x, y, z and diameter
    SectionType type;
    int parentId;
    unsigned int id;
    unsigned int lineNumber;
};

class ErrorMessages
{
public:
    ErrorMessages() {}
    ErrorMessages(const std::string& uri)
        : _uri(uri)
    {
    }

    /**
       Is the output of the warning ignored
    **/
    static bool isIgnored(Warning warning);

    const std::string errorLink(long unsigned int lineNumber, ErrorLevel errorLevel) const
    {
        std::map<ErrorLevel, std::string> SEVERITY{{ErrorLevel::INFO, "info"},
            {ErrorLevel::WARNING,
                "warning"},
            {ErrorLevel::ERROR,
                "error"}};

        const std::map<ErrorLevel, std::string> COLOR{
            {ErrorLevel::INFO, "\033[1;34m"},
            {ErrorLevel::WARNING, "\033[1;33m"},
            {ErrorLevel::ERROR, "\033[1;31m"}};

        const std::string COLOR_END("\033[0m");

        return COLOR.at(errorLevel) + _uri + ":" + std::to_string(lineNumber) + ":" + SEVERITY.at(errorLevel) + COLOR_END;
    }

    const std::string errorMsg(long unsigned int lineNumber, ErrorLevel errorLevel,
        std::string msg = "") const;

    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    const std::string ERROR_OPENING_FILE() const;

    const std::string ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const;

    const std::string ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                                     const SectionType& type) const;

    const std::string ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(long unsigned int lineNumber,
                                                                 const VascularSectionType& type) const;

    const std::string ERROR_MULTIPLE_SOMATA(
        const std::vector<Sample>& somata) const;

    const std::string ERROR_MISSING_PARENT(const Sample& sample) const;

    std::string ERROR_SOMA_BIFURCATION(
        const Sample& sample, const std::vector<Sample>& children) const;

    std::string ERROR_SOMA_WITH_NEURITE_PARENT(const Sample& sample) const;

    const std::string ERROR_REPEATED_ID(const Sample& originalSample,
        const Sample& newSample) const;

    const std::string ERROR_SELF_PARENT(const Sample& sample) const;

    const std::string ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(
        const std::string&) const;

    const std::string ERROR_MISSING_MITO_PARENT(int mitoParentId) const;

    ////////////////////////////////////////////////////////////////////////////////
    //             NEUROLUCIDA
    ////////////////////////////////////////////////////////////////////////////////
    const std::string ERROR_SOMA_ALREADY_DEFINED(long unsigned int lineNumber) const;

    const std::string ERROR_PARSING_POINT(long unsigned int lineNumber,
        const std::string& point) const;

    const std::string ERROR_UNKNOWN_TOKEN(long unsigned int lineNumber,
        const std::string& token) const;

    const std::string ERROR_UNEXPECTED_TOKEN(long unsigned int lineNumber,
        const std::string& expected,
        const std::string& got,
        const std::string& msg) const;

    const std::string ERROR_EOF_REACHED(long unsigned int lineNumber) const;

    const std::string ERROR_EOF_IN_NEURITE(long unsigned int lineNumber) const;

    const std::string ERROR_EOF_UNBALANCED_PARENS(long unsigned int lineNumber) const;

    const std::string ERROR_UNCOMPATIBLE_FLAGS(morphio::Option flag1,
        morphio::Option flag2) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              WRITERS
    ////////////////////////////////////////////////////////////////////////////////

    const std::string ERROR_WRONG_EXTENSION(const std::string filename) const;

    std::string ERROR_VECTOR_LENGTH_MISMATCH(const std::string& vec1,
        size_t length1,
        const std::string& vec2,
        size_t length2) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              WARNINGS
    ////////////////////////////////////////////////////////////////////////////////

    std::string WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED() const;
    std::string WARNING_WRITE_NO_SOMA() const;
    std::string WARNING_NO_SOMA_FOUND() const;
    std::string WARNING_DISCONNECTED_NEURITE(const Sample& sample) const;
    std::string WARNING_WRONG_DUPLICATE(
        std::shared_ptr<morphio::mut::Section> current,
        std::shared_ptr<morphio::mut::Section> parent) const;
    std::string WARNING_APPENDING_EMPTY_SECTION(std::shared_ptr<morphio::mut::Section>);
    const std::string WARNING_ONLY_CHILD(const DebugInfo& info, unsigned int parentId,
        unsigned int childId) const;

    const std::string WARNING_NEUROMORPHO_SOMA_NON_CONFORM(
        const Sample& root, const Sample& child1, const Sample& child2);

    std::string WARNING_WRONG_ROOT_POINT(
        const std::vector<Sample>& children) const;

private:
    std::string _uri;
};

} // namespace readers

} // namespace morphio
