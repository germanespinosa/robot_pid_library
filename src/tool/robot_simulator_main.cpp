#include <iostream>
#include <easy_tcp.h>
#include <robot_lib/robot_simulator.h>
#include <robot_lib/robot.h>
#include <params_cpp.h>
#include <agent_tracking/tracking_client.h>
#include <robot_lib/tracking_simulator.h>
#include <cell_world.h>
#include <experiment.h>
#include <robot_lib/robot_agent.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace easy_tcp;
using namespace params_cpp;
using namespace robot;
using namespace experiment;
using namespace controller;

int main(int argc, char *argv[])
{
    Key spawn_coordinates_key{"-s","--spawn_coordinates"};
    Key rotation_key{"-r","--theta"};
    Key prey_key{"-p","--prey"};
    Key interval_key{"-i","--interval"};
    Key noise_key{"-n","--noise"};
    Key occlusions{"-wo","--occlusions"};
    Key world_configuration{"-wc","--world_configuration"};
    Key world_implementation{"-wi","--world_implementation"};

    Parser p(argc, argv);
    auto wc = Resources::from("world_configuration").key(p.get(world_configuration,"hexagonal")).get_resource<World_configuration>();
    auto wi = Resources::from("world_implementation").key(p.get(world_configuration,"hexagonal")).key(p.get(world_implementation,"canonical")).get_resource<World_implementation>();
    auto o = Resources::from("cell_group").key(p.get(world_configuration,"hexagonal")).key(p.get(occlusions,"00_00")).key("occlusions").get_resource<Cell_group_builder>();

    Key tracking_server_ip{"-tsi","--tracking_server_ip"};
    Key tracking_server_port{"-tsp","--tracking_server_port"};

    auto tsi = p.get(tracking_server_ip, "");
    auto tsp = stoi(p.get(tracking_server_port, "0"));

    World world(wc, wi, o);
    Cell_group cells = world.create_cell_group();
    Map map(cells);
    auto rotation = stof(p.get(rotation_key,"0"));
    auto interval = stoi(p.get(interval_key,"30"));
    auto spawn_coordinates_str = p.get(spawn_coordinates_key, "{\"x\":4,\"y\":0}");
    auto verbose = p.contains(Key("-v"));

    if (p.contains(prey_key)){
        Robot_simulator::start_prey();
    }

    Coordinates spawn_coordinates;
    cout << spawn_coordinates_str << endl;
    try {
        spawn_coordinates = json_cpp::Json_create<Coordinates>(spawn_coordinates_str);
    } catch (...) {
        cout << "Wrong parameters "<< endl;
        exit(1);
    }
    Location location = map[spawn_coordinates].location;

    agent_tracking::Tracking_client tracker;
    if (!tsi.empty()){
        tracker.connect(tsi, tsp);
    }
    Robot_simulator::start_simulation(world, location, rotation, interval, tracker);
    Server<Robot_simulator> server;
    if (!server.start(Robot::port())) {
        std::cout << "Server setup failed " << std::endl;
        return EXIT_FAILURE;
    }
    Agent_operational_limits limits;
    limits.load("../config/robot_simulator_operational_limits.json");
    Robot_agent robot(limits);
    robot.connect("127.0.0.1");

    while (Robot_simulator::is_running())
        if (tracker.contains_agent_state("predator")){
            if (verbose) cout << "track: " << tracker.get_current_state("predator") << endl;
            Timer::wait(.5);
        }
    server.stop();
    return 0;
}
