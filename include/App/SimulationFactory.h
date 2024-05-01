#pragma once
#include "Simulation.h"

#include <map>
#include <vector>
#include <functional>

class SimulationFactory
{
public:
    typedef std::function<Simulation* (const v2& position, const std::string& type)> Builder;

    // returns true if the registration succeeded, false otherwise
    inline bool Register(const std::string& key, const Builder& builder)
    {
        return map.insert(std::make_pair(key, builder)).second;
    }

    // returns a pointer to a new instance of Foo (or a derived class)
    // if the key was found, 0 otherwise
    inline Simulation* Build(const std::string& key, const v2& position) const
    {
        auto it = map.find(key);
        if (it == map.end()) { return nullptr; } // no such key
        return (it->second)(position, key);
    }

    inline std::vector<std::string> Names() const
    {
        std::vector<std::string> arr;
        for (const auto& pair : map)
            arr.push_back(pair.first);
        return arr;
    }

private:
    std::map<std::string, Builder> map;
};

template <typename Derived>
extern inline Simulation* SimulationBuilder(const v2& position, const std::string& type) { return new Derived(position, type); }

extern inline SimulationFactory& GetSimulationFactory() { static SimulationFactory F; return F; }