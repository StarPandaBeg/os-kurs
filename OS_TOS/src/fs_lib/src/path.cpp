#include "path.h"

#include <numeric>
#include <stdexcept>
#include <sstream>

using namespace fs;

Path::Path(const std::string& path)
{
	set(path);
}

Path::Path(const char* path) 
{
    set(path);
}

void Path::set(const std::string& path)
{
    if (!is_absolute_path(path)) {
        throw std::invalid_argument("Path must be absolute and start with '/'");
    }
    parse_path(path);
    update_full_path();
}

std::string Path::get() const
{
    return m_full_path;
}

std::string Path::filename() const
{
    return filename_level(m_components.size() - 1);
}

std::string Path::filename_level(size_t level) const
{
    if (level >= m_components.size()) {
        throw std::out_of_range("Invalid level");
    }
    return m_components[level];
}

std::string Path::get_level(size_t level) const
{
    if (level >= m_components.size()) {
        throw std::out_of_range("Invalid level");
    }
    if (level == 0) {
        return m_components[0];
    }

    size_t total_length = 0;
    for (size_t i = 0; i <= level; ++i) {
        total_length += m_components[i].size() + 1;  // +1 for root '/'
    }

    std::string result;
    result.reserve(total_length);

    for (size_t i = 0; i <= level; ++i) {
        result += m_components[i];
        if (i > 0 && result.back() != PATH_SEPARATOR) {
            result += PATH_SEPARATOR;
        }
    }

    if (result.size() > 1 && result.back() == PATH_SEPARATOR) {
        result.pop_back();
    }

    return result;
}

void Path::move_up()
{
    if (m_components.size() > 1) {
        m_components.pop_back();
        update_full_path();
    }
    else {
        throw std::runtime_error("Already at the root");
    }
}

void Path::add_component(const std::string& component)
{
    if (!component.empty() && component != "." && component != "..") {
        std::istringstream ss(component);
        std::string s;
        while (std::getline(ss, s, '/')) {
            m_components.push_back(s);
        }
        update_full_path();
    }
    else {
        throw std::invalid_argument("Invalid component");
    }
}

const std::vector<std::string>& Path::components() const
{
    return m_components;
}

const std::size_t Path::level() const
{
    return m_components.size() - 1;
}

void Path::parse_path(const std::string& path)
{
    m_components.clear();
    std::stringstream ss(path);
    std::string token;

    while (std::getline(ss, token, PATH_SEPARATOR)) {
        if (!token.empty() && token != ".") {
            m_components.push_back(token);
        }
    }

    if (!path.empty() && path[0] == PATH_SEPARATOR) {
        m_components.insert(m_components.begin(), std::string{ PATH_SEPARATOR });
    }
}

void Path::update_full_path()
{
    auto sep = std::string{ PATH_SEPARATOR };
    m_full_path.clear();
    for (const auto& comp : m_components) {
        if (comp == sep) {
            m_full_path += sep;
        }
        else {
            m_full_path += comp + sep;
        }
    }

    if (m_full_path.size() > 1 && m_full_path.back() == PATH_SEPARATOR) {
        m_full_path.pop_back();
    }
}

bool Path::is_absolute_path(const std::string& path) const
{
    return !path.empty() && path[0] == PATH_SEPARATOR;
}
