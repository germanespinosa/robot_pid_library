from time import sleep
import sys
from random import choice
from cellworld import World, Display, Location, Agent_markers, Capture, Capture_parameters, Step, Timer, Cell_group_builder, to_radians, to_degrees, Location_list, Cell_group
from cellworld_controller_service import ControllerClient
from cellworld_experiment_service import ExperimentClient
from cellworld_tracking import TrackingService, TrackingClient

display = None
world = None
occlusions = Cell_group_builder()
free_cells = Cell_group()
spawn_locations = Location_list()
current_predator_destination = Location


class AgentData:
    def __init__(self, agent_name: str):
        self.is_valid = None
        self.step = Step()
        self.step.agent_name = agent_name


def on_step(step: Step):
    if step.agent_name == "predator":
        predator.is_valid = Timer(time_out)
        predator.step = step


def on_click(event):
    global current_predator_destination

    if event.button == 1:
        location = Location(event.xdata, event.ydata)
        cell_id = world.cells.find(location)
        destination_cell = world.cells[cell_id]
        if destination_cell.occluded:
            print("can't navigate to an occluded cell")
            return
        current_predator_destination = destination_cell.location
        controller.set_destination(destination_cell.location)
        destination_circle.set(center=(current_predator_destination.x, current_predator_destination.y), color=explore_color)



# create predator object
predator = AgentData('predator')

# initialize world
world = World.get_from_parameters_names("hexagonal", "canonical")
display = Display(world, fig_size=(10, 10), animated=True)
time_out = 1.0


# connect to controller
controller = ControllerClient()
if not controller.connect("127.0.0.1", 4590):
    print("failed to connect to the controller")
    exit(1)
controller.set_request_time_out(1000000)
controller.subscribe()
controller.on_step = on_step


# INITIALIZE KEYBOARD & CLICK INTERRUPTS
cid1 = display.fig.canvas.mpl_connect('button_press_event', on_click)


# ADD PREDATOR AND PREY TO WORLD
display.set_agent_marker("predator", Agent_markers.arrow())

# ADD PREDATOR DESTINATION TO WORLD
explore_color = "magenta"
pursue_color = "cyan"
spawn_color = "green"
destination_circle = display.circle(predator.step.location, 0.01, explore_color)


running = True
while running:

    if predator.is_valid:
        display.agent(step=predator.step, color="blue", size=10)

    else:
        display.agent(step=predator.step, color="gray", size=10)

    display.fig.canvas.draw_idle()
    display.fig.canvas.start_event_loop(0.1)
#    sleep(0.1)

controller.unsubscribe()
controller.stop()










