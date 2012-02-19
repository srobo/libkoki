env = Environment( CPPPATH = "#include",
                   CCFLAGS = "-O3 -g -std=gnu99 -Werror",
                   tools = [ "default", "doxygen" ],
                   toolpath = "." )

env.ParseConfig( "pkg-config --cflags --libs opencv glib-2.0 yaml-0.1" )

Export("env")

SConscript( Glob( "*/SConscript" ) )
