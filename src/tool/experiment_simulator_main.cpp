#include <iostream>
#include <easy_tcp.h>
#include <robot_lib/robot_simulator.h>
#include <robot_lib/robot.h>
#include <params_cpp.h>
#include <agent_tracking/tracking_client.h>
#include <robot_lib/tracking_simulator.h>
#include <cell_world.h>
#include <experiment.h>
#include <controller.h>
#include <map>
#include <robot_lib/robot_agent.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace easy_tcp;
using namespace params_cpp;
using namespace robot;
using namespace experiment;
using namespace controller;
using namespace agent_tracking;

int main(int argc, char *argv[])
{
    Experiment_service::set_logs_folder("experiment_logs/");
    Experiment_server experiment_server;
    experiment_server.start(Experiment_service::get_port());

    Tracking_server tracking_server;
    tracking_server.start(Tracking_service::get_port());
    auto &tracking_client = tracking_server.create_local_client<Experiment_tracking_client>();
    tracking_client.experiment_server = &experiment_server;
    experiment_server.set_tracking_client(tracking_client);
    while (true) {
        this_thread::sleep_for(1s);
    }
}
