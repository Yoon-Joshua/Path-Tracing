#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
class Configuration
{
public:
    static Configuration &GetInstance()
    {
        static Configuration handle(configFile);
        return handle;
    }
    static const std::string configFile;

private:
    Configuration(const std::string &configFile)
    {
        std::ifstream f(configFile);
        nlohmann::json config = nlohmann::json::parse(f);
        f.close();

        cameraSetting.speed = config["main_camera"]["speed"];
        cameraSetting.sensitivity = config["main_camera"]["sensitivity"];

        for (auto s : config["mesh_to_load"])
        {
            std::string name = s;
            name = "assets/" + name;
            meshToLoad.push_back(name);
        }
    }
    ~Configuration() = default;

    struct
    {
        float speed;
        float sensitivity;
    } cameraSetting;

    std::vector<std::string> meshToLoad;

    friend class Camera;
    friend class SimpleScene;
};