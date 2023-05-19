from cellworld_tracking import TrackingService, TrackingClient
from cellworld import World, Display, Location, Agent_markers, Step, Timer, Cell_group_builder, Location_list, Cell_group


ts = TrackingService()
ts.start(port=6587)

tc = TrackingClient()
tc.connect(port=6587)
tc.subscribe()

display = None
world = None
occlusions = Cell_group_builder()
free_cells = Cell_group()
spawn_locations = Location_list()
current_predator_destination = Location

# initialize world
world = World.get_from_parameters_names("hexagonal", "canonical")
display = Display(world, fig_size=(10, 10), animated=True)
time_out = 1.0

# ADD PREDATOR AND PREY TO WORLD
display.set_agent_marker("predator", Agent_markers.arrow())

# ADD PREDATOR DESTINATION TO WORLD
explore_color = "magenta"
pursue_color = "cyan"
spawn_color = "green"

running = True
while running:

    if tc.contains_agent_state("predator"):
        display.agent(step=tc.get_current_state("predator"), color="blue", size=10)

    display.fig.canvas.draw_idle()
    display.fig.canvas.start_event_loop(0.1)
    print("it's working")









