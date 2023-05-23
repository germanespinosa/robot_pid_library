#include <iostream>
#include <params_cpp.h>
#include <cell_world.h>
#include <experiment.h>
#include <controller.h>
#include <agent_tracking.h>
#include <robot_lib/robot_agent.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace easy_tcp;
using namespace params_cpp;
using namespace robot;
using namespace experiment;
using namespace controller;

struct Robot_experiment_client : Experiment_client {
    void on_experiment_started(const Start_experiment_response &experiment) {
    }
    void on_episode_started(const std::string &experiment_name) {
    };
};

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

    auto tsi = p.get(tracking_server_ip, "127.0.0.1");
    auto tsp = stoi(p.get(tracking_server_port, "4510"));

    Key experiment_server_ip{"-esi","--experiment_server_ip"};
    Key experiment_server_port{"-esp","--experiment_server_port"};

    auto esi = p.get(experiment_server_ip, "127.0.0.1");
    auto esp = stoi(p.get(experiment_server_port, "4540"));


    World world(wc, wi, o);

    auto capture_parameters = Resources::from("capture_parameters").key("default").get_resource<Capture_parameters>();
    auto peeking_parameters = Resources::from("peeking_parameters").key("default").get_resource<Peeking_parameters>();
    Capture capture(capture_parameters, world);
    Peeking peeking(peeking_parameters, world);
    Cell_group cells = world.create_cell_group();
    Map map(cells);
    Location_visibility visibility(cells, wc.cell_shape, wi.cell_transformation);

    Controller_server::Controller_experiment_client controller_experiment_client;
    if (!controller_experiment_client.connect(esi)) {
        cerr << "failed to connect to experiment server" << endl;
        exit(1);
    }
    controller_experiment_client.subscribe();


    Controller_server::Controller_tracking_client controller_tracking_client(visibility, 90, capture, peeking, "predator", "prey");
    if (!controller_tracking_client.connect(tsi, tsp)) {
        cerr << "failed to connect to tracking server" << endl;
        exit(1);
    }
    controller_tracking_client.set_throughput(10);
    controller_tracking_client.subscribe();

    Agent_operational_limits limits;
    limits.load("../config/robot_simulator_operational_limits.json");
    Robot_agent robot(limits);
    if (!robot.connect("127.0.0.1")) {
        cerr << "failed to connect to robot" << endl;
        exit(1);
    }
    Controller_service::set_logs_folder("controller_logs/");
    Location a,b,c;
    Controller_server controller_server("../config/pid.json", robot, controller_tracking_client, controller_experiment_client, a, b, c);
    if (!controller_server.start(Controller_service::get_port())) {
        cout << "failed to start controller" << endl;
        exit(1);
    }
    while (true) {
        this_thread::sleep_for(1s);
    }
    return 0;
}
