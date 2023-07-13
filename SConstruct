env = Environment(CXXFLAGS = "-g --std=c++20")

libs = ["podofo", "boost_program_options"]


voyager_overlay_sources = ['voyager-overlay.cpp']

env.Program('voyager-overlay', voyager_overlay_sources, LIBS = libs)
