#include <iostream>
#include <random>
#include <fstream> // Needed for writing ppm image
#include <filesystem>

#include "lodepng.h"

#include "utils/vector.hpp"
#include "utils/rotation.hpp"
#include "utils/euler_angles.hpp"

#include "sensors/simple_sensor.hpp"

#include "cameras/camera.hpp"
#include "cameras/simple_camera.hpp"

#include "lights/light.hpp"
#include "lights/point_light.hpp"

#include "primitives/triangle.hpp"
#include "primitives/aabb.hpp"
#include "geometry.hpp"

#include "scene.hpp"

#include <chrono>

using Scalar = float;

int main(){
    // Define sensor:
    Scalar resolution[2] = {600,600};
    // Scalar resolution[2] = {100,100};
    Scalar size[2] = {36,36};
    SimpleSensor<Scalar> sensor(resolution, size);

    // Define camera:
    SimpleCamera<Scalar> camera(50, sensor, false);
    camera.set_position(0,0.4,-.1);
    Rotation<Scalar> rotation = XYZ_euler<Scalar>(90,0,180);
    camera.set_rotation(rotation);

    // Define a simple light:
    std::vector<Light<Scalar>*> lights;
    lights.push_back(new PointLight<Scalar>(500));
    lights[0]->set_position(0,3,1.5);

    std::vector<Geometry<Scalar>*> geometries;

    auto start = std::chrono::high_resolution_clock::now();
    
    // geometries.push_back(new Geometry<Scalar>("67p.bin","binary"));
    // geometries.push_back(new Geometry<Scalar>("../../BENNU/binary_global_set/l_00050mm_alt_dtm_3041s08903_v021.bin", "binary"));
    // geometries.push_back(new Geometry<Scalar>("../../BENNU/binary_global_set/l_00050mm_alt_dtm_1146s08248_v021.bin", "binary"));
    for (auto &p : std::filesystem::recursive_directory_iterator("../../BENNU/binary_global_set/")){
        geometries.push_back(new Geometry<Scalar>(p.path().c_str(),"binary"));
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "    Header read in " << duration.count() << " milliseconds\n";

    // Create the scene:
    auto scene = Scene<Scalar>(geometries);
    scene.Build();

    // Render:
    auto image = scene.render(camera,lights);

    // Save the image:
    size_t width  = (size_t) floor(camera.sensor->get_resolution_h());
    size_t height = (size_t) floor(camera.sensor->get_resolution_v());
    unsigned error = lodepng::encode("frame.png", image, width, height);
    if(error) {
        std::cout << "PNG error " << error << ": "<< lodepng_error_text(error) << std::endl;
    }

    return 0;
}