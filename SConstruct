env = Environment( CPPPATH = "#include",
                   CCFLAGS = "-O3 -g -std=gnu99 -Werror",
                   tools = [ "default", "doxygen" ],
                   toolpath = "." )

env.ParseConfig( "pkg-config --cflags --libs opencv glib-2.0 yaml-0.1" )

# An environment that links against libkoki
lk_env = env.Clone()
lk_env.Append( LIBS = "koki", LIBPATH = "#lib" )

Export("env lk_env")

SConscript( Glob( "*/SConscript" ) )
